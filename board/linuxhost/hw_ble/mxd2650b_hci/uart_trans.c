/****************************************************************************
 * Copyright (C) 2015 The YunOS Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>//for flock
#include <pthread.h>

#include "uart_trans.h"

/*****************************************************
* Pre-processor Define
*****************************************************/
#define CONFIG_BLUETOOTH_DEV_PATH "/dev/ttyUSB0"

/*****************************************************
* Private Data Define
*****************************************************/
static int g_dev_fd = -1;
static uart_recv_cb_t g_uart_recv_cb = NULL;
static int g_poll_thread_run = 0;

/*****************************************************
* Private Function Define
*****************************************************/
static int setup_port(int fd)
{
    struct termio term_attr;

    /* Get current setting */
    if (ioctl(fd, TCGETA, &term_attr) < 0)
    {
        return -1;
    }

    term_attr.c_iflag &= ~(INLCR | IGNCR | ICRNL | ISTRIP);
    term_attr.c_iflag &= ~(IXON | IXOFF | IXANY);

    term_attr.c_oflag &= ~(OPOST | ONLCR | OCRNL);

    term_attr.c_lflag &= ~(ISIG | ECHO | ICANON | NOFLSH);

    term_attr.c_cflag &= ~CBAUD;
    term_attr.c_cflag |= CREAD | B115200;

    /* Set databits */
    term_attr.c_cflag &= ~(CSIZE);
    term_attr.c_cflag |= CS8;

    /* Set parity */
    term_attr.c_cflag &= ~(PARENB);

    /* Set stopbits */
    term_attr.c_cflag &= ~CSTOPB;

    term_attr.c_cc[VMIN] = 1;
    term_attr.c_cc[VTIME] = 0;

    if (ioctl(fd, TCSETAW, &term_attr) < 0)
    {
        return -1;
    }

    if (ioctl(fd, TCFLSH, 2) < 0)
    {
        return -1;
    }

    return 0;
}

static int port_adapter_open(void)
{
    int i,ret;
    char dev_path[] = CONFIG_BLUETOOTH_DEV_PATH;
    int slen = strlen(dev_path);
    
    for(i = 0; i < 2; i++)
    {
        g_dev_fd = open(dev_path, O_RDWR);
        
        if (g_dev_fd < 0)
        {
            dev_path[slen-1]++;
            continue;
        }
        else
        {
            //reopen check
            ret = flock(g_dev_fd, LOCK_EX|LOCK_NB);
            if (ret < 0)
            {
                close(g_dev_fd);
                g_dev_fd = -1;
                dev_path[slen-1]++;
                continue;
            }
        }

        if (g_dev_fd > 0)
            break;
    }

    printf("[BLE] open dev %s fd=%d\n", dev_path, g_dev_fd);
    
    if (g_dev_fd < 0)
    {
        return -1;
    }

    if (setup_port(g_dev_fd) < 0)
    {
        close(g_dev_fd);
        g_dev_fd = -1;
        return -1;
    }

    return 0;
}
static int port_adapter_close(void)
{
    if (g_dev_fd >= 0)
    {
        close(g_dev_fd);
    }

    g_dev_fd = -1;

    return 0;
}

static void *poll_task(void *arg)
{
    char buffer[128];
    int rlen = 0;
    int ret = 0;

    struct pollfd pfds[1];
    pfds[0].fd = g_dev_fd;
    pfds[0].events = POLLIN; //| POLLOUT;
    pfds[0].revents = 0;

    g_poll_thread_run = 1;
    while (g_poll_thread_run)
    {
        ret = poll(pfds, 1, 1000);
        if (ret > 0)
        {
            if (pfds[0].revents & POLLIN)
            {
                memset(buffer, 0, sizeof(buffer));
                rlen = read(g_dev_fd, buffer, sizeof(buffer));
                //printf("-------------Serial rlen=%d\n", rlen);

                g_uart_recv_cb((uint8_t *)buffer, rlen);
            }
        }
        else if (ret == 0)
        {
            //printf("poll timeout\n");
        }
        else
        {
            printf("poll error\n");
        }
    }

    port_adapter_close();
    return NULL;
}

/*****************************************************
* Public Function Define
*****************************************************/
int uart_trans_init(uart_recv_cb_t cb)
{
    int ret;
    pthread_t tid;

    //open serial
    if (port_adapter_open() < 0)
    {
        return -1;
    }

    //set callback
    g_uart_recv_cb = cb;

    //pthread_attr_t  attr;
    //pthread_attr_init(&attr);
    //attr.stacksize = stacksize;
    //attr.priority  = PTHREAD_DEFAULT_PRIORITY;

    //start task
    ret = pthread_create(&tid, NULL, (void *(*)(void *))poll_task, NULL);

    return ret;
}

int uart_trans_send(uint8_t *pdata, int len)
{
    int wlen = 0;
    wlen = write(g_dev_fd, pdata, len);
    return wlen;
}

int uart_trans_destroy(void)
{
    g_poll_thread_run = 0;
    return 0;
}
