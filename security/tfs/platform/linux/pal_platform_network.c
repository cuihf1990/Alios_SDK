/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include "log.h"

#define HTTP_PACKET_MAX 512

#define TAG_PAL_NETWORK "PAL_PLATFORM_NETWORK"

int pal_network_send(int sockfd, const char *buf, int len)
{
    int ret = 0;

    do {
        ret = send(sockfd, buf + ret, len,0);
        if (ret < 0 || ret == 0) {
            LOGD(TAG_PAL_NETWORK, "[%s]: write EINTR errno=%d %s ret = %d\n", __func__, errno, strerror(errno), ret);
            if (errno == EINTR) {
                continue;
            }
            LOGE(TAG_PAL_NETWORK, "[%s]: write socket error, ret = %d\n", __func__, ret);
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
            LOGD(TAG_PAL_NETWORK, "[%s]: read EINTR errno=%d %s ret = %d\n", __func__, errno, strerror(errno), ret);
            if (errno == EINTR) {
                continue;
            }
            LOGE(TAG_PAL_NETWORK, "[%s]: read socket error, ret = %d\n", __func__, ret);
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

int _http_check_conn(int sockfd) {
    struct pollfd fd = { .fd = sockfd, .events = POLLOUT };
    int ret = 0;
    socklen_t len = 0;

    LOGD(TAG_PAL_NETWORK, "[%s]: enter.\n", __func__);
    while (poll(&fd, 1, -1) == -1) {
        if (errno != EINTR ){
             LOGE(TAG_PAL_NETWORK, "[%s]: poll EINTR\n", __func__);
             return -1;
        }
    }
    len = sizeof(ret);
    if (getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &ret, &len) == -1) {
        LOGE(TAG_PAL_NETWORK, "[%s]: getsockopt error\n", __func__);
        return -1;
    }
    if (ret != 0) {
        LOGE(TAG_PAL_NETWORK, "[%s]: socket %d connect failed: %s\n", __func__, sockfd, strerror(ret));
        return -1;
    }

    return 0;
}

int pal_network_create(const char *server, int port)
{
    int ret = 0;
    int sockfd;
    struct hostent *host;
    struct sockaddr_in serv_addr;

    host = gethostbyname(server);
    if (host == NULL) {
        LOGE(TAG_PAL_NETWORK, "[%s]: get host by name error\n", __func__);
        return -1;
    }


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        LOGE(TAG_PAL_NETWORK, "[%s]: socket init error\n", __func__);
        return -1;
    }

    struct timeval tv;
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

    ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
    if (ret < 0) {
        if (errno == EINTR) {
            LOGE(TAG_PAL_NETWORK, "[%s]: connect EINTR errno=%d %s ret = %d\n", __func__, errno, strerror(errno), ret);
            if (_http_check_conn(sockfd) == 0) {
                return sockfd;
            }
        }
        close(sockfd);
        LOGE(TAG_PAL_NETWORK, "[%s]: connect error\n", __func__);
        return -1;
    }

    return sockfd;
}
