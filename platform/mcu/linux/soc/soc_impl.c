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
#define       SYS_DYN_POOL_SIZE (288 *1024)
size_t        sys_pool_start[SYS_DYN_POOL_SIZE / sizeof(size_t)];
k_mm_region_t g_mm_region[] = {{(uint8_t*)&sys_pool_start,SYS_DYN_POOL_SIZE}};
#endif

void soc_err_proc(kstat_t err)
{
    /* assert */
}
yunos_err_proc_t g_err_proc = soc_err_proc;

