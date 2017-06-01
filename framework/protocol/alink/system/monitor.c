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

#include "work_queue.h"

extern int os_malloc_get_peak_usage(void);
extern int os_malloc_get_current_usage(void);
extern void os_malloc_dump_memory_usage(void);

void system_monitor(void);

#define MONITOR_WORK_CYCLE      (60 * 1000)
static struct work_struct monitor_work = {
    .func = (work_func_t)&system_monitor,
    .prio = DEFAULT_WORK_PRIO,
    .name = "system monitor",
};

void system_monitor_init(void)
{
    queue_work(&monitor_work);
}

void system_monitor_exit(void)
{
    cancel_work(&monitor_work);
}

void system_monitor(void)
{
    int memory_peak_usage = os_malloc_get_peak_usage();
    int memory_cur_usage = os_malloc_get_current_usage();

    log_info("sdk memory usage: peak %d, current %d",
            memory_peak_usage, memory_cur_usage);

    os_malloc_dump_memory_usage();

    queue_delayed_work(&monitor_work, MONITOR_WORK_CYCLE);
}
