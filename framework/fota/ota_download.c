/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <yos/network.h>
#include "ota_constants.h"
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
void http_gethost_info(char* src, char* web, char* file, int* port) {
    char* pa;
    char* pb;

    if(!src || strlen(src) == 0) {
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

static int _ota_socket_check_conn(int sock) {
    struct pollfd fd = { .fd = sock, .events = POLLOUT };
    int ret = 0;
    socklen_t len = 0;

    while (poll(&fd, 1, -1) == -1) {
        if (errno != EINTR ){
             return -1;
        }
    }

    len = sizeof(ret);
    if (getsockopt (sock, SOL_SOCKET, SO_ERROR, &ret, &len) == -1 ||
        ret != 0) {
        return -1;
    }
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
int http_socket_init(int port, char *host_addr) {
    if(host_addr == NULL || strlen(host_addr) == 0 || port <= 0)
    {
        OTA_LOG_E("http_socket_init parms   error\n ");
        return -1;
    }
    struct sockaddr_in server_addr;
    struct hostent *host;
    int sockfd;
    if ((host = gethostbyname(host_addr)) == NULL)/*取得主机IP地址*/
    {
        OTA_LOG_E("Gethostname   error,   %s\n ", strerror(errno));
        return -1;
    }
    /*   客户程序开始建立   sockfd描述符   */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)/*建立SOCKET连接*/
    {
        OTA_LOG_E("Socket   Error:%s\a\n ", strerror(errno));
        return -1;
    }
    /*   客户程序填充服务端的资料   */
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr*) host->h_addr);
    /*   客户程序发起连接请求   */
    if (connect(sockfd, (struct sockaddr*) (&server_addr), sizeof(struct sockaddr)) == -1)/*连接网站*/
    {
        OTA_LOG_E("socket connecting %s failed!\n",  strerror(errno));
        if (errno != EINTR)
            goto err_out;
        if (_ota_socket_check_conn(sockfd) < 0)
            goto err_out;
    }
    return sockfd;

err_out:
    close(sockfd);
    return -1;
}

static MD5_CTX            g_ctx;

int check_md5(const char *buffer, const int32_t len)
{
    unsigned char digest[16] = {0};
    MD5Final((unsigned char*)digest, &g_ctx);
    char digest_str[33] = {0}; 
    int i= 0;
    for(; i< 16 ;i++) {
        snprintf(digest_str + i*2, 2+1, "%2X", digest[i]);
    }
    //OTA_LOG_D("digestMD5=%s",digest_str);
    if (strncmp(digest_str, buffer, 32)) {
        OTA_LOG_E("Download update_packet FAIL!");
        return 0;
    }
    return 1;
}


int http_download(char *url, write_flash_cb_t func) {
    if(!url || strlen(url) == 0 || func == NULL) {
        OTA_LOG_E("http_download error!\n");
        return -1;
    }
    int sockfd = 0;
    char http_buffer[BUFFER_MAX_SIZE] = {0};
    int port = 0;
    int nbytes = 0;
    char host_file[128] = {0};
    char host_addr[256] = {0};
    int send = 0;
    int totalsend = 0;
    int i = 0;
  

    OTA_LOG_I("parameter.1 is: %s\n ", url);
    http_gethost_info(url, host_addr, host_file, &port);

    sockfd = http_socket_init(port, host_addr);
    if(sockfd < 0 )
    {
        OTA_LOG_E("http_socket_init error\n ");
        return -1;
    }
    sprintf(http_buffer,
            "GET   /%s   HTTP/1.1\r\nAccept:   */*\r\nAccept-Language:   zh-cn\r\n"
                    "User-Agent:   Mozilla/4.0   (compatible;   MSIE   5.01;   Windows   NT   5.0)\r\n"
                    "Host:   %s:%d\r\nConnection:   Close\r\n\r\n ", host_file,
            host_addr, port);
    OTA_LOG_I("%s\n", http_buffer);
    send = 0;
    totalsend = 0;
    nbytes = strlen(http_buffer);
    while (totalsend < nbytes) {
        send = write(sockfd, http_buffer + totalsend, nbytes - totalsend);
        if (send == -1) {
            OTA_LOG_E("send error!%s\n ", strerror(errno));
            return -1;
        }
        totalsend += send;
        OTA_LOG_I("%d bytes send OK!\n ", totalsend);
    }

    i = 0;
    /*连接成功了，接收http响应,每次处理1024个字节*/
    int size = 0; 
    MD5Init(&g_ctx);
    int buffer_size = 1;
    memset(http_buffer, 0, sizeof http_buffer);
    while ((nbytes = read(sockfd, http_buffer, buffer_size))) {
        if (i < 4) {
            /* process head packet*/
            if (http_buffer[0] == '\r' || http_buffer[0] == '\n') {
                i++;
                if(i == 4) {
                    buffer_size = BUFFER_MAX_SIZE;
                }
            } else {
                i = 0;
            }
        } else /*if buffer not ended with \r\n\r\n，request packet content*/
        {
            size +=nbytes;
	    //OTA_LOG_E("size %d, %d", size,count);
	    MD5Update(&g_ctx, (unsigned char*)http_buffer, nbytes);
	    func(BUFFER_MAX_SIZE, (uint8_t *)http_buffer, nbytes, 0);
	    memset(http_buffer, 0, BUFFER_MAX_SIZE);
        }
    }
    
    close(sockfd);
    return 0;
}

