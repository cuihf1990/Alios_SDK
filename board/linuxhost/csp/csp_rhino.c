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
#include <k_api.h>
#include <csp.h>

#ifndef WITH_LWIP
#define lwip_write write
#define lwip_eventfd eventfd
#endif

static void sem_notify_cb(blk_obj_t *obj, kobj_set_t *handle)
{
    int fd = (int)(long)handle->docker;
    uint64_t val = 1;

    lwip_write(fd, &val, sizeof val);
}

static int create_eventfd(csp_sem_t sem, kobj_set_t *handle)
{
    int fd = lwip_eventfd(0, 0);

    handle->notify = sem_notify_cb;
    handle->docker = (void *)fd;
    ((blk_obj_t *)sem.hdl)->handle = handle;

    sem_count_t count;
    yunos_sem_count_get(sem.hdl, &count);
    if (count > 0)
        sem_notify_cb((blk_obj_t *)sem.hdl, handle);

    return fd;
}

static void remove_eventfd(csp_sem_t sem, kobj_set_t *handle)
{
    int fd = (int)(long)handle->docker;
    close(fd);
    ((blk_obj_t *)sem.hdl)->handle = NULL;
}

#ifdef WITH_LWIP
#include "lwip/sys.h"

extern void hw_start_hal(void);

static int csp_select(int maxfds, fd_set *rfds, fd_set *wfds, uint32_t timeout, csp_sem_t sem)
{
    struct timeval tv = {
        .tv_sec = timeout / 1024,
        .tv_usec = (timeout % 1024) * 1024,
    };
    kobj_set_t obj_handle;
    int ret;

    int efd = create_eventfd(sem, &obj_handle);
    if (efd >= maxfds)
        maxfds = efd + 1;
    FD_SET(efd, rfds);

    ret = lwip_select(maxfds, rfds, wfds, NULL, &tv);
    remove_eventfd(sem, &obj_handle);

    return ret;
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

    ret = csp_select(maxfds+1, &rfds, &wfds, timeout, sem);
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
#else

int csp_poll(struct pollfd *pollfds, int nfds, csp_sem_t sem,
                  uint32_t timeout)
{
    kobj_set_t obj_handle;
    int ret;
    int sz = sizeof(struct pollfd) * (nfds + 1);
    struct pollfd *pfds = malloc(sz);
    struct pollfd *evt_fdp = pfds + nfds;

    bzero(pfds, sz);
    memcpy(pfds, pollfds, sz - sizeof(struct pollfd));
    evt_fdp->fd = create_eventfd(sem, &obj_handle);
    evt_fdp->events = POLLIN;

    ret = poll(pfds, nfds+1, timeout);

    memcpy(pollfds, pfds, sz - sizeof(struct pollfd));

    remove_eventfd(sem, &obj_handle);

    return ret;
}
#endif

void hal_arch_time_msleep(int ms)
{
    yunos_task_sleep(ms * YUNOS_CONFIG_TICKS_PER_SECOND / 1000);
}

unsigned long hal_arch_get_time_counter(void)
{
    return 0;
}

void csp_os_init(void)
{
    hw_start_hal();

    yunos_init();
}

void csp_os_start(void)
{
    yunos_start();
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

