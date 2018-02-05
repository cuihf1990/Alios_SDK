/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <aos/aos.h>
#include <netmgr.h>

static void handle_test_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    aos_cli_printf("Hello world\r\n");
}

static void handle_ip_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    char ip[16] = {0};
    netmgr_wifi_get_ip(ip);
    aos_cli_printf("The IP address is %s\r\n", ip);
}

static struct cli_command ncmds[] = {
    {
        .name = "test",
        .help = "test",
        .function = handle_test_cmd
    },
    {
        .name = "ip",
        .help = "Get ip address.",
        .function = handle_ip_cmd
    }
};

int application_start(int argc, char *argv[])
{
    aos_cli_register_commands(&ncmds[0], sizeof(ncmds) / sizeof(ncmds[0]));
    netmgr_init();
    netmgr_start(false);
    aos_loop_run();
    return 0;
}

