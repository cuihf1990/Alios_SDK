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

extern void hw_start_hal(void);

int csp_mutex_new(csp_mutex_t *mutex)
{
    int ret;

    pthread_mutex_t*s = malloc(sizeof(pthread_mutex_t));
    ret = pthread_mutex_init(s, NULL);
    mutex->hdl = s;

    return ret;
}

uint32_t csp_mutex_lock(csp_mutex_t mutex)
{
    return pthread_mutex_lock(mutex.hdl);
}

uint32_t csp_mutex_unlock(csp_mutex_t mutex)
{
    return pthread_mutex_unlock(mutex.hdl);
}

void csp_mutex_free(csp_mutex_t *mutex)
{
    pthread_mutex_t *s = mutex->hdl;
    pthread_mutex_destroy(s);
    mutex->hdl = NULL;
    free(s);
}

int csp_sem_new(csp_sem_t *sem, int32_t count)
{
    sem_t *s = malloc(sizeof(*s));
    sem_init(s, 0, count);
    sem->hdl = s;
    return 0;
}

uint32_t csp_sem_wait(csp_sem_t sem, uint32_t timeout)
{
    int ret;
    struct timespec ts;

    if (!timeout || timeout == -1U) {
        return sem_wait(sem.hdl);
    }

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout / 1000;
    timeout %= 1000;
    ts.tv_nsec += timeout * 1000000;
    ts.tv_sec += ts.tv_nsec / 1000000000;
    ts.tv_nsec %= 1000000000;

    while ((ret = sem_timedwait(sem.hdl, &ts)) != 0) {
        if (errno != EINTR)
            break;
    }

    return ret;
}

void csp_sem_signal(csp_sem_t sem)
{
    sem_post(sem.hdl);
}

void csp_sem_free(csp_sem_t *sem)
{
    sem_t *s = sem->hdl;
    sem_destroy(s);
    sem->hdl = NULL;
    free(s);
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

int csp_poll(struct pollfd *pollfds, int nfds, csp_sem_t sem, uint32_t timeout)
{
    /* a fixed timeout for workaround */
    return poll(pollfds, nfds, 50);
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

