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

kstat_t      g_sys_stat;
uint8_t      g_idle_task_spawned;

runqueue_t   g_ready_queue;

/* schedule lock counter */
uint8_t      g_sched_lock;
uint8_t      g_intrpt_nested_level;

/* highest pri task in ready queue */
ktask_t     *g_preferred_ready_task;

/* current active task */
ktask_t     *g_active_task;

/* idle task attribute */
ktask_t      g_idle_task;
idle_count_t g_idle_count;
cpu_stack_t  g_idle_task_stack[YUNOS_CONFIG_IDLE_TASK_STACK_SIZE];

/* tick attribute */
tick_t       g_tick_count;
klist_t      g_tick_head[YUNOS_CONFIG_TICK_HEAD_ARRAY];
sys_time_t   g_sys_time_tick;

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
kobj_list_t  g_kobj_list;
#endif

#if (YUNOS_CONFIG_TIMER > 0)
klist_t      g_timer_head;
tick_t       g_timer_count;
uint32_t     g_timer_ctrl;
ktask_t      g_timer_task;
cpu_stack_t  g_timer_task_stack[YUNOS_CONFIG_TIMER_TASK_STACK_SIZE];
ksem_t       g_timer_sem;
kmutex_t      g_timer_mutex;
#endif

#if (YUNOS_CONFIG_DYNTICKLESS > 0)
tick_t       g_next_intrpt_ticks;
tick_t       g_pend_intrpt_ticks;
tick_t       g_elapsed_ticks;
#endif

#if (YUNOS_CONFIG_TICK_TASK > 0)
ktask_t      g_tick_task;
cpu_stack_t  g_tick_task_stack[YUNOS_CONFIG_TICK_TASK_STACK_SIZE];
ksem_t       g_tick_sem;
#endif

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
hr_timer_t   g_sched_disable_time_start;
hr_timer_t   g_sched_disable_max_time;
hr_timer_t   g_cur_sched_disable_max_time;
#endif

#if (YUNOS_CONFIG_DISABLE_INTRPT_STATS > 0)
uint16_t     g_intrpt_disable_times;
hr_timer_t   g_intrpt_disable_time_start;
hr_timer_t   g_intrpt_disable_max_time;
hr_timer_t   g_cur_intrpt_disable_max_time;
#endif

#if (YUNOS_CONFIG_HW_COUNT > 0)
hr_timer_t   g_sys_measure_waste;
#endif

#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
ktask_t      g_cpu_usage_task;
cpu_stack_t  g_cpu_task_stack[YUNOS_CONFIG_CPU_USAGE_TASK_STACK];
idle_count_t g_idle_count_max;
uint32_t     g_cpu_usage;
uint32_t     g_cpu_usage_max;
#endif

#if (YUNOS_CONFIG_TASK_SCHED_STATS > 0)
ctx_switch_t g_sys_ctx_switch_times;
#endif

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
kqueue_t     g_dyn_queue;
void        *g_dyn_queue_msg[YUNOS_CONFIG_K_DYN_QUEUE_MSG];
ktask_t      g_dyn_mem_proc_task;
cpu_stack_t  g_dyn_mem_proc_stack[YUNOS_CONFIG_K_DYN_TASK_STACK];
#endif

#if (YUNOS_CONFIG_WORKQUEUE > 0)
workqueue_t *g_workqueue_head;
kmutex_t     g_workqueue_mutex;
#endif

#if (YUNOS_CONFIG_MM_BESTFIT > 0 || YUNOS_CONFIG_MM_FIRST_FIT > 0)
klist_t  g_mm_region_list_head = { NULL, NULL };
#if (YUNOS_CONFIG_MM_REGION_MUTEX == 1)
kmutex_t             g_mm_region_mutex;
#endif
#endif

