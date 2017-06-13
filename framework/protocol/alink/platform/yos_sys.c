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

#include "yos_sys.h"

uint32_t yos_get_time_ms(void)
{
    struct timeval tv = { 0 };
    uint32_t time_ms;

    gettimeofday(&tv, NULL);

    time_ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return time_ms;
}

pthread_mutex_t *yos_mutex_init()
{
    pthread_mutex_t *ret = NULL;
    ret = (pthread_mutex_t *)os_malloc(sizeof(pthread_mutex_t));
    if (!ret) {
        return NULL;
    }
    if (0 != pthread_mutex_init(ret, NULL)) {
        os_free(ret);
        return NULL;
    }
    return ret;
}

void yos_mutex_destroy(pthread_mutex_t *mutex)
{
    if (mutex) {
        pthread_mutex_destroy(mutex);
        os_free(mutex);
    }
}

uint32_t yos_get_unaligned_be32(uint8_t *ptr)
{
    uint32_t res;

    memcpy(&res, ptr, sizeof(uint32_t));

    return yos_be32toh(res);
}

