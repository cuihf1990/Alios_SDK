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
#include <tfs.h>
#include <poll.h>
#include <yos/network.h>
#include "ota_constants.h"
#include "ota_util.h"
#include "ota_log.h"

typedef struct {
	OTA_STATUS_T status;
	OTA_ENUM_UPDATE_WAY update_way;
	char packet_ver[PACKET_VER_SIZE];

    void* cb_para;
} ota_info_t;

static ota_info_t g_ota_info_storage = {
        .status = E_OTA_IDLE,
        .update_way = E_OTA_SILENT,
};
static ota_info_t *g_ota_info = &g_ota_info_storage;

OTA_STATUS_T ota_get_status(void)
{
    return g_ota_info->status;
}

void ota_set_status(OTA_STATUS_T status)
{
    g_ota_info->status = status;
}

void ota_set_packet_version(char* ver, uint32_t len)
{
    OTA_LOG_D("version: %s", ver);
    strncpy(g_ota_info->packet_ver, ver, sizeof(g_ota_info->packet_ver));
    g_ota_info->packet_ver[len] = 0;
}


char* ota_get_id2(void) {


    static char id2[TFS_ID2_LEN+1] = {0};
    if (!strlen(id2)) {
        uint32_t len = TFS_ID2_LEN;
        if ( tfs_get_ID2((unsigned char *)id2, &len) < 0 ) {
            OTA_LOG_E("get_ID2 failed!");
        }
    }

    OTA_LOG_D("id2: %s", id2);
    return id2;
}

static int _ota_socket_check_conn(int sock) {
#ifdef CSP_LINUXHOST
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
#else
    return -1;
#endif
}

int ota_http_init(const char *server, int port)
{
    struct hostent *host;
    int sockfd;
    struct sockaddr_in serv_addr;
    int ret = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        OTA_LOG_E("Creat socket error\n");
        return -1;
    }

    host = gethostbyname(server);
    if (host == NULL)
    {
        OTA_LOG_E("Get host by name error\n");
        goto err_out;
    }

    OTA_LOG_E("Server IP:%s\n", (host->h_name));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);
    serv_addr.sin_port = htons(port);

    struct timeval tv;
    tv.tv_sec  = 10;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

    ret = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        OTA_LOG_E("socket connecting failed!\n");
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

int ota_write(int fd,void* buffer,int length)
{
    int bytes_left;
    int written_bytes;
    char *ptr;
    ptr = buffer;
    bytes_left = length;
    while (bytes_left > 0)
    {
        written_bytes = write(fd,ptr,bytes_left);
        if(written_bytes < 0){
            if (errno == EINTR)
                continue;
            return(-1);
        }

        bytes_left -= written_bytes;
        ptr += written_bytes;
    }
    return(0);
}

int ota_read(int fd,void *buffer,int length, int range)
{
    int bytes_left = length;
    int bytes_read = 0;
    int header_len = 0;
    char* ptail = NULL;
    char *ptr;
    ptr=buffer;
    bytes_left=length;
    OTA_LOG_I("ota_read in len:%d\n",  length);
    while(bytes_left>0)
    {
        OTA_LOG_I("ota_read bytes_left:%d\n", bytes_left);

        do {
            bytes_read=read(fd,ptr, bytes_left);
            if (bytes_read >= 0)
                break;
            OTA_LOG_D("ota_read this time read len :%d %d\n", bytes_read, errno);
            if (bytes_read < 0 && errno != EINTR)
            {
                return(-1);
            }
        } while(1);

        if(bytes_read==0)
             break;
        bytes_left-=bytes_read;
        ptr+=bytes_read;
        if(NULL == ptail){
            ptail = strstr(buffer, "\r\n\r\n");
            if(NULL != ptail) {
                header_len= ptail - (char*)buffer + 4;
            }
        }
        OTA_LOG_I("header_len :%d range :%d bytes readed:%d\n", header_len, range, length - bytes_left);
        if(header_len + range + 1 == length - bytes_left) {
                break;
        }
    }
    return(length-bytes_left);
}
