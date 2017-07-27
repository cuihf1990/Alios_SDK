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

#include "yos/cli.h"
#include "stdarg.h"

#include "include.h"
#include "mem_pub.h"
#include "str_pub.h"
#include "uart_pub.h"
#include "mico_rtos.h"
#include <hal/base.h>
#include <hal/wifi.h>


#include "hal/soc/soc.h"

#define MICO_CLI_ENABLE
#define DEBUG 1

#ifdef MICO_CLI_ENABLE

/*
*  Command buffer API
*/
void wifiscan_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
	bk_wlan_start_scan();
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
	    bk_wlan_stop_monitor();
	}
	else
	{
	    bk_wlan_start_monitor();
	    bk_wlan_register_monitor_cb(cli_monitor_cb);
	    bk_wlan_set_channel(channel_num);
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

void driver_state_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("driver_state_Command\r\n");
}

void partShow_Command(char *pcWriteBuffer, int xWriteBufferLen, int argc, char **argv)
{
    os_printf("partShow_Command\r\n");
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

    {"wifidriver", "show wifi driver status", driver_state_Command}, // bus credite, flow control...

    {"flash",    "Flash memory map",            partShow_Command},

    {"GPIO", "GPIO <cmd> <arg1> <arg2>", Gpio_op_Command},
    {"GPIO_INT", "GPIO_INT <cmd> <arg1> <arg2>", Gpio_int_Command},
    {"FLASH", "FLASH <cmd(R/W/E)> <len>", flash_command_test},
    {"UART", "UART I <index>", Uart_command_test},

    {"txevm", "txevm [-m] [-c] [-l] [-r] [-w]", tx_evm_cmd_test},
    {"rxsens", "rxsens [-m] [-d] [-c] [-l]", rx_sens_cmd_test},
    {"rxsens", "rxsens [-m] [-d] [-c] [-l]", rx_sens_cmd_test},
};

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
		bk_wlan_register_monitor_cb(monitor);
        bk_wlan_start_monitor();
    }
    else if (!os_strcasecmp(argv[1], "off"))
    {
        cmd_printf("stop monitor\r\n");
        mico_debug_enabled = 0;
		bk_wlan_register_monitor_cb(NULL);
		bk_wlan_stop_monitor();
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
	bk_wlan_set_channel(channel);
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
        printf("invalid cmd\r\n");
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

int board_cli_init()
{
    cli_register_commands(&built_ins[0],
                              sizeof(built_ins) /
                              sizeof(struct cli_command));
    cli_register_commands(&user_clis[0],
                              sizeof(user_clis) /
                              sizeof(struct cli_command));

}

#endif

