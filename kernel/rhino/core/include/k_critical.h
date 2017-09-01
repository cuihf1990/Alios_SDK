/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef K_CRITICAL_H
#define K_CRITICAL_H

#if (YUNOS_CONFIG_DISABLE_INTRPT_STATS > 0)
#define YUNOS_CRITICAL_ENTER()          \
    do {                                \
        YUNOS_CPU_INTRPT_DISABLE();     \
        intrpt_disable_measure_start(); \
    } while (0)

#define YUNOS_CRITICAL_EXIT()           \
    do {                                \
        intrpt_disable_measure_stop();  \
        YUNOS_CPU_INTRPT_ENABLE();      \
    } while (0)

#if (YUNOS_CONFIG_CPU_NUM > 1)
#define YUNOS_CRITICAL_EXIT_SCHED()         \
        do {                                \
            intrpt_disable_measure_stop();  \
            core_sched();                   \
            YUNOS_CPU_INTRPT_ENABLE();      \
        } while (0)
#else
#define YUNOS_CRITICAL_EXIT_SCHED()         \
        do {                                \
            intrpt_disable_measure_stop();  \
            YUNOS_CPU_INTRPT_ENABLE();      \
            core_sched();                   \
        } while (0)
#endif

#else /* YUNOS_CONFIG_DISABLE_INTRPT_STATS */
#define YUNOS_CRITICAL_ENTER()          \
    do {                                \
        YUNOS_CPU_INTRPT_DISABLE();     \
    } while (0)

#define YUNOS_CRITICAL_EXIT()           \
    do {                                \
        YUNOS_CPU_INTRPT_ENABLE();      \
    } while (0)

#if (YUNOS_CONFIG_CPU_NUM > 1)
#define YUNOS_CRITICAL_EXIT_SCHED()     \
        do {                            \
            core_sched();               \
            YUNOS_CPU_INTRPT_ENABLE();  \
        } while (0)
#else
#define YUNOS_CRITICAL_EXIT_SCHED()     \
        do {                            \
            YUNOS_CPU_INTRPT_ENABLE();  \
            core_sched();               \
        } while (0)
#endif

#endif /* YUNOS_CONFIG_DISABLE_INTRPT_STATS */

#endif /* K_CRITICAL_H */

