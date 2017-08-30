/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <yos/yos.h>
#include <yos/network.h>

#include "ota_util.h"
#include "ota_log.h"
#include "md5.h"
#include "ota_update_manifest.h"

#define BUFFER_MAX_SIZE 512


/**
 * @brief http_gethost_info
 *
 * @Param: src  url
 * @Param: web  WEB
 * @Param: file  download filename
 * @Param: port  default 80
 */
void http_gethost_info(char *src, char *web, char *file, int *port)
{
    char *pa;
    char *pb;

    if (!src || strlen(src) == 0) {
        OTA_LOG_E("http_gethost_info parms error!\n");
        return;
    }

    *port = 0;
    if (!(*src)) {
        return;
    }
    pa = src;
    if (!strncmp(pa, "http://", strlen("http://"))) {
        pa = src + strlen("http://");
    }

    pb = strchr(pa, '/');
    if (pb) {
        memcpy(web, pa, strlen(pa) - strlen(pb));
        if (pb + 1) {
            memcpy(file, pb + 1, strlen(pb) - 1);
            file[strlen(pb) - 1] = 0;
        }
    } else {
        memcpy(web, pa, strlen(pa));
    }
    if (pb) {
        web[strlen(pa) - strlen(pb)] = 0;
    } else {
        web[strlen(pa)] = 0;
    }
    pa = strchr(web, ':');
    if (pa) {
        *port = atoi(pa + 1);
    } else {
        *port = 80;
    }
}

static int _ota_socket_check_conn(int sock)
{
#ifndef WITH_LWIP
    struct pollfd fd = { .fd = sock, .events = POLLOUT };
    int ret = 0;
    socklen_t len = 0;

    while (poll(&fd, 1, -1) == -1) {
        if (errno != EINTR ) {
            return -1;
        }
    }

    len = sizeof(ret);
    if (getsockopt (sock, SOL_SOCKET, SO_ERROR, &ret, &len) == -1 ||
        ret != 0) {
        return -1;
    }
#endif
    return 0;
}

/**
 * @brief http_socket_init
 *
 * @Param: port
 * @Param: host_addr
 *
 * Returns: socket fd
 */
int http_socket_init(int port, char *host_addr)
{
    if (host_addr == NULL || strlen(host_addr) == 0 || port <= 0) {
        OTA_LOG_E("http_socket_init parms   error\n ");
        return -1;
    }
    struct sockaddr_in server_addr;
    struct hostent *host;
    int sockfd;
    if ((host = gethostbyname(host_addr)) == NULL) { /*取得主机IP地址*/
        OTA_LOG_E("Gethostname   error,   %s\n ", strerror(errno));
        return -1;
    }
    /*   客户程序开始建立   sockfd描述符   */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { /*建立SOCKET连接*/
        OTA_LOG_E("Socket   Error:%s\a\n ", strerror(errno));
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                    sizeof(timeout)) < 0) {
        OTA_LOG_E("setsockopt failed\n");
        return -1;
    }

    /*   客户程序填充服务端的资料   */
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *) host->h_addr);
    /*   客户程序发起连接请求   */
    if (connect(sockfd, (struct sockaddr *) (&server_addr), sizeof(struct sockaddr)) == -1) { /*连接网站*/
        OTA_LOG_E("socket connecting %s failed!\n",  strerror(errno));
        if (errno != EINTR) {
            goto err_out;
        }
        if (_ota_socket_check_conn(sockfd) < 0) {
            goto err_out;
        }
    }
    return sockfd;

err_out:
    close(sockfd);
    return -1;
}

static MD5_CTX            g_ctx;

int check_md5(const char *buffer, const int32_t len)
{
    uint8_t digest[16] = {0};
    char digest_str[33] = {0};
    int i = 0;
    OTA_LOG_D("digest=%s", buffer);
    MD5_Final((uint8_t *)digest, &g_ctx);
    for (; i < 16 ; i++) {
        snprintf(digest_str + i * 2, 2 + 1, "%02X", digest[i]);
    }

    OTA_LOG_D("digestMD5=%s", digest_str);
    if (strncmp(digest_str, buffer, 32)) {
        OTA_LOG_E("update_packet md5 check FAIL!");
        return -1;
    }
    return 0;
}

#define HTTP_HEADER  "GET /%s HTTP/1.1\r\nAccept:*/*\r\n\
Accept-Language:zh-cn\r\n\
User-Agent:Mozilla/5.0\r\n\
Cache-Control: no-cache\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Connection: close\r\n\
Host:%s:%d\r\n\r\n"

int http_download(char *url, write_flash_cb_t func)
{
    if (!url || strlen(url) == 0 || func == NULL) {
        OTA_LOG_E("http_download url or func  error!\n");
        return OTA_DOWNLOAD_URL_FAIL;
    }
    int ret = 0;
    int sockfd = 0;
    char http_buffer[BUFFER_MAX_SIZE] = {0};
    int port = 0;
    int nbytes = 0;
    char host_file[128] = {0};
    char host_addr[256] = {0};
    int send = 0;
    int totalsend = 0;

    OTA_LOG_I("parameter is: %s\n ", url);
    http_gethost_info(url, host_addr, host_file, &port);

    sockfd = http_socket_init(port, host_addr);
    if (sockfd < 0 ) {
        OTA_LOG_E("http_socket_init error\n ");
        ret = OTA_DOWNLOAD_SOCKET_FAIL;
        return ret;
    }
    sprintf(http_buffer, HTTP_HEADER, host_file, host_addr, port);
    OTA_LOG_I("%s\n", http_buffer);
    send = 0;
    totalsend = 0;
    nbytes = strlen(http_buffer);
    while (totalsend < nbytes) {
        send = write(sockfd, http_buffer + totalsend, nbytes - totalsend);
        if (send == -1) {
            OTA_LOG_E("send error!%s\n ", strerror(errno));
            ret = OTA_DOWNLOAD_SEND_FAIL;
            goto DOWNLOAD_END;
        }
        totalsend += send;
        OTA_LOG_I("%d bytes send OK!\n ", totalsend);
    }

    /*连接成功了，接收http响应,每次处理1024个字节*/
    int size = 0;
    MD5_Init(&g_ctx);
    memset(http_buffer, 0, sizeof http_buffer);
    char headbuf[BUFFER_MAX_SIZE + 1] = {0};
    int header_found = 0;
    char *pos = 0;
    int file_size = 0;

    while ((nbytes = read(sockfd, http_buffer, BUFFER_MAX_SIZE))) {

        if (nbytes < 0) {
            if (errno != EINTR) {
                break;
            }
            if (_ota_socket_check_conn(sockfd) < 0) {
                OTA_LOG_E("download system error %s" , strerror(errno));
                break;
            } else {
                continue;
            }

        }

        if (!header_found) {
            if (!file_size) {
                char *ptr = strstr(http_buffer, "Content-Length:");
                if (ptr) {
                    //sscanf(ptr, "%*[^ ]%s", file_length);
                    //file_size = atoi(file_length);
                    sscanf(ptr, "%*[^ ]%d", &file_size);
                    OTA_LOG_I("file_length %d", file_size);
                }
            }

            pos = strstr(http_buffer, "\r\n\r\n");

            if (!pos) {
                //header pos
                memcpy(headbuf, http_buffer, BUFFER_MAX_SIZE);
            } else {
                int len = pos - http_buffer;
                header_found = 1;
                pos += 4;
                size = nbytes - len - 4;
                memcpy(headbuf, http_buffer, len);
                MD5_Update(&g_ctx, (const uint8_t *)pos, size);
                func(BUFFER_MAX_SIZE, (uint8_t *)pos, size, 0);
            }

            OTA_LOG_I("headbuf %s", headbuf);
            memset(headbuf, 0, sizeof headbuf);
            continue;
        }

        size += nbytes;
        OTA_LOG_I("size nbytes %d, %d", size, nbytes);
        MD5_Update(&g_ctx, (const uint8_t *) http_buffer, nbytes);
        func(BUFFER_MAX_SIZE, (uint8_t *) http_buffer, nbytes, 0);
        memset(http_buffer, 0, BUFFER_MAX_SIZE);

        if (size == file_size) {
            nbytes = 0;
            break;
        }

        if (ota_get_status() == OTA_CANCEL) {
            break;
        }
    }

    if (nbytes < 0) {
        OTA_LOG_E("download read error %s" , strerror(errno));
        ret = OTA_DOWNLOAD_FAILED;
    } else if (nbytes == 0) {
        ret = OTA_DOWNLOAD_FINISH;
    } else {
        ret = OTA_DOWNLOAD_CANCEL;
    }

DOWNLOAD_END:
    close(sockfd);
    return ret;
}

