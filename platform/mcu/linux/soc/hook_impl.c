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

void soc_hw_timer_init()
{
}

#if (YUNOS_CONFIG_USER_HOOK > 0)
void yunos_idle_hook(void)
{
    cpu_idle_hook();
}

void yunos_init_hook(void)
{
#if (YUNOS_CONFIG_HW_COUNT > 0)
    soc_hw_timer_init();
#endif
    cpu_init_hook();
}
#endif

void yunos_start_hook(void)
{
#if (YUNOS_CONFIG_TASK_SCHED_STATS > 0)
    yunos_task_sched_stats_reset();
#endif
    cpu_start_hook();
}

void yunos_task_create_hook(ktask_t *task)
{
    cpu_task_create_hook(task);
}

void yunos_task_del_hook(ktask_t *task)
{
    cpu_task_del_hook(task);
}

void yunos_task_switch_hook(ktask_t *orgin, ktask_t *dest)
{
    (void)orgin;
    (void)dest;
}

void yunos_tick_hook(void)
{
}

void yunos_task_abort_hook(ktask_t *task)
{
    (void)task;
}

yunos_mm_alloc_hook(void *mem, size_t size)
{
    (void)mem;
    (void)size;
}
