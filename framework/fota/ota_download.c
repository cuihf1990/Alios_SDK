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

#include "ota_update_manifest.h"

#define BUFFER_MAX_SIZE 1024

/**
 * @brief http_string_strchr 搜索字符串右边起的第一个匹配字符
 *
 * @Param: s  原始字符串
 * @Param: x  待匹配的字符
 *
 * Returns: 索到即返回x对应的值，否则返回0
 */
char* http_string_strchr(char* s, char x) {
    int i = strlen(s);
    if (!(*s)) {
        return 0;
    }
    while (s[i - 1]) {
        if (strchr(s + (i - 1), x)) {
            return (s + (i - 1));
        } else {
            i--;
        }
    }
    return 0;
}
/**
 * @brief http_string_lower 字符串转换为全小写
 *
 * @Param: s  原始字符串
 */
void http_string_lower(char* s) {
    char *p = s;
    while (*p && *p != '\0') {
        if (*p > 'A' && *p < 'Z')
            *p = *p + 32;
        p++;
    }
}
/**
 * @brief http_gethost_info 字符串src中分析出网站地址和端口，并得到用户要下载的文件
 *
 * @Param: src  输入字符串
 * @Param: web  WEB地址
 * @Param: file  需要下载的文件
 * @Param: port  WEB端口号，默认为80
 */
void http_gethost_info(char* src, char* web, char* file, int* port) {
    char* pa;
    char* pb;

    if(!src || !web || !file) {
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
    } else if (!strncmp(pa, "https://", strlen("https://"))) {
        pa = src + strlen("https://");
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
 * @brief http_socket_init 初始化套接字
 *
 * @Param: port
 * @Param: host_addr
 *
 * Returns: 返回套接字描述符
 */
int http_socket_init(int port, char *host_addr) {
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



int http_download(char *url, write_flash_cb_t func) {
    int sockfd = 0;
    char buffer[BUFFER_MAX_SIZE] = {0};
    int port = 0;
    int nbytes = 0;
    char host_file[128] = {0};
    char host_addr[256] = {0};
    char request[BUFFER_MAX_SIZE] = {0};
    int send = 0;
    int totalsend = 0;
    int i = 0;
    //char *pt = NULL;
    char psave[BUFFER_MAX_SIZE] = {0};;
    size_t index = 0;

    OTA_LOG_I("parameter.1 is: %s\n ", url);
    http_gethost_info(url, host_addr, host_file, &port);/*分析网址、端口、文件名等*/

    sockfd = http_socket_init(port, host_addr);
    if(sockfd < 0 )
    {
        OTA_LOG_E("http_socket_init error\n ");
        return -1;
    }
    sprintf(request,
            "GET   /%s   HTTP/1.1\r\nAccept:   */*\r\nAccept-Language:   zh-cn\r\n"
                    "User-Agent:   Mozilla/4.0   (compatible;   MSIE   5.01;   Windows   NT   5.0)\r\n"
                    "Host:   %s:%d\r\nConnection:   Close\r\n\r\n ", host_file,
            host_addr, port);
    OTA_LOG_I("%s\n", request);/*准备request，将要发送给主机*/
//    /*取得真实的文件名*/
//    if (*host_file) {
//        pt = http_string_strchr(host_file, '/');
//    } else {
//        pt = 0;
//    }
    /*发送http请求request*/
    send = 0;
    totalsend = 0;
    nbytes = strlen(request);
    while (totalsend < nbytes) {
        send = write(sockfd, request + totalsend, nbytes - totalsend);
        if (send == -1) {
            OTA_LOG_E("send error!%s\n ", strerror(errno));
            exit(0);
        }
        totalsend += send;
        OTA_LOG_I("%d bytes send OK!\n ", totalsend);
    }

    i = 0;
    /*连接成功了，接收http响应,每次处理4096个字节*/
    memset(psave, 0, BUFFER_MAX_SIZE);
    while ((nbytes = read(sockfd, buffer, 1)) == 1) {
        if (i < 4) {
            /*这里处理http头部*/
            if (buffer[0] == '\r' || buffer[0] == '\n') {
                i++;
            } else {
                i = 0;
            }
            OTA_LOG_I("%c", buffer[0]);/*把http头信息打印在屏幕上*/
        } else /*如果结尾部分不为\r\n\r\n则表示头接收完毕，下面是请求内容*/
        {
            psave[index++] = buffer[0];
            if (index > BUFFER_MAX_SIZE) {
                func(BUFFER_MAX_SIZE, (uint8_t *)psave, BUFFER_MAX_SIZE, 0);
                memset(psave, 0, BUFFER_MAX_SIZE);
                index = 0;
            }
        }
    }
    /*将剩余的字符写入文件*/
    if (index <= BUFFER_MAX_SIZE) {
        func(BUFFER_MAX_SIZE, (uint8_t*)psave, index, 0);
    }
    close(sockfd);

    return 0;
}

