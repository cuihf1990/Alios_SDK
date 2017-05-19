#include <string.h>
#include <errno.h>
#include "log.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include "lwip/err.h"

#define HTTP_PACKET_MAX 512

static const char *TAG = "PAL_PLATFORM_NETWORK";

int pal_network_send(int sockfd, const char *buf, int len)
{
    int ret = 0;

    do {
        ret = send(sockfd, buf + ret, len,0);
        if (ret < 0 || ret == 0) {
            LOGE(TAG, "write EINTR errno=%d %s ret = %d", errno, strerror(errno), ret);
            if (errno == EINTR) {
                continue;
            }
            LOGE(TAG, "write socket error, ret = %d", ret);
            return -1;
        }
        len -= ret;
    } while (len != 0);

    return 0;
}

int pal_network_recv(int sockfd, char *buf, int *len)
{
    int ret = 0;
    int count = 0;

    do {
        ret = recv(sockfd, buf + count, HTTP_PACKET_MAX - count,0);
        if (ret < 0) {
            LOGE(TAG, "read EINTR errno=%d %s ret = %d", errno, strerror(errno), ret);
            if (errno == EINTR) {
                continue;
            }
            LOGE(TAG, "read socket error, ret = %d", ret);
            return -1;
        }
        count += ret;
        break;
    } while (1);

    if (len != NULL)
        *len = count;

    return 0;
}

int pal_network_close(int fd)
{
    return close(fd);
}

int pal_network_create(const char *server, int port)
{
    int ret = 0;
    int sockfd;
    struct hostent *host;
    struct sockaddr_in serv_addr;

    host = gethostbyname(server);
    if (host == NULL) {
        LOGE(TAG, "get host by name error");
        return -1;
    }


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        LOGE(TAG, "socket init error");
        return -1;
    }

    struct timeval tv;
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

    ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    if (ret < 0) {
        close(sockfd);
        LOGE(TAG, "connect error");
        return -1;
    }

    return sockfd;
}
