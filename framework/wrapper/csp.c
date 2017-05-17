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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <csp.h>
#include <yos/kernel.h>
#include <hal/soc/hal.h>
#include <poll.h>
#include <k_api.h>

int yos_mutex_new(yos_mutex_t *mutex) {
    return csp_mutex_new(mutex);
}

uint32_t yos_mutex_lock(yos_mutex_t mutex) {
    return csp_mutex_lock(mutex);
}

uint32_t yos_mutex_unlock(yos_mutex_t mutex)
{
    return csp_mutex_unlock(mutex);
}

void yos_mutex_free(yos_mutex_t *mutex)
{
    csp_mutex_free(mutex);
}

int yos_sem_new(yos_sem_t *sem, int32_t count)
{
    return csp_sem_new(sem, count);
}

uint32_t yos_sem_wait(yos_sem_t sem, uint32_t timeout)
{
    return csp_sem_wait(sem, timeout);
}

void yos_sem_signal(yos_sem_t sem)
{
    csp_sem_signal(sem);
}

void yos_sem_free(yos_sem_t *sem)
{
    csp_sem_free(sem);
}

uint64_t yos_now(void)
{
    return csp_now();
}

#define ms2tick(ms) \
    ((ms * YUNOS_CONFIG_TICKS_PER_SECOND + 999) / 1000)

void yos_msleep(int ms)
{
    yunos_task_sleep(ms2tick(ms));
}

int yos_task_new(const char *name, void (*fn)(void *), void *arg, int stacksize)
{
    return csp_task_new(name, fn, arg, stacksize);
}

int yos_task_new_ext(const char *name, void (*fn)(void *),
                     void *arg, int stacksize, int prio)
{
    return csp_task_new_ext(name, fn, arg, stacksize, prio);
}

void yos_task_exit(int code)
{
    csp_task_exit(code);
}

#ifdef HAVE_RHINO_KERNEL
#include <k_api.h>

int csp_mutex_new(csp_mutex_t *mutex)
{
    kmutex_t *s = malloc(sizeof(kmutex_t));
    yunos_mutex_create(s, "csp");
    mutex->hdl = s;
    return 0;
}

uint32_t csp_mutex_lock(csp_mutex_t mutex)
{
    kstat_t ret = yunos_mutex_lock(mutex.hdl, YUNOS_WAIT_FOREVER);

    /*rhino allow nested*/
    if (ret == YUNOS_MUTEX_OWNER_NESTED)
    {ret = YUNOS_SUCCESS;}

    return ret;
}

uint32_t csp_mutex_unlock(csp_mutex_t mutex)
{
    kstat_t ret = yunos_mutex_unlock(mutex.hdl);

    /*rhino allow nested*/
    if (ret == YUNOS_MUTEX_OWNER_NESTED)
    {ret = YUNOS_SUCCESS;}

    return ret;
}

void csp_mutex_free(csp_mutex_t *mutex)
{
    kmutex_t *s = mutex->hdl;
    yunos_mutex_del(s);
    mutex->hdl = NULL;
    free(s);
}

int csp_sem_new(csp_sem_t *sem, int32_t count)
{
    ksem_t *s = malloc(sizeof(ksem_t));
    yunos_sem_create(s, "csp", count);
    sem->hdl = s;
    return 0;
}

uint32_t csp_sem_wait(csp_sem_t sem, uint32_t timeout)
{
    if (timeout == YUNOS_WAIT_FOREVER) {
        return yunos_sem_take(sem.hdl, YUNOS_WAIT_FOREVER);
    } else {
        return yunos_sem_take(sem.hdl, ms2tick(timeout));
    }

    return 0;
}

void csp_sem_signal(csp_sem_t sem)
{
    yunos_sem_give(sem.hdl);
}

void csp_sem_free(csp_sem_t *sem)
{
    ksem_t *s = sem->hdl;
    yunos_sem_del(s);
    sem->hdl = NULL;
    free(s);
}

int csp_task_new_ext(const char *name, void (*fn)(void *), void *arg,
                     int stacksize, int prio)
{
    int ret = 0;
#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
    ktask_t *task_handle = NULL;
    ret = yunos_task_dyn_create(&task_handle, name, arg,
                              prio, 0, stacksize / sizeof(cpu_stack_t), fn, 1u);
#endif

    return ret;
}
#endif /* HAVE_RHINO_KERNEL */

int csp_task_new(const char *name, void (*fn)(void *), void *arg,
                 int stacksize)
{
    return csp_task_new_ext(name, fn, arg, stacksize, 9);
}

#ifdef HAVE_RHINO_KERNEL
void csp_task_exit(int code)
{
    /* delete my self*/
#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
    yunos_task_dyn_del(NULL);
#endif
}

uint64_t csp_now(void)
{
    return yunos_sys_time_get() * 1000 * 1000;
}

int csp_sys_reset()
{
    if (hal_arch_reboot) {
        hal_arch_reboot();
    }

    return 0xff;
}

int csp_sys_free(uint32_t *f)
{
    return 0;
}

void *yos_malloc(size_t sz)
{
    return yunos_mm_alloc(sz);
}

void yos_free(void *p)
{
    yunos_mm_free(p);
}

#if defined(WITH_LWIP)
#include "lwip/sys.h"

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
    lwip_close(fd);
    ((blk_obj_t *)sem.hdl)->handle = NULL;
}

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
#endif
#endif /* HAVE_RHINO_KERNEL */

int csp_net_errno(int fd)
{
    return errno;
}

static char **strsplit(char *src, int max_fields)
{
    char *  token;
    char ** argv = malloc((max_fields + 1) * sizeof(char *));
    int     args = 0;

    argv[args++] = "yos";
    for (token = strsep(&src, ","); token != NULL; token = strsep(&src, ",")) {
        if (strlen(token) == 0)
            continue;
        if (args >= max_fields) {
            break;
        }
        argv[args ++] = token;
    }
    argv[args] = NULL;
    return argv;
}
