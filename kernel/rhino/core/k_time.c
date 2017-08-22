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

#if (YUNOS_CONFIG_DYNTICKLESS > 0)
void yunos_tickless_proc(tick_t ticks)
{
    CPSR_ALLOC();

#if (YUNOS_CONFIG_INTRPT_GUARD > 0)
    soc_intrpt_guard();
#endif

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_tick_hook();
#endif

    YUNOS_CPU_INTRPT_DISABLE();

    g_pend_intrpt_ticks += ticks;

    YUNOS_CPU_INTRPT_ENABLE();

#if (YUNOS_CONFIG_TICK_TASK > 0)
    yunos_task_sem_give(&g_tick_task);
#else
    tick_list_update();
#endif
}
#else
void yunos_tick_proc(void)
{
#if (YUNOS_CONFIG_INTRPT_GUARD > 0)
    soc_intrpt_guard();
#endif

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_tick_hook();
#endif

#if (YUNOS_CONFIG_TICK_TASK > 0)
    yunos_task_sem_give(&g_tick_task);
#else
    tick_list_update();
#endif

#if (YUNOS_CONFIG_SCHED_RR > 0)
    time_slice_update(g_active_task[cpu_cur_get()]->prio);
#endif

#if (YUNOS_CONFIG_TIMER > 0)
    timer_task_sched();
#endif
}
#endif

sys_time_t yunos_sys_tick_get(void)
{
    sys_time_t tick_tmp;

#if (YUNOS_CONFIG_DYNTICKLESS > 0)
    tick_t elapsed_ticks;

    CPSR_ALLOC();

    YUNOS_CPU_INTRPT_DISABLE();

    elapsed_ticks = soc_elapsed_ticks_get();
    tick_tmp = g_sys_time_tick + (sys_time_t)(elapsed_ticks);

    YUNOS_CPU_INTRPT_ENABLE();
#else
    tick_tmp = g_sys_time_tick;
#endif

    return tick_tmp;
}

sys_time_t yunos_sys_time_get(void)
{
    return (sys_time_t)(yunos_sys_tick_get() * 1000 /
                        YUNOS_CONFIG_TICKS_PER_SECOND);
}

tick_t     yunos_ms_to_ticks(sys_time_t ms)
{
    uint16_t  padding;
    uint16_t  surplus;
    tick_t    ticks;

    surplus = ms % 1000;
    ticks = (ms / 1000) * YUNOS_CONFIG_TICKS_PER_SECOND;
    padding = 1000 / YUNOS_CONFIG_TICKS_PER_SECOND;
    padding = (padding > 0) ? (padding - 1) : 0;

    ticks += ((surplus + padding) * YUNOS_CONFIG_TICKS_PER_SECOND) / 1000;

    return ticks;
}

sys_time_t yunos_ticks_to_ms(tick_t ticks)
{
    uint32_t   padding;
    uint32_t   surplus;
    sys_time_t time;

    surplus = ticks % YUNOS_CONFIG_TICKS_PER_SECOND;
    time = (ticks / YUNOS_CONFIG_TICKS_PER_SECOND) * 1000;
    padding = YUNOS_CONFIG_TICKS_PER_SECOND / 1000;
    padding = (padding > 0) ? (padding - 1) : 0;

    time += ((surplus + padding) * 1000) / YUNOS_CONFIG_TICKS_PER_SECOND;

    return time;
}

