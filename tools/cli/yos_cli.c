/**
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include "stdarg.h"
#include <k_err.h>
#include <yos/kernel.h>
#include <k_api.h>
#include "hal/soc/soc.h"
#include "dumpsys.h"
#include <yos/cli.h>

#ifndef STDIO_UART
#define STDIO_UART 0
#endif

#define RET_CHAR        '\n'
#define END_CHAR        '\r'
#define PROMPT          "\r\n# "
#define EXIT_MSG        "exit"

#define DEBUG 1

static void task_Command( char *pcWriteBuffer, int xWriteBufferLen, int argc,
                          char **argv );

static struct cli_st *pCli = NULL;
static int            cliexit = 0;

int cli_putstr(const char *msg);

/* Find the command 'name' in the cli commands table.
* If len is 0 then full match will be performed else upto len bytes.
* Returns: a pointer to the corresponding cli_command struct or NULL.
*/
static const struct cli_command *lookup_command(char *name, int len)
{
    int i = 0;
    int n = 0;

    while (i < MAX_COMMANDS && n < pCli->num_commands) {
        if (pCli->commands[i]->name == NULL) {
            i++;
            continue;
        }
        /* See if partial or full match is expected */
        if (len != 0) {
            if (!strncmp(pCli->commands[i]->name, name, len)) {
                return pCli->commands[i];
            }
        } else {
            if (!strcmp(pCli->commands[i]->name, name)) {
                return pCli->commands[i];
            }
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
    struct {
        unsigned inArg: 1;
        unsigned inQuote: 1;
        unsigned done: 1;
    } stat;
    static char *argv[16];
    int argc = 0;
    int i = 0;
    const struct cli_command *command = NULL;
    const char *p;

    memset((void *)&argv, 0, sizeof(argv));
    memset(&stat, 0, sizeof(stat));

    do {
        switch (inbuf[i]) {
            case '\0':
                if (stat.inQuote) {
                    return 2;
                }
                stat.done = 1;
                break;

            case '"':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) {
                    memcpy(&inbuf[i - 1], &inbuf[i],
                           strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (!stat.inQuote && stat.inArg) {
                    break;
                }
                if (stat.inQuote && !stat.inArg) {
                    return 2;
                }

                if (!stat.inQuote && !stat.inArg) {
                    stat.inArg = 1;
                    stat.inQuote = 1;
                    argc++;
                    argv[argc - 1] = &inbuf[i + 1];
                } else if (stat.inQuote && stat.inArg) {
                    stat.inArg = 0;
                    stat.inQuote = 0;
                    inbuf[i] = '\0';
                }
                break;

            case ' ':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg) {
                    memcpy(&inbuf[i - 1], &inbuf[i],
                           strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (!stat.inQuote && stat.inArg) {
                    stat.inArg = 0;
                    inbuf[i] = '\0';
                }
                break;

            default:
                if (!stat.inArg) {
                    stat.inArg = 1;
                    argc++;
                    argv[argc - 1] = &inbuf[i];
                }
                break;
        }
    } while (!stat.done && ++i < INBUF_SIZE);

    if (stat.inQuote) {
        return 2;
    }

    if (argc < 1) {
        return 0;
    }

    if (!pCli->echo_disabled) {
        cli_printf("\r\n");
    }

    /*
    * Some comamands can allow extensions like foo.a, foo.b and hence
    * compare commands before first dot.
    */
    i = ((p = strchr(argv[0], '.')) == NULL) ? 0 :
        (p - argv[0]);
    command = lookup_command(argv[0], i);
    if (command == NULL) {
        return 1;
    }

    memset(pCli->outbuf, 0, OUTBUF_SIZE);
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

    cli_printf("\r\n");

    /* show matching commands */
    for (i = 0, n = 0, m = 0; i < MAX_COMMANDS && n < pCli->num_commands;
         i++) {
        if (pCli->commands[i]->name != NULL) {
            if (!strncmp(inbuf, pCli->commands[i]->name, *bp)) {
                m++;
                if (m == 1) {
                    fm = pCli->commands[i]->name;
                } else if (m == 2)
                    cli_printf("%s %s ", fm,
                               pCli->commands[i]->name);
                else
                    cli_printf("%s ",
                               pCli->commands[i]->name);
            }
            n++;
        }
    }

    /* there's only one match, so complete the line */
    if (m == 1 && fm) {
        n = strlen(fm) - *bp;
        if (*bp + n < INBUF_SIZE) {
            memcpy(inbuf + *bp, fm + *bp, n);
            *bp += n;
            inbuf[(*bp)++] = ' ';
            inbuf[*bp] = '\0';
        }
    }

    /* just redraw input line */
    cli_printf("%s%s", PROMPT, inbuf);
}

/* Get an input line.
*
* Returns: 1 if there is input, 0 if the line should be ignored. */
static int get_input(char *inbuf, unsigned int *bp)
{
    if (inbuf == NULL) {
        cli_printf("inbuf_null\r\n");
        return 0;
    }

    while (cli_getchar(&inbuf[*bp]) == 1) {
        if (inbuf[*bp] == RET_CHAR) {
            continue;
        }
        if (inbuf[*bp] == END_CHAR) {   /* end of input line */
            inbuf[*bp] = '\0';
            *bp = 0;
            return 1;
        }

        if ((inbuf[*bp] == 0x08) || /* backspace */
            (inbuf[*bp] == 0x7f)) { /* DEL */
            if (*bp > 0) {
                (*bp)--;
                if (!pCli->echo_disabled) {
                    cli_printf("%c %c", 0x08, 0x08);
                }
            }
            continue;
        }

        if (inbuf[*bp] == '\t') {
            inbuf[*bp] = '\0';
            tab_complete(inbuf, bp);
            continue;
        }

        if (!pCli->echo_disabled) {
            cli_printf("%c", inbuf[*bp]);
        }

        (*bp)++;
        if (*bp >= INBUF_SIZE) {
            cli_printf("Error: input buffer overflow\r\n");
            cli_printf(PROMPT);
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
    if (cmd_string != NULL) {
        char *c = cmd_string;
        cli_printf("command '");
        while (*c != '\0') {
            if ((*c) >= 0x20 && (*c) <= 0x7f) {
                cli_printf("%c", *c);
            } else {
                cli_printf("\\0x%x", *c);
            }
            ++c;
        }
        cli_printf("' not found\r\n");
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
static void cli_main( void *data )
{
    while (!cliexit) {
        int ret;
        char *msg = NULL;

        if (get_input(pCli->inbuf, &pCli->bp)) {
            msg = pCli->inbuf;
#if 0
            if (strcmp(msg, EXIT_MSG) == 0) {
                break;
            }
#endif
            ret = handle_input(msg);
            if (ret == 1) {
                print_bad_command(msg);
            } else if (ret == 2) {
                cli_printf("syntax error\r\n");
            }

            cli_printf(PROMPT);
        }
    }

    cli_printf("CLI exited\r\n");
    yos_free(pCli);
    pCli = NULL;

    yos_task_exit(0);
}

static void task_Command( char *pcWriteBuffer, int xWriteBufferLen, int argc,
                          char **argv )
{
    dumpsys_task_func( NULL, 0, 1);
}

static void tftp_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                         char **argv)
{
}

static void uptime_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                           char **argv)
{
    cli_printf("UP time %ldms\r\n", yos_now_ms());
}

//extern void tftp_ota( void );
void tftp_ota_thread( void *arg )
{
    //tftp_ota( );
    yos_task_exit(0);
}

void ota_Command( char *pcWriteBuffer, int xWriteBufferLen, int argc,
                  char **argv )
{
    yos_task_new("LOCAL OTA", tftp_ota_thread, 0, 4096);
}

void help_command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                  char **argv);


void devname_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                       char **argv)
{
    cli_printf("%s\r\n", SYSINFO_DEVICE_NAME);
}

void dumpsys_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                     char **argv)
{
    dumpsys_func(pcWriteBuffer, xWriteBufferLen, argc, argv);
}

void memory_show_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                         char **argv)
{
    dumpsys_mm_info_func(NULL, 0);
}

void memory_dump_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                         char **argv)
{
    cli_printf("memory_dump_Command\r\n");
}

void memory_set_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                        char **argv)
{
    cli_printf("memory_set_Command\r\n");
}

void memp_dump_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                       char **argv)
{
    cli_printf("memp_dump_Command\r\n");
}

void get_version(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                 char **argv)
{
    cli_printf("%s\r\n", SYSINFO_OS_VERSION);
}

void reboot(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    FUNCPTR reboot = 0;

    cli_printf("reboot\r\n");

    hal_reboot();
}

void wifi_debug_mode_command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                             char **argv)
{
    hal_wifi_start_debug_mode();
}

static void echo_cmd_handler(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                             char **argv)
{
    if (argc == 1) {
        cli_printf("Usage: echo on/off. Echo is currently %s\r\n",
                   pCli->echo_disabled ? "Disabled" : "Enabled");
        return;
    }

    if (!strcmp(argv[1], "on")) {
        cli_printf("Enable echo\r\n");
        pCli->echo_disabled = 0;
    } else if (!strcmp(argv[1], "off")) {
        cli_printf("Disable echo\r\n");
        pCli->echo_disabled = 1;
    }
}

static void cli_exit_handler(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                             char **argv)
{
    // exit command not executed
    cliexit = 1;
    return;
}


static const struct cli_command built_ins[] = {
    {"help", NULL, help_command},
    {"version", NULL, get_version},
    {"echo", NULL, echo_cmd_handler},
    {"exit", "CLI exit", cli_exit_handler},


    // os
    {"tasklist", "list all thread name status", task_Command},

    // others
    {"devname", "print device name", devname_Command},
    {"dumpsys", "dump system information", dumpsys_Command},
    {"memshow", "print memory information", memory_show_Command},
    {"memdump", "<addr> <length>", memory_dump_Command},
    {"memset", "<addr> <value 1> [<value 2> ... <value n>]", memory_set_Command},
    {"memp", "print memp list", memp_dump_Command},

    {"reboot", "reboot system", reboot},

    {"tftp",     "tftp",                        tftp_Command},
    {"time",     "system time",                 uptime_Command},
    {"ota",      "system ota",                  ota_Command},
    { "wifi_debug", "wifi debug mode", wifi_debug_mode_command},
};

/* Built-in "help" command: prints all registered commands and their help
* text string, if any. */
void help_command(char *pcWriteBuffer, int xWriteBufferLen, int argc,
                  char **argv)
{
    int i, n;
    uint32_t build_in_count = sizeof(built_ins) / sizeof(struct cli_command);

#if (DEBUG)
    build_in_count++;
#endif

    cli_printf( "====Build-in Commands====\r\n" );
    for (i = 0, n = 0; i < MAX_COMMANDS && n < pCli->num_commands; i++) {
        if (pCli->commands[i]->name) {
            cli_printf("%s: %s\r\n", pCli->commands[i]->name,
                       pCli->commands[i]->help ?
                       pCli->commands[i]->help : "");
            n++;
            if ( n == build_in_count - 1 ) {
                cli_printf("\r\n====User Commands====\r\n");
            }
        }
    }
}


int cli_register_command(const struct cli_command *command)
{
    int i;
    if (!command->name || !command->function) {
        return 1;
    }

    if (pCli->num_commands < MAX_COMMANDS) {
        /* Check if the command has already been registered.
        * Return 0, if it has been registered.
        */
        for (i = 0; i < pCli->num_commands; i++) {
            if (pCli->commands[i] == command) {
                return 0;
            }
        }
        pCli->commands[pCli->num_commands++] = command;
        return 0;
    }

    return 1;
}

int cli_unregister_command(const struct cli_command *command)
{
    int i;
    if (!command->name || !command->function) {
        return 1;
    }

    for (i = 0; i < pCli->num_commands; i++) {
        if (pCli->commands[i] == command) {
            pCli->num_commands--;
            int remaining_cmds = pCli->num_commands - i;
            if (remaining_cmds > 0) {
                memmove(&pCli->commands[i], &pCli->commands[i + 1],
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
        if (cli_register_command(commands++)) {
            return 1;
        }
    return 0;
}

int cli_unregister_commands(const struct cli_command *commands,
                            int num_commands)
{
    int i;
    for (i = 0; i < num_commands; i++)
        if (cli_unregister_command(commands++)) {
            return 1;
        }

    return 0;
}

__attribute__ ((weak)) int board_cli_init(void)
{
    return 0;
}

int yos_cli_stop(void)
{
    cliexit = 1;
    return 0;
}

int yos_cli_init(void)
{
    int ret;
    yos_task_t task;

    pCli = (struct cli_st *)yos_malloc(sizeof(struct cli_st));
    if (pCli == NULL) {
        return YUNOS_NO_MEM;
    }

    memset((void *)pCli, 0, sizeof(struct cli_st));

    /* add our built-in commands */
    if (cli_register_commands(&built_ins[0],
                              sizeof(built_ins) /
                              sizeof(struct cli_command))) {
        goto init_general_err;
    }

    ret = yos_task_new_ext(&task,"cli", cli_main, 0, 4096, YOS_DEFAULT_APP_PRI);
    if (ret != YUNOS_SUCCESS) {
        cli_printf("Error: Failed to create cli thread: %d\r\n",
                   ret);
        goto init_general_err;
    }

    pCli->initialized = 1;
    pCli->echo_disabled = 0;

    board_cli_init();

    return YUNOS_SUCCESS;

init_general_err:
    if (pCli) {
        yos_free(pCli);
        pCli = NULL;
    }

    return YUNOS_SYS_FATAL_ERR;
}


/* ========= CLI input&output APIs ============ */
int cli_printf(const char *msg, ...)
{
    va_list ap;
    char *pos, message[256];
    int sz;
    int nMessageLen = 0;

    memset(message, 0, 256);
    pos = message;

    sz = 0;
    va_start(ap, msg);
    nMessageLen = vsnprintf(pos, 256 - sz, msg, ap);
    va_end(ap);

    if ( nMessageLen <= 0 ) {
        return 0;
    }

    cli_putstr((const char *)message);
    return 0;
}


int cli_putstr(const char *msg)
{
    if (msg[0] != 0) {
        hal_uart_send(STDIO_UART, (const char *)msg, strlen(msg) );
    }

    return 0;
}

int cli_getchar(char *inbuf)
{
    if (hal_uart_recv(STDIO_UART, inbuf, 1, NULL, 0xFFFFFFFF) == 0) {
        return 1;
    } else {
        return 0;
    }
}


