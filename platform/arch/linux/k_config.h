/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef K_CONFIG_H
#define K_CONFIG_H

/* chip level conf */
#define RHINO_CONFIG_LITTLE_ENDIAN           1
#define RHINO_CONFIG_CPU_STACK_DOWN          1

/* kernel feature conf */
#define RHINO_CONFIG_SEM                     1
#define RHINO_CONFIG_QUEUE                   1
#define RHINO_CONFIG_TASK_SEM                1
#define RHINO_CONFIG_EVENT_FLAG              1
#define RHINO_CONFIG_TIMER                   1
#define RHINO_CONFIG_BUF_QUEUE               1
#define RHINO_CONFIG_MM_BLK                  1
#define RHINO_CONFIG_MM_DEBUG                1
#define RHINO_CONFIG_MM_TLF                  1
#define RHINO_CONFIG_MM_MAXMSIZEBIT          24
#define RHINO_CONFIG_GCC_RETADDR             1
#define RHINO_CONFIG_MM_LEAKCHECK            0
#define K_MM_STATISTIC                       1
#define RHINO_CONFIG_KOBJ_SET                1
#define RHINO_CONFIG_RINGBUF_VENDOR          1

/* kernel task conf */
#define RHINO_CONFIG_TASK_SUSPEND            1
#define RHINO_CONFIG_TASK_INFO               10
#define RHINO_CONFIG_TASK_DEL                1
#define RHINO_CONFIG_TASK_WAIT_ABORT         1
#define RHINO_CONFIG_TASK_STACK_OVF_CHECK    1
#define RHINO_CONFIG_SCHED_RR                1
#define RHINO_CONFIG_TIME_SLICE_DEFAULT      10
#define RHINO_CONFIG_PRI_MAX                 62
#define RHINO_CONFIG_USER_PRI_MAX            (RHINO_CONFIG_PRI_MAX - 2)

/* kernel workqueue conf */
#define RHINO_CONFIG_WORKQUEUE               1

/* kernel mm_region conf */
#define RHINO_CONFIG_MM_REGION_MUTEX         0

/* kernel timer&tick conf */
#define RHINO_CONFIG_HW_COUNT                1
#define RHINO_CONFIG_TICK_TASK               1
#if (RHINO_CONFIG_TICK_TASK > 0)
#define RHINO_CONFIG_TICK_TASK_STACK_SIZE    256
#define RHINO_CONFIG_TICK_TASK_PRI           1
#endif
#define RHINO_CONFIG_TICKLESS                0
#define RHINO_CONFIG_TICKS_PER_SECOND        100
/* must be 2^n size!, such as 1, 2, 4, 8, 16,32, etc....... */
#define RHINO_CONFIG_TICK_HEAD_ARRAY         8
#define RHINO_CONFIG_TIMER_TASK_STACK_SIZE   200
#define RHINO_CONFIG_TIMER_RATE              1
#define RHINO_CONFIG_TIMER_TASK_PRI          5

/* kernel intrpt conf */
#define RHINO_CONFIG_INTRPT_STACK_REMAIN_GET 1
#define RHINO_CONFIG_INTRPT_STACK_OVF_CHECK  0
#define RHINO_CONFIG_INTRPT_MAX_NESTED_LEVEL 188u
#define RHINO_CONFIG_INTRPT_GUARD            0

/* kernel dyn alloc conf */
#define RHINO_CONFIG_KOBJ_DYN_ALLOC          1
#if (RHINO_CONFIG_KOBJ_DYN_ALLOC > 0)
#define RHINO_CONFIG_K_DYN_QUEUE_MSG         30
#define RHINO_CONFIG_K_DYN_TASK_STACK        256
#define RHINO_CONFIG_K_DYN_MEM_TASK_PRI      5
#endif

/* kernel idle conf */
#define RHINO_CONFIG_IDLE_TASK_STACK_SIZE    100

/* kernel hook conf */
#define RHINO_CONFIG_USER_HOOK               1

/* kernel stats conf */
#define RHINO_CONFIG_SYSTEM_STATS            1
#define RHINO_CONFIG_DISABLE_SCHED_STATS     1
#define RHINO_CONFIG_DISABLE_INTRPT_STATS    1
#define RHINO_CONFIG_CPU_USAGE_STATS         0
#define RHINO_CONFIG_CPU_USAGE_TASK_PRI      (RHINO_CONFIG_PRI_MAX - 2)
#define RHINO_CONFIG_TASK_SCHED_STATS        0
#define RHINO_CONFIG_CPU_USAGE_TASK_STACK    256

/* kernel trace conf */
#define RHINO_CONFIG_TRACE                   1

#ifndef RHINO_CONFIG_CPU_NUM
#define RHINO_CONFIG_CPU_NUM                 1
#endif

#endif /* K_CONFIG_H */

