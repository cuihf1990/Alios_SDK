/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <unistd.h>
#include <sys/time.h>
#include <yos/kernel.h>

#include "iot_import.h"

void *HAL_MutexCreate(void)
{
    yos_mutex_t mutex;

    if (0 != yos_mutex_new(&mutex)) {
        return NULL;
    }

}

void HAL_MutexDestroy(_IN_ void *mutex)
{
    yos_mutex_free((yos_mutex_t *)&mutex);
}

void HAL_MutexLock(_IN_ void *mutex)
{
    yos_mutex_lock((yos_mutex_t *)&mutex, YOS_WAIT_FOREVER);
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
    yos_mutex_unlock((yos_mutex_t *)&mutex);
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return yos_malloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    return yos_free(ptr);
}

uint32_t HAL_UptimeMs(void)
{
    return yos_now_ms();
}

void HAL_SleepMs(_IN_ uint32_t ms)
{
    yos_msleep(ms);
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

char *HAL_GetPartnerID(char pid_str[])
{
    return NULL;
}
