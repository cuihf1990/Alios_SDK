/**
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 *
 */
#include "rtos_pub.h"
#include "error.h"

#include "mico_cli.h"
#include "stdarg.h"

#include "include.h"
#include "mem_pub.h"
#include "str_pub.h"
#include "uart_pub.h"
#include "mico_rtos.h"
#include "mico_wlan.h"

#include "hal/soc/soc.h"

#define MICO_CLI_ENABLE
#define DEBUG 1

#ifdef MICO_CLI_ENABLE
#ifndef MOC
static void task_Command( char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv );
#endif

static struct cli_st *pCli = NULL;

int cli_putstr(const char *msg);


/* Find the command 'name' in the cli commands table.
* If len is 0 then full match will be performed else upto len bytes.
* Returns: a pointer to the corresponding cli_command struct or NULL.
*/
static const struct cli_command *lookup_command(char *name, int len)
{
    int i = 0;
    int n = 0;

    while (i < MAX_COMMANDS && n < pCli->num_commands)
    {
        if (pCli->commands[i]->name == NULL)
        {
            i++;
            continue;
        }
        /* See if partial or full match is expected */
        if (len != 0)
        {
            if (!os_strncmp(pCli->commands[i]->name, name, len))
                return pCli->commands[i];
        }
        else
        {
            if (!os_strcmp(pCli->commands[i]->name, name))
                return pCli->commands[i];
        }

        i++;
        n++;
    }

    return NULL;
}

/* Parse input line and locate arguments (if any), keeping count of the number
* of arguments and their locations.  Look up and call the corresponding cli
* function if one is found and pass it the argv array.
*
* Returns: 0 on success: the input line contained at least a function name and
*          that function exists and was called.
*          1 on lookup failure: there is no corresponding function for the
*          input line.
*          2 on invalid syntax: the arguments list couldn't be parsed
*/
static int handle_input(char *inbuf)
{
    struct
    {
        unsigned inArg: 1;
        unsigned inQuote: 1;
        unsigned done: 1;
    } stat;
    static char *argv[16];
    int argc = 0;
    int i = 0;
    const struct cli_command *command = NULL;
    const char *p;

    os_memset((void *)&argv, 0, sizeof(argv));
    os_memset(&stat, 0, sizeof(stat));

    do
    {
        switch (inbuf[i])
        {
        case '\0':
            if (stat.inQuote)
                return 2;
            stat.done = 1;
            break;

        case '"':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg)
            {
                os_memcpy(&inbuf[i - 1], &inbuf[i],
                       os_strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
                break;
            if (stat.inQuote && !stat.inArg)
                return 2;

            if (!stat.inQuote && !stat.inArg)
            {
                stat.inArg = 1;
                stat.inQuote = 1;
                argc++;
                argv[argc - 1] = &inbuf[i + 1];
            }
            else if (stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                stat.inQuote = 0;
                inbuf[i] = '\0';
            }
            break;

        case ' ':
            if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg)
            {
                os_memcpy(&inbuf[i - 1], &inbuf[i],
                       os_strlen(&inbuf[i]) + 1);
                --i;
                break;
            }
            if (!stat.inQuote && stat.inArg)
            {
                stat.inArg = 0;
                inbuf[i] = '\0';
            }
            break;

        default:
            if (!stat.inArg)
            {
                stat.inArg = 1;
                argc++;
                argv[argc - 1] = &inbuf[i];
            }
            break;
        }
    }
    while (!stat.done && ++i < INBUF_SIZE);

    if (stat.inQuote)
        return 2;

    if (argc < 1)
        return 0;

    if (!pCli->echo_disabled)
        os_printf("\r\n");

    /*
    * Some comamands can allow extensions like foo.a, foo.b and hence
    * compare commands before first dot.
    */
    i = ((p = os_strchr(argv[0], '.')) == NULL) ? 0 :
        (p - argv[0]);
    command = lookup_command(argv[0], i);
    if (command == NULL)
        return 1;

    os_memset(pCli->outbuf, 0, OUTBUF_SIZE);
    cli_putstr("\r\n");
    command->function(pCli->outbuf, OUTBUF_SIZE, argc, argv);
    cli_putstr(pCli->outbuf);
    return 0;
}

/* Perform basic tab-completion on the input buffer by string-matching the
* current input line against the cli functions table.  The current input line
* is assumed to be NULL-terminated. */
static void tab_complete(char *inbuf, unsigned int *bp)
{
    int i, n, m;
    const char *fm = NULL;

    os_printf("\r\n");

    /* show matching commands */
    for (i = 0, n = 0, m = 0; i < MAX_COMMANDS && n < pCli->num_commands;
            i++)
    {
        if (pCli->commands[i]->name != NULL)
        {
            if (!os_strncmp(inbuf, pCli->commands[i]->name, *bp))
            {
                m++;
                if (m == 1)
                    fm = pCli->commands[i]->name;
                else if (m == 2)
                    os_printf("%s %s ", fm,
                               pCli->commands[i]->name);
                else
                    os_printf("%s ",
                               pCli->commands[i]->name);
            }
            n++;
        }
    }

    /* there's only one match, so complete the line */
    if (m == 1 && fm)
    {
        n = os_strlen(fm) - *bp;
        if (*bp + n < INBUF_SIZE)
        {
            os_memcpy(inbuf + *bp, fm + *bp, n);
            *bp += n;
            inbuf[(*bp)++] = ' ';
            inbuf[*bp] = '\0';
        }
    }

    /* just redraw input line */
    os_printf("%s%s", PROMPT, inbuf);
}

/* Get an input line.
*
* Returns: 1 if there is input, 0 if the line should be ignored. */
static int get_input(char *inbuf, unsigned int *bp)
{
    if (inbuf == NULL)
    {
		os_printf("inbuf_null\r\n");
        return 0;
    }
	
    while (cli_getchar(&inbuf[*bp]) == 1)
    {
        if (inbuf[*bp] == RET_CHAR)
            continue;
        if (inbuf[*bp] == END_CHAR)  	/* end of input line */
        {
            inbuf[*bp] = '\0';
            *bp = 0;
            return 1;
        }

        if ((inbuf[*bp] == 0x08) ||	/* backspace */
                (inbuf[*bp] == 0x7f))  	/* DEL */
        {
            if (*bp > 0)
            {
                (*bp)--;
                if (!pCli->echo_disabled)
                    os_printf("%c %c", 0x08, 0x08);
            }
            continue;
        }

        if (inbuf[*bp] == '\t')
        {
            inbuf[*bp] = '\0';
            tab_complete(inbuf, bp);
            continue;
        }

        if (!pCli->echo_disabled)
            os_printf("%c", inbuf[*bp]);

        (*bp)++;
        if (*bp >= INBUF_SIZE)
        {
            os_printf("Error: input buffer overflow\r\n");
            os_printf(PROMPT);
            *bp = 0;
            return 0;
        }

    }

    return 0;
}

/* Print out a bad command string, including a hex
* representation of non-printable characters.
* Non-printable characters show as "\0xXX".
*/
static void print_bad_command(char *cmd_string)
{
    if (cmd_string != NULL)
    {
        char *c = cmd_string;
        os_printf("command '");
        while (*c != '\0')
        {
            if (isprint(*c))
            {
                os_printf("%c", *c);
            }
            else
            {
                os_printf("\\0x%x", *c);
            }
            ++c;
        }
        os_printf("' not found\r\n");
    }
}

/* Main CLI processing thread
*
* Waits to receive a command buffer pointer from an input collector, and
* then processes.  Note that it must cleanup the buffer when done with it.
*
* Input collectors handle their own lexical analysis and must pass complete
* command lines to CLI.
*/
static void cli_main( uint32_t data )
{
    while (1)
    {
        int ret;
        char *msg = NULL;

        if(get_input(pCli->inbuf, &pCli->bp))
        {
        	msg = pCli->inbuf;
			
            if (os_strcmp(msg, EXIT_MSG) == 0)
                break;
			
            ret = handle_input(msg);
            if (ret == 1)
                print_bad_command(msg);
            else if (ret == 2)
                os_printf("syntax error\r\n");
			
            os_printf(PROMPT);
        }
    }

    os_printf("CLI exited\r\n");
    os_free(pCli);
    pCli = NULL;
	
    mico_rtos_delete_thread(NULL);
}

#ifndef MOC
static void task_Command( char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv )
{
    mico_rtos_print_thread_status( pcWriteBuffer, xWriteBufferLen );
}
#endif

static void tftp_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
}

static void partShow_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    

}

static void uptime_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("UP time %ldms\r\n", mico_rtos_get_time());
}

#ifdef CONFIG_YOS_MESH
extern void ur_cli_input_args(char **argv, uint16_t argc);
static void umesh_command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    ur_cli_input_args(argv, argc);
}
#endif

//extern void tftp_ota( void );
void tftp_ota_thread( mico_thread_arg_t arg )
{
    //tftp_ota( );
    mico_rtos_delete_thread( NULL );
}

void ota_Command( char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv )
{
    mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "LOCAL OTA", tftp_ota_thread, 0x4096, 0 );
}

void help_command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv);

/*
*  Command buffer API
*/
void wifiscan_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	os_printf("wifiscan_Command\r\n");
	test_scan_app_init();
}

void softap_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	char *ap_ssid = NULL;
	char *ap_key;
	
	os_printf("softap_Command\r\n");
	if (argc == 2)
	{
		ap_ssid = argv[1];
		ap_key = "1";
	}
	else if (argc == 3)
	{
		ap_ssid = argv[1];
		ap_key = argv[2];
	}
	
	if(ap_ssid)
	{
		test_softap_app_init(ap_ssid, ap_key);
	}
}

void cli_monitor_cb(uint8_t*data, int len)
{
	uint32_t count, i;

	count = MIN(32,len);
	os_printf("cli_monitor_cb:%d:%d\r\n", count, len);
	for(i = 0; i < count; i ++)
	{
		os_printf("%x ", data[i]);
	}
	os_printf("\r\n");
}

void mtr_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    uint32_t channel_num;

    if(argc != 2)
    {
        os_printf("monitor_parameter invalid\r\n");
        return;
    }
    channel_num = os_strtoul(argv[1], NULL, 10);

	if(99 == channel_num)
	{
	    mico_wlan_stop_monitor();
	}
	else
	{
	    mico_wlan_start_monitor();
	    mico_wlan_register_monitor_cb(cli_monitor_cb);
	    mico_wlan_set_channel(channel_num);
	}
}

void sta_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    char *oob_ssid = NULL;
    char *connect_key;

    os_printf("sta_Command\r\n");
    if (argc == 2)
    {
        oob_ssid = argv[1];
        connect_key = "1";
    }
    else if (argc == 3)
    {
        oob_ssid = argv[1];
        connect_key = argv[2];
    }
    else
    {
        os_printf("parameter invalid\r\n");
    }

    if(oob_ssid)
    {
        test_sta_app_init(oob_ssid, connect_key);
    }
}


void wifistate_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("wifistate_Command\r\n");
}

void wifidebug_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("wifidebug_Command\r\n");
}

void ifconfig_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("ifconfig_Command\r\n");
}

void arp_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("arp_Command\r\n");
}

void ping_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("ping_Command\r\n");
}

void dns_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("dns_Command\r\n");
}

void socket_show_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("socket_show_Command\r\n");
}

void memory_show_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	dumpsys_mm_info_func(NULL, 0);
}

void memory_dump_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("memory_dump_Command\r\n");
}

void memory_set_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("memory_set_Command\r\n");
}

void memp_dump_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("memp_dump_Command\r\n");
}

void driver_state_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("driver_state_Command\r\n");
}

void get_version(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("get_version\r\n");
}

void reboot(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    FUNCPTR reboot = 0;

    os_printf("reboot\r\n");

    (*reboot)();
}

static void echo_cmd_handler(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    if (argc == 1)
    {
        os_printf("Usage: echo on/off. Echo is currently %s\r\n",
                  pCli->echo_disabled ? "Disabled" : "Enabled");
        return;
    }

    if (!os_strcasecmp(argv[1], "on"))
    {
        os_printf("Enable echo\r\n");
        pCli->echo_disabled = 0;
    }
    else if (!os_strcasecmp(argv[1], "off"))
    {
        os_printf("Disable echo\r\n");
        pCli->echo_disabled = 1;
    }
}

static void cli_exit_handler(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    // exit command not executed
}

/*
CMD FORMAT: GPIO CMD index PARAM
exmaple:GPIO 0 18 2               (config GPIO18 input & pull-up)
*/
static void Gpio_op_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    uint32_t ret, id, mode, i;
    char cmd0 = 0;
    char cmd1 = 0;
    char cmd;

    for(i = 0; i < argc; i++)
    {
        os_printf("Argument %d = %s\r\n", i + 1, argv[i]);
    }

    if(argc == 4)
    {
        cmd = argv[1][0];
        mode = argv[3][0];

        cmd0 = argv[2][0] - 0x30;
        cmd1 = argv[2][1] - 0x30;

        id = (uint32_t)(cmd0 * 10 + cmd1);
        os_printf("---%x,%x----\r\n", id, mode);
        ret = MicoGpioOp(cmd, id, mode);
        os_printf("gpio op:%x\r\n", ret);
    }
    else
        os_printf("cmd param error\r\n");
}

void test_fun(char para)
{
    os_printf("---%d---\r\n", para);
}
/*
cmd format: GPIO_INT cmd index  triggermode
enable: GPIO_INT 1 18 0
*/
static void Gpio_int_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    uint32_t ret, id, mode, i;
    char cmd0 = 0;
    char cmd1 = 0;
    char cmd;

    if(argc == 4)
    {
        cmd = argv[1][0] - 0x30;
        mode = argv[3][0] - 0x30;

        cmd0 = argv[2][0] - 0x30;
        cmd1 = argv[2][1] - 0x30;

        id = (uint32_t)(cmd0 * 10 + cmd1);
        gpio_test_func(cmd, id, mode, test_fun);
    }
    else
        os_printf("cmd param error\r\n");

}

/*
format: FLASH  E/R/W  0xABCD
example:	    FLASH  R  0x00100

*/
static void flash_command_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    
}

/*UART  I  index
example:   UART I 0
*/
static void Uart_command_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    

}
static void tx_evm_cmd_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int ret = do_evm(NULL, 0, argc, argv);
    if(ret) {
        os_printf("tx_evm bad parameters\r\n");
    }
}

static void rx_sens_cmd_test(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int ret = do_rx_sensitivity(NULL, 0, argc, argv);
    if(ret) {
        os_printf("rx sensitivity bad parameters\r\n");
    }
}

static const struct cli_command built_ins[] =
{
    {"help", NULL, help_command},
    {"version", NULL, get_version},
    {"echo", NULL, echo_cmd_handler},
    {"exit", "CLI exit", cli_exit_handler},

    /// WIFI
    {"scan", "scan ap", wifiscan_Command},
    {"softap", "softap ssid key", softap_Command},
    {"sta", "sta ap_ssid key", sta_Command},
    {"mtr", "mtr channel", mtr_Command},

    {"wifistate", "Show wifi state", wifistate_Command},
    {"wifidebug", "wifidebug on/off", wifidebug_Command},

    // network
    {"ifconfig", "Show IP address", ifconfig_Command},
    {"arp", "arp show/clean", arp_Command},
    {"ping", "ping <ip>", ping_Command},
    {"dns", "show/clean/<domain>", dns_Command},
    {"sockshow", "Show all sockets", socket_show_Command},
    // os
    {"tasklist", "list all thread name status", task_Command},

    // others
    {"memshow", "print memory information", memory_show_Command},
    {"memdump", "<addr> <length>", memory_dump_Command},
    {"os_memset", "<addr> <value 1> [<value 2> ... <value n>]", memory_set_Command},
    {"memp", "print memp list", memp_dump_Command},
    
    {"wifidriver", "show wifi driver status", driver_state_Command}, // bus credite, flow control...
    {"reboot", "reboot system", reboot},
    
    {"tftp",     "tftp",                        tftp_Command},
    {"time",     "system time",                 uptime_Command},
    {"ota",      "system ota",                  ota_Command},
    {"flash",    "Flash memory map",            partShow_Command},

    {"GPIO", "GPIO <cmd> <arg1> <arg2>", Gpio_op_Command},
    {"GPIO_INT", "GPIO_INT <cmd> <arg1> <arg2>", Gpio_int_Command},
    {"FLASH", "FLASH <cmd(R/W/E)> <len>", flash_command_test},
    {"UART", "UART I <index>", Uart_command_test},
    
    {"txevm", "txevm [-m] [-c] [-l] [-r] [-w]", tx_evm_cmd_test},
    {"rxsens", "rxsens [-m] [-d] [-c] [-l]", rx_sens_cmd_test},    
#ifdef CONFIG_YOS_MESH
    {"umesh", "umesh <cmd> <args>", umesh_command},
#endif
};

/* Built-in "help" command: prints all registered commands and their help
* text string, if any. */
void help_command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int i, n;
    uint32_t build_in_count = sizeof(built_ins) / sizeof(struct cli_command);

#if (DEBUG)
    build_in_count++; //For command: micodebug
#endif

    os_printf( "====Build-in Commands====\r\n" );
    for (i = 0, n = 0; i < MAX_COMMANDS && n < pCli->num_commands; i++)
    {
        if (pCli->commands[i]->name)
        {
            os_printf("%s: %s\r\n", pCli->commands[i]->name,
                      pCli->commands[i]->help ?
                      pCli->commands[i]->help : "");
            n++;
            if( n == build_in_count )
            {
                os_printf("\r\n====User Commands====\r\n");
            }
        }
    }
}


int cli_register_command(const struct cli_command *command)
{
    int i;
    if (!command->name || !command->function)
        return 1;

    if (pCli->num_commands < MAX_COMMANDS)
    {
        /* Check if the command has already been registered.
        * Return 0, if it has been registered.
        */
        for (i = 0; i < pCli->num_commands; i++)
        {
            if (pCli->commands[i] == command)
                return 0;
        }
        pCli->commands[pCli->num_commands++] = command;
        return 0;
    }

    return 1;
}

int cli_unregister_command(const struct cli_command *command)
{
    int i;
    if (!command->name || !command->function)
        return 1;

    for (i = 0; i < pCli->num_commands; i++)
    {
        if (pCli->commands[i] == command)
        {
            pCli->num_commands--;
            int remaining_cmds = pCli->num_commands - i;
            if (remaining_cmds > 0)
            {
                os_memmove(&pCli->commands[i], &pCli->commands[i + 1],
                           (remaining_cmds *
                            sizeof(struct cli_command *)));
            }
            pCli->commands[pCli->num_commands] = NULL;
            return 0;
        }
    }

    return 1;
}


int cli_register_commands(const struct cli_command *commands, int num_commands)
{
    int i;
    for (i = 0; i < num_commands; i++)
        if (cli_register_command(commands++))
            return 1;
    return 0;
}

int cli_unregister_commands(const struct cli_command *commands,
                            int num_commands)
{
    int i;
    for (i = 0; i < num_commands; i++)
        if (cli_unregister_command(commands++))
            return 1;

    return 0;
}
#if (DEBUG)
int mico_debug_enabled;
static void micodebug_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    if (argc == 1)
    {
        os_printf("Usage: micodebug on/off. MICO debug is currently %s\r\n",
                  mico_debug_enabled ? "Enabled" : "Disabled");
        return;
    }

    if (!os_strcasecmp(argv[1], "on"))
    {
        os_printf("Enable MICO debug\r\n");
        mico_debug_enabled = 1;
    }
    else if (!os_strcasecmp(argv[1], "off"))
    {
        os_printf("Disable MICO debug\r\n");
        mico_debug_enabled = 0;
    }
}

void monitor(uint8_t *data, int len)
{
	int i;

	os_printf("[%d]: ", len);
	for(i=0;i<len;i++) {
		os_printf("%02x ", data[i]);
	}
	os_printf("\r\n");
}

static void monitor_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    if (argc == 1)
    {
        os_printf("Usage: monitor on/off.");
        return;
    }

    if (!os_strcasecmp(argv[1], "on"))
    {
        cmd_printf("start monitor\r\n");
		mico_wlan_register_monitor_cb(monitor);
        mico_wlan_start_monitor();
    }
    else if (!os_strcasecmp(argv[1], "off"))
    {
        cmd_printf("stop monitor\r\n");
        mico_debug_enabled = 0;
		mico_wlan_register_monitor_cb(NULL);
		mico_wlan_stop_monitor();
    }
}

static void channel_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	int channel;
	
    if (argc == 1)
    {
        os_printf("Usage: channel [1~13].");
        return;
    }

    channel = atoi(argv[1]);
	cmd_printf("set to channel %d\r\n", channel);
	mico_wlan_set_channel(channel);
}
static void mac_command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    int i, n;
    uint8_t mac[6];

    if (argc == 1)
    {
        wifi_get_mac_address(mac);
        os_printf("MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n", 
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
	else if(argc == 2)
	{
		hexstr2bin(argv[1], mac, 6);
		wifi_set_mac_address(mac);
        os_printf("Set MAC address: %02x-%02x-%02x-%02x-%02x-%02x\r\n", 
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
    else
    {
        os_printf("invalid cmd\r\n");
    }

}

static const struct cli_command user_clis[] =
{
    {"micodebug", "micodebug on/off", micodebug_Command},
	{"monitor", "monitor on/off", monitor_Command},
	{"channel", "channel []", channel_Command},
	{"mac", "mac <mac>, Get mac/Set mac. <mac>: c89346000001", mac_command}
};
#endif

const hal_uart_config_t config = 
{
    .baud_rate = 921600,
    .data_width = DATA_WIDTH_8BIT,
    .parity = NO_PARITY,
    .stop_bits = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
    .rx_buf_size = 256,
};

int cli_init(void)
{
    int ret;

    pCli = (struct cli_st *)os_malloc(sizeof(struct cli_st));
    if (pCli == NULL)
        return kNoMemoryErr;

    os_memset((void *)pCli, 0, sizeof(struct cli_st));
	hal_uart_init(CLI_UART, &config);

    /* add our built-in commands */
    if (cli_register_commands(&built_ins[0],
                              sizeof(built_ins) /
                              sizeof(struct cli_command)))
    {
        goto init_general_err;
    }

	cli_register_commands(user_clis, sizeof(user_clis) / sizeof(struct cli_command));


    ret = mico_rtos_create_thread(NULL,
                                  MICO_DEFAULT_WORKER_PRIORITY,
                                  "cli",
                                  cli_main,
                                  4096,
                                  0);
    if (ret != kNoErr)
    {
        os_printf("Error: Failed to create cli thread: %d\r\n",
                  ret);
        goto init_general_err;
    }

    pCli->initialized = 1;
    pCli->echo_disabled = 0;

    return kNoErr;

init_general_err:
    if(pCli)
    {
        os_free(pCli);
        pCli = NULL;
    }

    return kGeneralErr;
}


/* ========= CLI input&output APIs ============ */
int cli_printf(const char *msg, ...)
{
    va_list ap;
    char *pos, message[256];
    int sz;
    int nMessageLen = 0;

    os_memset(message, 0, 256);
    pos = message;

    sz = 0;
    va_start(ap, msg);
    nMessageLen = vsnprintf(pos, 256 - sz, msg, ap);
    va_end(ap);

    if( nMessageLen <= 0 ) return 0;

    cli_putstr((const char *)message);
    return 0;
}


int cli_putstr(const char *msg)
{
    if (msg[0] != 0)
        hal_uart_send( CLI_UART, (const char *)msg, os_strlen(msg) );

    return 0;
}

int cli_getchar(char *inbuf)
{
    if (hal_uart_recv(CLI_UART, inbuf, 1, NULL, MICO_WAIT_FOREVER) == 0)
        return 1;
    else
        return 0;
}

#endif

