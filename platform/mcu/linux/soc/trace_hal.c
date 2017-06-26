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

#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fifo.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "trace_hal.h"

static int trace_is_started;

#if 0

void *trace_hal_init(void)
{
    int fh1;

    if (trace_is_started == 0) {
        fh1 = open("trace_test", O_CREAT | O_RDWR,
                S_IROTH | S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP | S_IWOTH);

        trace_is_started = 1;
        return (void *)fh1;
    }

    yunos_task_dyn_del(NULL);

    return 0;
}


ssize_t trace_hal_send(void *handle, void *buf, size_t len)
{
     return write((int)handle, buf, len);
}


ssize_t trace_hal_recv(void *handle, void *buf)
{

    return 0;
}

void trace_hal_deinit(void *handle)
{
    int fh1;

    fh1 = (int)handle;
    close(fh1);
}

#else

#define DEFAULT_PORT    8000

void *trace_hal_init(void)
{
    int sockfd;

    struct sockaddr_in servaddr;

    if (trace_is_started == 0) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
            exit(0);
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(8000);

        inet_aton("127.0.0.1", &servaddr.sin_addr);

        if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
            printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
            exit(0);
        }

        trace_is_started = 1;
        return (void *)sockfd;
    }

    yunos_task_dyn_del(NULL);

    return 0;
}


ssize_t trace_hal_send(void *handle, void *buf, size_t len)
{
    int len_send;
    int len_total_send;

    len_send = 0;
    len_total_send = 0;

    while (1) {
        len_send = send((int)handle, (buf + len_total_send) , len - len_total_send, 0);

        if (len_send < 0) {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            exit(0);
        }

        len_total_send += len_send;
        len -= len_send;

        if (len == 0) {
            break;
        }
    }

    return len_send;
}

ssize_t trace_hal_recv(void *handle, void *buf)
{
    return 0;
}

void trace_hal_deinit(void *handle)
{
    int fh1;

    fh1 = (int)handle;
    close(fh1);
}

#endif

