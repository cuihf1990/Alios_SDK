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

#include <yos/sysdep.h>
#include <yos/log.h>
#ifdef HAL_ARCH_USE_NATIVE_GETTIMEOFDAY
#include <sys/time.h>
#endif

#include <hal/hal.h>
#include <hal/platform.h>

#ifdef HAL_ARCH_USE_COUNTER

#define S2US (1024ULL * 1024ULL)
#define MAX_CUR_CNT 200000000 /*200s ;when cur > MAX_CUR_CNT,need refresh base_time */
#define TAG "HAL_TIME"

static unsigned long long base_time_t = 1470208006ULL * S2US;
static unsigned int       cur_cnt;

void hal_time_refresh_base(void)
{
    unsigned int cur = hal_arch_get_time_counter();

    base_time_t += (cur - cur_cnt) * S2US / 1000000ULL;

    cur_cnt = cur;
}

void hal_time_set_base(unsigned long long base)
{
    base_time_t = base * S2US;

    cur_cnt = hal_arch_get_time_counter();
}

unsigned long long hal_time_get(void)
{
    unsigned int       cur = hal_arch_get_time_counter() - cur_cnt;
    unsigned long long c = base_time_t + cur;

    c /= S2US;

    return c;
}

void hal_time_gettimeofday(struct timeval *tv, void *tz)
{
    unsigned int       cur = hal_arch_get_time_counter() - cur_cnt;
    unsigned long long c = base_time_t + cur;

    tv->tv_sec  = c / S2US;
    tv->tv_usec = (c % S2US);
    if(cur >= MAX_CUR_CNT)
    {
        LOGD(TAG,"cur %d cur_cnt %d\n",cur,cur_cnt);
        hal_time_refresh_base();
    }
}

void hal_time_msleep(int ms)
{
    hal_arch_time_msleep(ms);
}

#elif defined(HAL_ARCH_USE_NATIVE_GETTIMEOFDAY)
#include <unistd.h>
void hal_time_refresh_base(void)
{
}

void hal_time_set_base(unsigned long long base)
{
}

unsigned long long hal_time_get()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec;
}

void hal_time_gettimeofday(struct timeval *tv, void *tz)
{
    gettimeofday(tv, tz);
}

void hal_time_msleep(int ms)
{
    hal_arch_time_msleep(ms);
}

#else

#error "non counter not support, contact sihai.ysh@alibaba-inc.com"

#endif /* HAL_ARCH_USE_COUNTER */

