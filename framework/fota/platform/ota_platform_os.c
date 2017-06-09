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

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <yos/kernel.h>
#include <yos/log.h>
#include <ota_platform_os.h>

#define DEFAULT_THREAD_PRI   10

void *ota_malloc(uint32_t size)
{
    return malloc(size);
}

void ota_free(void *ptr)
{
    free(ptr);
}

void *ota_mutex_init(void)
{
    yos_mutex_t *mutex = (yos_mutex_t *)malloc(sizeof(yos_mutex_t));

    if (NULL == mutex) {
        return NULL;
    }

    if (0 != yos_mutex_new(mutex)) {
        free(mutex);
        return NULL;
    }

    return mutex;
}

void ota_mutex_lock(void *mutex)
{
    yos_mutex_lock((yos_mutex_t *)mutex, YOS_WAIT_FOREVER);
}

void ota_mutex_unlock(void *mutex)
{
    yos_mutex_unlock((yos_mutex_t *)mutex);
}

void ota_mutex_destroy(void *mutex)
{
    yos_mutex_free((yos_mutex_t *)mutex);
    free(mutex);
}

void *ota_semaphore_init(void)
{
    yos_sem_t *sem = (yos_sem_t *)malloc(sizeof(yos_sem_t));

    if (NULL == sem) {
        return NULL;
    }

    if (0 != yos_sem_new(sem, 0)) {
        free(sem);
        return NULL;
    }

    return sem;
}

int8_t ota_semaphore_wait( void *sem, uint32_t timeout_ms)
{
    if (-1 == timeout_ms) {
        return yos_sem_wait((yos_sem_t *)sem, -1);
    } else {
        return yos_sem_wait((yos_sem_t *)sem, timeout_ms);
    }
}

void ota_semaphore_post(void *sem)
{
    yos_sem_signal((yos_sem_t *)sem);
}

void ota_semaphore_destroy(void *sem)
{
    yos_sem_free((yos_sem_t *)sem);
    free(sem);
}

uint32_t ota_get_time_ms(void)
{
    struct timeval tv = { 0 };
    uint32_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return time_ms;
}


void ota_thread_exit(void *thread)
{
    yos_task_exit(0);
}

