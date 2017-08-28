#include <k_api.h>

#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
void idle_count_set(idle_count_t value)
{
    CPSR_ALLOC();

    YUNOS_CPU_INTRPT_DISABLE();

    g_idle_count[cpu_cur_get()] = value;

    YUNOS_CPU_INTRPT_ENABLE();
}

idle_count_t idle_count_get(void)
{
    CPSR_ALLOC();

    idle_count_t idle_count;

    YUNOS_CPU_INTRPT_DISABLE();

    idle_count = g_idle_count[cpu_cur_get()];

    YUNOS_CPU_INTRPT_ENABLE();

    return idle_count;
}
#endif

void idle_task(void *arg)
{
    CPSR_ALLOC();

    /* avoid warning */
    (void)arg;

    while (YUNOS_TRUE) {
        YUNOS_CPU_INTRPT_DISABLE();

        g_idle_count[cpu_cur_get()]++;

        YUNOS_CPU_INTRPT_ENABLE();

#if (YUNOS_CONFIG_USER_HOOK > 0)
        yunos_idle_hook();
#endif
    }
}

