/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef K_STATS_H
#define K_STATS_H

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
void kobj_list_init(void);
#endif

#if (YUNOS_CONFIG_TASK_STACK_OVF_CHECK > 0)
/**
 * This function will check task stack overflow
 */
void yunos_stack_ovf_check(void);
#endif

#if (YUNOS_CONFIG_TASK_SCHED_STATS > 0)
/**
 * This function will reset task schedule stats
 */
void yunos_task_sched_stats_reset(void);
/**
 * This function will get task statistic data
 */
void yunos_task_sched_stats_get(void);
#endif

#if (YUNOS_CONFIG_HW_COUNT > 0)
void yunos_overhead_measure(void);
#endif

#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
void yunos_cpu_usage_stats_init(void);
#endif

#endif /* K_STATS_H */

