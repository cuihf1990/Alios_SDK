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
#include <hal/timer.h>

#ifndef WITH_LWIP
static void sem_notify_cb(blk_obj_t *obj, kobj_set_t *handle)
{
    int fd = (int)(long)handle->docker;
    uint64_t val = 1;
    int ret;

    ret = write(fd, &val, sizeof val);
    (void)ret;
}

static int create_eventfd(csp_sem_t sem, kobj_set_t *handle)
{
    int fd = eventfd(0, 0);

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

#define us2tick(us) \
    ((us * YUNOS_CONFIG_TICKS_PER_SECOND + 999999) / 1000000)

static void _timer_cb(void *timer, void *arg)
{
    hal_timer_t *tmr = arg;
    tmr->cb(tmr->arg);
}

void hal_timer_init(hal_timer_t *tmr, unsigned first_us, unsigned period_us, hal_timer_cb_t cb, void *arg)
{
    bzero(tmr, sizeof(*tmr));
    tmr->cb = cb;
    tmr->arg = arg;
    yunos_timer_dyn_create((ktimer_t **)&tmr->priv, "hwtmr", _timer_cb,
                           us2tick(first_us), us2tick(period_us), tmr, 0);
}

int hal_timer_start(hal_timer_t *tmr)
{
    return yunos_timer_start(tmr->priv);
}

void hal_timer_stop(hal_timer_t *tmr)
{
    yunos_timer_stop(tmr->priv);
    yunos_timer_dyn_del(tmr->priv);
    tmr->priv = NULL;
}
