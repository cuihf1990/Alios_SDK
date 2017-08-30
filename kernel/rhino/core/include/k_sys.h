/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef K_SYS_H
#define K_SYS_H

#define YUNOS_VERSION  "0.4.0"

#define YUNOS_IDLE_PRI (YUNOS_CONFIG_PRI_MAX - 1)
#define YUNOS_FALSE    0u
#define YUNOS_TRUE     1u

#if (YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK > 0)
#if (YUNOS_CONFIG_CPU_STACK_DOWN > 0)
extern cpu_stack_t *g_intrpt_stack_bottom;
#else
extern cpu_stack_t *g_intrpt_stack_top;
#endif
#endif /* YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK */

/**
 * This function will init yunos
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t       yunos_init(void);

/**
 * This function will start yunos
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t       yunos_start(void);

/**
 * This function will enter interrupt
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t       yunos_intrpt_enter(void);

/**
 * This function will exit interrupt
 */
void          yunos_intrpt_exit(void);

/**
 * This function will check intrpt-stack overflow
 */
void          yunos_intrpt_stack_ovf_check(void);

/**
 * This function will get the whole ram space used by kernel
 * @return  the whole ram space used by kernel
 */
size_t        yunos_global_space_get(void);

/**
 * This function will get kernel version
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
const name_t *yunos_version_get(void);

#endif /* K_SYS_H */

