#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <aos/aos.h>
#include <sal.h>
#include <sal_arch.h>
#include <sal_def.h>
#include <sal_ipaddr.h>
#include <sal_sockets.h>
#include <sal_err.h>
#include <hal/soc/atcmd.h>
#include <atparser.h>
#include <netmgr.h>

#define TAG "salapp"
#define SALAPP_BUFFER_MAX_SIZE  1512


static void yloop_action(void *arg);

static void yloop_action(void *arg)
{
    LOG("Hello, %s");
    aos_post_delayed_action(5000, yloop_action, NULL);
}

static void handle_sal(char *pwbuf, int blen, int argc, char **argv)
{
    char *ptype = argc > 1 ? argv[1] : "default";
    char buf[SALAPP_BUFFER_MAX_SIZE] = {0};
    int  readlen = 0;
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
        while(1){
            readlen = read(fd, buf, SALAPP_BUFFER_MAX_SIZE - 1);
            if (readlen < 0){
                LOGE(TAG, "recv failed, errno = %d.", errno);
                close(fd);
                return;
            }

            if (readlen == 0){
                continue;
            }
            
            LOGD(TAG, "recv server reply len %d info %s \n", readlen, buf);
            if (strstr(buf, pdata)){
                LOGI(TAG, "Goodbye! See you! (%d)\n", fd);
                break;
            }
        }
        
        close(fd);
        LOGI(TAG, "sal tcp_c test successful.");
    } else if (strcmp(ptype, "yloop") == 0) {
        aos_schedule_call(yloop_action,NULL);
    } else if (strcmp(ptype, "otaapi") == 0) {
        char domain[] = "www.baidu.com";
        int port = 8080;
        struct hostent *host;
        struct timeval timeout;
        int fd;
        struct sockaddr_in server_addr;

        if ((host = gethostbyname(domain)) == NULL) {
            LOGE(TAG, "gethostbyname failed, errno: %d", errno);
            return;
        }

        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            LOGE(TAG, "Socket failed, errno: %d", errno);
            return;
        }

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        if (setsockopt (fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                        sizeof(timeout)) < 0) {
            LOGE(TAG, "setsockopt failed, errno: %d", errno);
            close(fd);
        }

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);

        if (connect(fd, (struct sockaddr *) (&server_addr),
            sizeof(struct sockaddr)) == -1) {
            LOGE(TAG, "Connect failed, errno: %d", errno);
            close(fd);
            return;
        }

        close(fd);
        LOGI(TAG, "sal otaapi test successful.");
    } else if (strcmp(ptype, "mbedtlsapi") == 0) {
        struct addrinfo hints, *addr_list;
        int proto = 0; // 0 - tcp, 1 - udp
        char nodename[] = "www.baidu.com";
        char *port = "8080";
        int fd, ret;

        memset( &hints, 0, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = proto ? SOCK_DGRAM : SOCK_STREAM;
        hints.ai_protocol = proto ? IPPROTO_UDP : IPPROTO_TCP;

        // getaddrinfo only returns 1 addr
        if (getaddrinfo(nodename, port, &hints, &addr_list) != 0) {
            LOGE(TAG, "getaddrinfo faied, errno: %d", errno);
            return;
        }

        fd = socket(addr_list->ai_family, addr_list->ai_socktype,
                    addr_list->ai_protocol);
        if (fd < 0) {
            LOGE(TAG, "socket failed, errno: %d", errno);
            return;
        }

        do {
            ret = connect(fd, addr_list->ai_addr, addr_list->ai_addrlen);
            if (ret == -1) {
                close(fd);
                freeaddrinfo(addr_list);
            } else {
                if (errno == EINTR) {
                    continue;
                }
                break;
           }
        } while(1);

        close(fd);
        freeaddrinfo(addr_list);
    }
}

static struct cli_command salcmds[] = {
    {
        .name = "sal",
        .help = "sal tcp_c|udp_c|yloop|otaapi|mbedtlsapi [remote_ip remote_port data]",
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
