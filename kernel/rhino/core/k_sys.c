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

extern k_mm_region_t g_mm_region;

YUNOS_INLINE void rhino_stack_check_init(void)
{
#if (YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK > 0)
#if (YUNOS_CONFIG_CPU_STACK_DOWN > 0)
    *g_intrpt_stack_bottom = YUNOS_INTRPT_STACK_OVF_MAGIC;
#else
    *g_intrpt_stack_top = YUNOS_INTRPT_STACK_OVF_MAGIC;
#endif
#endif /* YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK */

#if (YUNOS_CONFIG_STACK_OVF_CHECK_HW != 0)
    cpu_intrpt_stack_protect();
#endif
}

void workqueue_init(void);
YUNOS_INLINE kstat_t rhino_init(void)
{
    g_sys_stat = YUNOS_STOPPED;

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_init_hook();
#endif

    TRACE_INIT();

    runqueue_init(&g_ready_queue);

    tick_list_init();

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    kobj_list_init();
#endif

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
    /* init memory region */
#if(YUNOS_CONFIG_MM_TLF > 0)
    yunos_init_mm_head(&g_kmm_head, g_mm_region.start, g_mm_region.len);
#elif (YUNOS_CONFIG_MM_BESTFIT > 0 || YUNOS_CONFIG_MM_FIRSTFIT > 0)
    yunos_mm_region_init(&g_kmm_region_head, &g_mm_region,
                         sizeof(g_mm_region) / sizeof(k_mm_region_t));
#endif
#if (YUNOS_CONFIG_MM_LEAKCHECK > 0 )
    yos_mm_leak_region_init();
#endif
    yunos_queue_create(&g_dyn_queue, "Kobj_dyn_queue", (void **)&g_dyn_queue_msg,
                       YUNOS_CONFIG_K_DYN_QUEUE_MSG);
    dyn_mem_proc_task_start();
#endif

    yunos_task_create(&g_idle_task, "idle_task", NULL, YUNOS_IDLE_PRI, 0,
                      g_idle_task_stack, YUNOS_CONFIG_IDLE_TASK_STACK_SIZE,
                      idle_task, 1u);

#if (YUNOS_CONFIG_TIMER > 0)
    timer_init();
#endif

#if (YUNOS_CONFIG_TICK_TASK > 0)
    tick_task_start();
#endif

#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
    cpu_usage_stats_start();
#endif

    rhino_stack_check_init();

    return YUNOS_SUCCESS;
}

YUNOS_INLINE kstat_t rhino_start(void)
{
    if (g_sys_stat == YUNOS_STOPPED) {
        preferred_ready_task_get(&g_ready_queue);
        g_active_task = g_preferred_ready_task;

        workqueue_init();

#if (YUNOS_CONFIG_USER_HOOK > 0)
        yunos_start_hook();
#endif

        g_sys_stat = YUNOS_RUNNING;
        cpu_first_task_start();

        /* should not be here */
        return YUNOS_SYS_FATAL_ERR;
    }

    return YUNOS_RUNNING;
}

kstat_t yunos_init(void)
{
    return rhino_init();
}

kstat_t yunos_start(void)
{
    return rhino_start();
}

#if (YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK > 0)
#if (YUNOS_CONFIG_CPU_STACK_DOWN > 0)
void yunos_intrpt_stack_ovf_check(void)
{
    if (*g_intrpt_stack_bottom != YUNOS_INTRPT_STACK_OVF_MAGIC) {
        k_err_proc(YUNOS_INTRPT_STACK_OVF);
    }
}
#else
void yunos_intrpt_stack_ovf_check(void)
{
    if (*g_intrpt_stack_top != YUNOS_INTRPT_STACK_OVF_MAGIC) {
        k_err_proc(YUNOS_INTRPT_STACK_OVF);
    }
}
#endif
#endif /* YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK */

kstat_t yunos_intrpt_enter(void)
{
    CPSR_ALLOC();

#if (YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK > 0)
    yunos_intrpt_stack_ovf_check();
#endif

    YUNOS_CPU_INTRPT_DISABLE();

    if (g_intrpt_nested_level >= YUNOS_CONFIG_INTRPT_MAX_NESTED_LEVEL) {
        k_err_proc(YUNOS_INTRPT_NESTED_LEVEL_OVERFLOW);
        YUNOS_CPU_INTRPT_ENABLE();

        return YUNOS_INTRPT_NESTED_LEVEL_OVERFLOW;
    }

    g_intrpt_nested_level++;

    YUNOS_CPU_INTRPT_ENABLE();

    return YUNOS_SUCCESS;
}

void yunos_intrpt_exit(void)
{
    CPSR_ALLOC();

#if (YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK > 0)
    yunos_intrpt_stack_ovf_check();
#endif

    if (g_intrpt_nested_level == 0u) {
        k_err_proc(YUNOS_INV_INTRPT_NESTED_LEVEL);
    }

    YUNOS_CPU_INTRPT_DISABLE();

    g_intrpt_nested_level--;

    if (g_intrpt_nested_level > 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    if (g_sched_lock > 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    preferred_ready_task_get(&g_ready_queue);
    if (g_preferred_ready_task == g_active_task) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    TRACE_INTRPT_TASK_SWITCH(g_active_task, g_preferred_ready_task);

    cpu_intrpt_switch();

#if (YUNOS_CONFIG_STACK_OVF_CHECK_HW != 0)
    cpu_task_stack_protect(g_preferred_ready_task->task_stack_base,
                           g_preferred_ready_task->stack_size);
#endif

    YUNOS_CPU_INTRPT_ENABLE();
}

size_t yunos_global_space_get(void)
{
    size_t mem;

    mem = sizeof(g_sys_stat) + sizeof(g_idle_task_spawned) + sizeof(g_ready_queue)
          + sizeof(g_sched_lock) + sizeof(g_intrpt_nested_level) + sizeof(
              g_preferred_ready_task)
          + sizeof(g_active_task) + sizeof(g_idle_task) + sizeof(g_idle_task_stack)
          + sizeof(g_tick_head) + sizeof(g_idle_count) + sizeof(g_sys_time_tick);


#if (YUNOS_CONFIG_TIMER > 0)
    mem += sizeof(g_timer_head) + sizeof(g_timer_count) + sizeof(g_timer_ctrl)
           + sizeof(g_timer_task) + sizeof(g_timer_task_stack) + sizeof(g_timer_sem)
           + sizeof(g_timer_mutex);
#endif

#if (YUNOS_CONFIG_TICK_TASK > 0)
    mem += sizeof(g_tick_task) + sizeof(g_tick_task_stack) + sizeof(g_tick_sem);
#endif

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    mem += sizeof(g_kobj_list);
#endif

    return mem;
}

const name_t *yunos_version_get(void)
{
    return YUNOS_VERSION;
}

