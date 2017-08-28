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

    runqueue_init(&g_ready_queue);

    tick_list_init();

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    kobj_list_init();
#endif

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
    /* init memory region */
#if(YUNOS_CONFIG_MM_TLF > 0)
    yunos_init_mm_head(&g_kmm_head, g_mm_region.start, g_mm_region.len);
#endif
#if (YUNOS_CONFIG_MM_LEAKCHECK > 0 )
    yos_mm_leak_region_init();
#endif
    yunos_queue_create(&g_dyn_queue, "Kobj_dyn_queue", (void **)&g_dyn_queue_msg,
                       YUNOS_CONFIG_K_DYN_QUEUE_MSG);
    dyn_mem_proc_task_start();
#endif

#if (YUNOS_CONFIG_CPU_NUM > 1)
    for (uint8_t i = 0; i < YUNOS_CONFIG_CPU_NUM; i++) {
        yunos_task_cpu_create(&g_idle_task[i], "idle_task", NULL, YUNOS_IDLE_PRI, 0,
                              &g_idle_task_stack[i][0], YUNOS_CONFIG_IDLE_TASK_STACK_SIZE,
                              idle_task, i, 1u);
    }
#else
    yunos_task_create(&g_idle_task[0], "idle_task", NULL, YUNOS_IDLE_PRI, 0,
                      &g_idle_task_stack[0][0], YUNOS_CONFIG_IDLE_TASK_STACK_SIZE,
                      idle_task, 1u);
#endif

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
#if (YUNOS_CONFIG_CPU_NUM > 1)
        for (uint8_t i = 0; i < YUNOS_CONFIG_CPU_NUM; i++) {
            preferred_cpu_ready_task_get(&g_ready_queue, i);
            g_active_task[i] = g_preferred_ready_task[i];
            g_active_task[i]->cur_exc = 1;
        }
#else
        preferred_cpu_ready_task_get(&g_ready_queue, 0);
        g_active_task[0] = g_preferred_ready_task[0];
#endif
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

    if (g_intrpt_nested_level[cpu_cur_get()] >= YUNOS_CONFIG_INTRPT_MAX_NESTED_LEVEL) {
        k_err_proc(YUNOS_INTRPT_NESTED_LEVEL_OVERFLOW);
        YUNOS_CPU_INTRPT_ENABLE();

        return YUNOS_INTRPT_NESTED_LEVEL_OVERFLOW;
    }

    g_intrpt_nested_level[cpu_cur_get()]++;

    YUNOS_CPU_INTRPT_ENABLE();

    return YUNOS_SUCCESS;
}

void yunos_intrpt_exit(void)
{
    CPSR_ALLOC();
    uint8_t cur_cpu_num;

#if (YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK > 0)
    yunos_intrpt_stack_ovf_check();
#endif

    YUNOS_CPU_INTRPT_DISABLE();

    cur_cpu_num = cpu_cur_get();

    if (g_intrpt_nested_level[cur_cpu_num] == 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        k_err_proc(YUNOS_INV_INTRPT_NESTED_LEVEL);
    }

    g_intrpt_nested_level[cur_cpu_num]--;

    if (g_intrpt_nested_level[cur_cpu_num] > 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    if (g_sched_lock[cur_cpu_num] > 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    preferred_cpu_ready_task_get(&g_ready_queue, cur_cpu_num);

    if (g_preferred_ready_task[cur_cpu_num] == g_active_task[cur_cpu_num]) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    TRACE_INTRPT_TASK_SWITCH(g_active_task[cur_cpu_num], g_preferred_ready_task[cur_cpu_num]);

#if (YUNOS_CONFIG_CPU_NUM > 1)
    g_active_task[cur_cpu_num]->cur_exc = 0;
#endif

    cpu_intrpt_switch();

#if (YUNOS_CONFIG_STACK_OVF_CHECK_HW != 0)
    cpu_task_stack_protect(g_preferred_ready_task[cur_cpu_num]->task_stack_base,
                           g_preferred_ready_task[cur_cpu_num]->stack_size);
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

