/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

#include <sys/time.h>
#include <time.h>
#include <yos/kernel.h>
#include <yos/log.h>
#include "platform.h"
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#define TAG "alink_os"

#define DEFAULT_THREAD_PRI   10

typedef void (*thread_entry_t)(void *arg);

typedef void *(*start_routine_cb)(void *);

typedef struct {
    start_routine_cb start_routine;
    void            *arg;
} platform_thread_arg_t;

void platform_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}


void *platform_malloc(uint32_t size)
{
    return yos_malloc(size);
}

void platform_free(void *ptr)
{
    yos_free(ptr);
}

void *platform_mutex_init(void)
{
    yos_mutex_t mutex;

    if (0 != yos_mutex_new(&mutex)) {
        return NULL;
    }

    return mutex.hdl;
}

void platform_mutex_lock(void *mutex)
{
    yos_mutex_lock((yos_mutex_t *)&mutex, YOS_WAIT_FOREVER);
}

void platform_mutex_unlock(void *mutex)
{
    yos_mutex_unlock((yos_mutex_t *)&mutex);
}

void platform_mutex_destroy(void *mutex)
{
    yos_mutex_free((yos_mutex_t *)&mutex);
}

void *platform_semaphore_init(void)
{
    yos_sem_t sem;

    if (0 != yos_sem_new(&sem, 0)) {
        return NULL;
    }

    return sem.hdl;
}

int platform_semaphore_wait(_IN_ void *sem, _IN_ uint32_t timeout_ms)
{
    if (PLATFORM_WAIT_INFINITE == timeout_ms) {
        return yos_sem_wait((yos_sem_t *)&sem, YOS_WAIT_FOREVER);
    } else {
        return yos_sem_wait((yos_sem_t *)&sem, timeout_ms);
    }
}

void platform_semaphore_post(void *sem)
{
    yos_sem_signal((yos_sem_t *)&sem);
}

void platform_semaphore_destroy(void *sem)
{
    yos_sem_free((yos_sem_t *)&sem);
}

void platform_msleep(_IN_ uint32_t ms)
{
    yos_msleep(ms);
}

uint32_t platform_get_time_ms(void)
{
    return yos_now_ms();
}

uint64_t platform_get_utc_time(_INOUT_ uint64_t *p_utc)
{
    return (uint64_t)time((time_t *)p_utc);
}

os_time_struct *platform_local_time_r(const _IN_ uint64_t *p_utc,
                                      _OUT_ os_time_struct *p_result)
{
    return (os_time_struct *)localtime_r((time_t *)p_utc, (struct tm *)p_result);
}

int platform_thread_get_stack_size(_IN_ const char *thread_name)
{
    /* Note: not used by linux platform, rtos tune stack size here */
    if (0 == strcmp(thread_name, "wsf_receive_worker")) {
        LOGD(TAG, "get wsf receive worker");
        return 8192;
    } else if (0 == strcmp(thread_name, "wsf_send_worker")) {
        LOGD(TAG, "get wsf send worker\n");
        return 4096;
    } else if (0 == strcmp(thread_name, "wsf_callback_worker")) {
        LOGD(TAG, "get wsf callback worker\n");
        return 4096;
    } else if (!strcmp(thread_name, "yunio_alink_worker")) {
        /* Yunio only need small stack */
        return 4096;
    } else if (0 == strcmp(thread_name, "fota_thread")) {
        LOGD(TAG, "get fota thread\n");
        return 4096;
    } else if (0 == strcmp(thread_name, "cota_thread")) {
        LOGD(TAG, "get cota thread\n");
        return 4096;
    } else if (0 == strcmp(thread_name, "alcs_thread")) {
        LOGD(TAG, "get alcs thread\n");
        return 8192;
    } else if (0 == strcmp(thread_name, "work queue")) {
        LOGD(TAG, "get work queue thread\n");
        return 4096;
    } else if (0 == strcmp(thread_name, "ifttt_update_system_utc")) {
        LOGD(TAG, "get ifttt_update_system_utc thread\n");
        return 8192;
    } else if (0 == strcmp(thread_name, "asr_websocket_thread")) {
        LOGD(TAG, "get asr_websocket_thread thread\n");
        return 8192;
    }

    //assert(0);
}

int platform_thread_create(
    _OUT_ void **thread,
    _IN_ const char *name,
    _IN_ void *(*start_routine)(void *),
    _IN_ void *arg,
    _IN_ void *stack,
    _IN_ uint32_t stack_size,
    _OUT_ int *stack_used)
{
    int ret = -1;
    *stack_used = 0;
    (void)stack;
    yos_task_t task;

    ret = yos_task_new_ext(&task, name, (thread_entry_t)start_routine, arg, stack_size,
                           DEFAULT_THREAD_PRI);

    *thread = start_routine;

    return ret;
}

void platform_thread_exit(void *thread)
{
    yos_task_exit(0);
}

