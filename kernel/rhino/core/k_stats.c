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

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
void kobj_list_init(void)
{
    klist_init(&(g_kobj_list.task_head));
    klist_init(&(g_kobj_list.mutex_head));
    //klist_init(&(g_kobj_list.mmpool_head));
    klist_init(&(g_kobj_list.mblkpool_head));

#if (YUNOS_CONFIG_SEM > 0)
    klist_init(&(g_kobj_list.sem_head));
#endif

#if (YUNOS_CONFIG_QUEUE > 0)
    klist_init(&(g_kobj_list.queue_head));
#endif

#if (YUNOS_CONFIG_BUF_QUEUE > 0)
    klist_init(&(g_kobj_list.buf_queue_head));
#endif

#if (YUNOS_CONFIG_EVENT_FLAG > 0)
    klist_init(&(g_kobj_list.event_head));
#endif
}
#endif

#if (YUNOS_CONFIG_TASK_STACK_OVF_CHECK > 0)
#if (YUNOS_CONFIG_CPU_STACK_DOWN > 0)
void yunos_stack_ovf_check(void)
{
    cpu_stack_t *stack_start;

    stack_start = g_active_task->task_stack_base;

    if (*stack_start != YUNOS_TASK_STACK_OVF_MAGIC) {
        k_err_proc(YUNOS_TASK_STACK_OVF);
    }

    if ((cpu_stack_t *)(g_active_task->task_stack) < stack_start) {
        k_err_proc(YUNOS_TASK_STACK_OVF);
    }
}

#else

void yunos_stack_ovf_check(void)
{
    cpu_stack_t *stack_start;
    cpu_stack_t *stack_end;

    stack_start = g_active_task->task_stack_base;
    stack_end   = stack_start + g_active_task->stack_size;

    if (*(stack_end - 1) != YUNOS_TASK_STACK_OVF_MAGIC) {
        k_err_proc(YUNOS_TASK_STACK_OVF);
    }

    if ((cpu_stack_t *)(g_active_task->task_stack) > stack_end) {
        k_err_proc(YUNOS_TASK_STACK_OVF);
    }
}
#endif
#endif

#if (YUNOS_CONFIG_TASK_SCHED_STATS > 0)
void yunos_task_sched_stats_reset(void)
{
    lr_timer_t cur_time;

#if (YUNOS_CONFIG_DISABLE_INTRPT_STATS > 0)
    g_cur_intrpt_disable_max_time = 0;
#endif

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
    g_cur_sched_disable_max_time = 0;
#endif

    /* system first task starting time should be measured otherwise not correct */
    cur_time = (lr_timer_t)LR_COUNT_GET();
    g_preferred_ready_task->task_time_start = cur_time;
}

void yunos_task_sched_stats_get(void)
{
    lr_timer_t cur_time;
    lr_timer_t exec_time;

#if (YUNOS_CONFIG_DISABLE_INTRPT_STATS > 0)
    hr_timer_t intrpt_disable_time;

    if (g_cur_intrpt_disable_max_time > g_sys_measure_waste) {
        intrpt_disable_time = g_cur_intrpt_disable_max_time - g_sys_measure_waste;
    } else {
        intrpt_disable_time = 0;
    }

    if (g_active_task->task_intrpt_disable_time_max < intrpt_disable_time) {
        g_active_task->task_intrpt_disable_time_max = intrpt_disable_time;
    }

    g_cur_intrpt_disable_max_time = 0;
#endif

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)

    if (g_active_task->task_sched_disable_time_max < g_cur_sched_disable_max_time) {
        g_active_task->task_sched_disable_time_max = g_cur_sched_disable_max_time;
    }

    g_cur_sched_disable_max_time = 0;
#endif

    /* Keep track of new task and total system context switch times */
    g_preferred_ready_task->task_ctx_switch_times++;
    g_sys_ctx_switch_times++;

    cur_time   = (lr_timer_t)LR_COUNT_GET();
    exec_time  = cur_time - g_active_task->task_time_start;

    g_active_task->task_time_total_run += (sys_time_t)exec_time;

    g_preferred_ready_task->task_time_start = cur_time;
}
#endif /* YUNOS_CONFIG_TASK_SCHED_STATS */

#if (YUNOS_CONFIG_DISABLE_INTRPT_STATS > 0)
void intrpt_disable_measure_start(void)
{
    g_intrpt_disable_times++;

    /* start measure interrupt disable time */
    if (g_intrpt_disable_times == 1u) {
        g_intrpt_disable_time_start = HR_COUNT_GET();
    }
}

void intrpt_disable_measure_stop(void)
{
    hr_timer_t diff;

    g_intrpt_disable_times--;

    if (g_intrpt_disable_times == 0u) {
        diff = HR_COUNT_GET() - g_intrpt_disable_time_start;

        if (g_intrpt_disable_max_time < diff) {
            g_intrpt_disable_max_time = diff;
        }

        if (g_cur_intrpt_disable_max_time < diff) {
            g_cur_intrpt_disable_max_time = diff;
        }
    }
}
#endif

#if (YUNOS_CONFIG_HW_COUNT > 0)
void yunos_overhead_measure(void)
{
    hr_timer_t diff;
    hr_timer_t m1;
    hr_timer_t m2;

    m1 = HR_COUNT_GET();

    HR_COUNT_GET();
    HR_COUNT_GET();

    m2 = HR_COUNT_GET();

    diff = m2 - m1;

    /* measure time overhead */
    g_sys_measure_waste = diff;
}
#endif

/*it should be called in cpu_stats task*/
#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
void yunos_cpu_usage_stats_init(void)
{

    klist_t *taskhead = &g_kobj_list.task_head;
    klist_t *stats_item = NULL;
    ktask_t *task = NULL;

    yunos_sched_disable();

    for (stats_item = taskhead->next; stats_item != taskhead;
         stats_item = stats_item->next) {
        task = yunos_list_entry(stats_item, ktask_t, task_stats_item);

        if (
#if (YUNOS_CONFIG_TICK_TASK > 0)
            (task != (ktask_t *)&g_tick_task) &&
#endif
#if (YUNOS_CONFIG_TIMER > 0)
            (task != (ktask_t *)&g_idle_task) &&
#endif
            (task != (ktask_t *)&g_active_task)) {
            yunos_task_suspend(task);
        }
    }

    yunos_sched_enable();

    idle_count_set(0u);

    yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);

    g_idle_count_max = idle_count_get();

    yunos_sched_disable();

    for (stats_item = taskhead->next; stats_item != taskhead;
         stats_item = stats_item->next) {
        task = yunos_list_entry(stats_item, ktask_t, task_stats_item);

        if (
#if (YUNOS_CONFIG_TICK_TASK > 0)
            (task != (ktask_t *)&g_tick_task) &&
#endif
#if (YUNOS_CONFIG_TIMER > 0)
            (task != (ktask_t *)&g_idle_task) &&
#endif
            (task != (ktask_t *)&g_active_task)) {
            yunos_task_resume(task);
        }
    }

    yunos_sched_enable();

}

static void cpu_usage_task_entry(void *arg)
{
    idle_count_t idle_count;

    (void)arg;

    yunos_cpu_usage_stats_init();

    while (1) {
        idle_count_set(0u);

        yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);

        idle_count = idle_count_get();

        if (idle_count < g_idle_count_max) {
            /* use 64bit for cpu_task_idle_count  to avoid overflow quickly */
            g_cpu_usage = 10000 - (uint32_t)((idle_count * 10000) / g_idle_count_max);

        }

        if (g_cpu_usage > g_cpu_usage_max) {
            g_cpu_usage_max = g_cpu_usage;

        }
    }
}

void cpu_usage_stats_start(void)
{
    /* create a statistic task to calculate cpu usage */
    yunos_task_create(&g_cpu_usage_task, "cpu_stats", 0,
                      YUNOS_CONFIG_CPU_USAGE_TASK_PRI,
                      0, g_cpu_task_stack, YUNOS_CONFIG_CPU_USAGE_TASK_STACK, cpu_usage_task_entry,
                      1);
}
#endif /* YUNOS_CONFIG_CPU_USAGE_STATS */

