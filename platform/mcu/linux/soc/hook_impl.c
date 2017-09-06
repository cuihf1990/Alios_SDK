/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>

void soc_hw_timer_init()
{
}

#if (RHINO_CONFIG_USER_HOOK > 0)
void yunos_idle_hook(void)
{
    cpu_idle_hook();
}

void yunos_init_hook(void)
{
#if (RHINO_CONFIG_HW_COUNT > 0)
    soc_hw_timer_init();
#endif
    cpu_init_hook();
}
#endif

void yunos_start_hook(void)
{
#if (RHINO_CONFIG_TASK_SCHED_STATS > 0)
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

void yunos_mm_alloc_hook(void *mem, size_t size)
{
    (void)mem;
    (void)size;
}
