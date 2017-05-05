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

#include <k_api.h>
#include <assert.h>
#include <time.h>

#if (YUNOS_CONFIG_HW_COUNT > 0)
hr_timer_t soc_hr_hw_cnt_get(void)
{
    return 0;
}
#endif

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
#define SYS_DYN_POOL_SIZE (100 *1024)
size_t  sys_pool_start[SYS_DYN_POOL_SIZE / sizeof(size_t)];
#ifdef CONFIG_YOC_RHINO_MMREGION
static k_mm_region_head_t my_mm_region_list_head;
static k_mm_region_t      g_kregions[] = {{(uint8_t*)&sys_pool_start,SYS_DYN_POOL_SIZE}};
#endif

void soc_sys_mem_init(void)
{

#ifdef CONFIG_YOC_RHINO_MMREGION
    yunos_mm_region_init(&my_mm_region_list_head,g_kregions,sizeof(g_kregions)/sizeof(k_mm_region_t));
#endif
}

void *soc_mm_alloc(size_t size)
{
    kstat_t ret;
    void   *mem;
#ifdef CONFIG_YOC_RHINO_MMREGION
        size_t  alloctor = (size_t)__builtin_return_address(1);
        ret = yunos_mm_bf_alloc(&my_mm_region_list_head, (void **)&mem, size, alloctor);
#endif
    if (ret != YUNOS_SUCCESS) {
       // assert(0);
        return NULL;
    }

    return mem;
}

void soc_mm_free(void *mem)
{
    kstat_t ret;
#ifdef CONFIG_YOC_RHINO_MMREGION
    ret = yunos_mm_bf_free(&my_mm_region_list_head,mem);
#endif
    assert(ret == YUNOS_SUCCESS);
}
#endif

#ifdef CONFIG_YOC_LPM
uint32_t        lpm_sleep_time;
static uint64_t lpm_time_before_sleep;

int yoc_lpm_wakeup_source_get(void *wakeup_src)
{
    return 0;
}

int yoc_lpm_wakeup_source_set(uint32_t wakeup_src)
{
    return 0;
}

int yoc_lpm_rtc_time_get(void *time)
{
    lpm_time_before_sleep = hal_time_get() * 1000; /* ms */
    return 0;
}

int yoc_lpm_rtc_time_set(uint32_t time)
{
    lpm_sleep_time = time;
    return 0;
}

uint32_t yoc_lpm_rtc_elapse_get(void *tm_start)
{
    return (hal_time_get() * 1000 - lpm_time_before_sleep);
}

void yoc_lpm_systick_update(uint32_t ticks)
{
    CPSR_ALLOC();

    YUNOS_CPU_INTRPT_DISABLE();

    g_sys_time_tick  += ticks;

    YUNOS_CPU_INTRPT_ENABLE();
}
#endif

void soc_apps_entry(void)
{
}

void soc_err_proc(kstat_t err)
{
    /* assert */
}
yunos_err_proc_t g_err_proc = soc_err_proc;

