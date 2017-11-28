#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <aos/aos.h>
#include <sal.h>
#include <sal_sockets.h>
#include <sal_err.h>
#include <hal/soc/atcmd.h>
#include <atparser.h>
#include <netmgr.h>

#define TAG "salapp"

static void yloop_action(void *arg);

static void yloop_action(void *arg)
{
    LOG("Hello, %s");
    aos_post_delayed_action(5000, yloop_action, NULL);
}

static void handle_sal(char *pwbuf, int blen, int argc, char **argv)
{
    char *ptype = argc > 1 ? argv[1] : "default";

    /* TCP client case */
    if (strcmp(ptype, "tcp_c") == 0) {
        char *pip, *pport, *pdata;
        ssize_t siz;
        int fd;
        struct sockaddr_in addr;
        fd = socket(AF_INET,SOCK_STREAM,0);

        pip = argv[2];
        pport = argv[3];
        pdata = argv[4];

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons((short)atoi(pport));
        addr.sin_addr.s_addr = inet_addr(pip);

        if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
            LOGE(TAG, "Connect failed, errno = %d", errno);
            close(fd);
            return;
        }

        // send-recv
        if ((siz = send(fd, pdata, strlen(pdata), 0)) <= 0) {
            LOGE(TAG, "send failed, errno = %d.", errno);
            close(fd);
            return;
        }

        aos_msleep(10000);

        close(fd);
    } else if (strcmp(ptype, "yloop") == 0) {
        yloop_action(NULL);
    }
}

static struct cli_command salcmds[] = {
    {
        .name = "sal",
        .help = "sal [tcp_c|udp_c] remote_ip remote_port data",
        .function = handle_sal
    }
};

static void wifi_event_handler(input_event_t *event, void *priv_data)
{
    if (event->type != EV_WIFI) return;
    if (event->code == CODE_WIFI_ON_PRE_GOT_IP)
        LOG("Hello, WiFi PRE_GOT_IP event!");
    if (event->code == CODE_WIFI_ON_GOT_IP)
        LOG("Hello, WiFi GOT_IP event!");
}

int application_start(int argc, char *argv[])
{
    uart_dev_t uart_1;

    printf("Hello app started\r\n");

    aos_set_log_level(AOS_LL_DEBUG);

    // AT UART init
    uart_1.port                = AT_UART_PORT;
    uart_1.config.baud_rate    = AT_UART_BAUDRATE;
    uart_1.config.data_width   = AT_UART_DATA_WIDTH;
    uart_1.config.parity       = AT_UART_PARITY;
    uart_1.config.stop_bits    = AT_UART_STOP_BITS;
    uart_1.config.flow_control = AT_UART_FLOW_CONTROL;

    if (at.init(&uart_1, AT_RECV_DELIMITER, AT_SEND_DELIMITER, 1000) != 0)
        return -1;

    at.set_mode(ASYN);

    sal_init();

    aos_register_event_filter(EV_WIFI, wifi_event_handler, NULL);

    aos_cli_register_commands((const struct cli_command *)&salcmds[0],
      sizeof(salcmds) / sizeof(salcmds[0]));

    netmgr_init();
    netmgr_start(false);

    aos_loop_run();

    return 0;
}
