/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>

kstat_t      g_sys_stat;
uint8_t      g_idle_task_spawned[YUNOS_CONFIG_CPU_NUM];

runqueue_t   g_ready_queue;

/* schedule lock counter */
uint8_t      g_sched_lock[YUNOS_CONFIG_CPU_NUM];
uint8_t      g_intrpt_nested_level[YUNOS_CONFIG_CPU_NUM];

/* highest pri task in ready queue */
ktask_t     *g_preferred_ready_task[YUNOS_CONFIG_CPU_NUM];

/* current active task */
ktask_t     *g_active_task[YUNOS_CONFIG_CPU_NUM];

/* idle task attribute */
ktask_t      g_idle_task[YUNOS_CONFIG_CPU_NUM];
idle_count_t g_idle_count[YUNOS_CONFIG_CPU_NUM];
cpu_stack_t  g_idle_task_stack[YUNOS_CONFIG_CPU_NUM][YUNOS_CONFIG_IDLE_TASK_STACK_SIZE];

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
kmutex_t     g_timer_mutex;
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
kqueue_t    g_dyn_queue;
void       *g_dyn_queue_msg[YUNOS_CONFIG_K_DYN_QUEUE_MSG];
ktask_t     g_dyn_mem_proc_task;
cpu_stack_t g_dyn_mem_proc_stack[YUNOS_CONFIG_K_DYN_TASK_STACK];
#endif

#if (YUNOS_CONFIG_WORKQUEUE > 0)
klist_t       g_workqueue_list_head;
kmutex_t      g_workqueue_mutex;
kworkqueue_t  g_workqueue_default;
cpu_stack_t   g_workqueue_stack[YUNOS_CONFIG_WORKQUEUE_STACK_SIZE];
#endif

#if (YUNOS_CONFIG_MM_TLF > 0)

k_mm_head       *g_kmm_head;

#endif

