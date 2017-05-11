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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#include "csp.h"
#include "lwip/sys.h"

extern void hw_start_hal(void);

int csp_mutex_new(csp_mutex_t *mutex)
{
    return sys_sem_new((sys_sem_t *)mutex, 1);
}

uint32_t csp_mutex_lock(csp_mutex_t mutex)
{
    sys_sem_wait((sys_sem_t *)&mutex);
    return 0;
}

uint32_t csp_mutex_unlock(csp_mutex_t mutex)
{
    sys_sem_signal((sys_sem_t *)&mutex);
    return 0;
}

void csp_mutex_free(csp_mutex_t *mutex)
{
    sys_sem_free((sys_sem_t *)mutex);
}

int csp_sem_new(csp_sem_t *sem, int32_t count)
{
    return sys_sem_new((sys_sem_t *)sem, count);
}

uint32_t csp_sem_wait(csp_sem_t sem, uint32_t timeout)
{
    sys_arch_sem_wait((sys_sem_t *)&sem, timeout);
    return 0;
}

void csp_sem_signal(csp_sem_t sem)
{
    sys_sem_signal((sys_sem_t *)&sem);
}

void csp_sem_free(csp_sem_t *sem)
{
    sys_sem_free((sys_sem_t *)sem);
}

extern int lwip_select2(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout, sys_sem_t *psem);
static int csp_select(int maxfds, void *rfds, void *wfds, void *exceptfds, uint32_t timeout, csp_sem_t sem)
{
    struct timeval tv = {
        .tv_sec = timeout / 1024,
        .tv_usec = (timeout % 1024) * 1024,
    };
    return lwip_select2(maxfds, rfds, wfds, NULL, &tv, (sys_sem_t *)&sem);
}

int csp_poll(struct pollfd *pollfds, int nfds, csp_sem_t sem, uint32_t timeout)
{
    int maxfds = 0;
    int i;
    fd_set rfds, wfds;
    int ret;

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);

    for (i=0;i<nfds;i++) {
        struct pollfd *pfd = pollfds + i;
        FD_CLR(pfd->fd, &rfds);
        FD_CLR(pfd->fd, &wfds);
        if (pfd->fd > maxfds)
            maxfds = pfd->fd;

        if (pfd->events & POLLIN)
            FD_SET(pfd->fd, &rfds);
        if (pfd->events & POLLOUT)
            FD_SET(pfd->fd, &wfds);
    }

    ret = csp_select(maxfds+1, &rfds, &wfds, NULL, timeout, sem);
    if (ret >= 0) {
        for (i=0;i<nfds;i++) {
            struct pollfd *pfd = pollfds + i;
            int events = 0;
            if (FD_ISSET(pfd->fd, &rfds))
                events |= POLLIN;
            if (FD_ISSET(pfd->fd, &wfds))
                events |= POLLOUT;
            if (!events) continue;

            pollfds[i].revents = events;
        }
    }

    return ret;
}

int csp_task_new_ext(const char *name, void (*fn)(void *), void *arg, int stacksize, int prio)
{
    pthread_t tid;
    int err = pthread_create(&tid, NULL, (void *(*)(void *))fn, arg);
    return err;
}

void csp_task_exit(int code)
{
    pthread_exit(NULL);
}

uint64_t csp_now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t ret = (((uint64_t) ts.tv_sec) * ((uint64_t) 1e9) +
            (uint64_t) ts.tv_nsec);
    return ret;
}

int csp_sys_reset()
{
    return 0xff;
}

int csp_sys_free(uint32_t *f)
{
    return 0;
}

void hal_arch_time_msleep(int ms)
{
    usleep(ms * 1000);
}

unsigned long hal_arch_get_time_counter(void)
{
    return 0;
}

void csp_os_init(void)
{
    hw_start_hal();
}

void csp_os_start(void)
{
    while (1) {
         sleep(5);
    }
}

void csp_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

uint32_t yoc_get_free_heap_size(void)
{
    return 0u;
}

void *yos_malloc(size_t size)
{
    return malloc(size);
}

void yos_free(void *mem)
{
    free(mem);
}

