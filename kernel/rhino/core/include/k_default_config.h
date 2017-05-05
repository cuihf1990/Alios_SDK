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

#ifndef K_DEFAULT_CONFIG_H
#define K_DEFAULT_CONFIG_H

/* chip level conf */
#ifndef YUNOS_CONFIG_LITTLE_ENDIAN
#define YUNOS_CONFIG_LITTLE_ENDIAN           1
#endif

#ifndef YUNOS_CONFIG_CPU_STACK_DOWN
#define YUNOS_CONFIG_CPU_STACK_DOWN          1
#endif

#ifndef YUNOS_CONFIG_BITMAP_HW
#define YUNOS_CONFIG_BITMAP_HW               0
#endif

/* kernel feature conf */
#ifndef YUNOS_CONFIG_SEM
#define YUNOS_CONFIG_SEM                     0
#endif

#ifndef YUNOS_CONFIG_QUEUE
#define YUNOS_CONFIG_QUEUE                   0
#endif

#ifndef YUNOS_CONFIG_TASK_SEM
#define YUNOS_CONFIG_TASK_SEM                0
#endif

#ifndef YUNOS_CONFIG_WORKQUEUE
#define YUNOS_CONFIG_WORKQUEUE               0
#endif

#ifndef YUNOS_CONFIG_EVENT_FLAG
#define YUNOS_CONFIG_EVENT_FLAG              0
#endif

#ifndef YUNOS_CONFIG_TIMER
#define YUNOS_CONFIG_TIMER                   0
#endif

#ifndef YUNOS_CONFIG_BUF_QUEUE
#define YUNOS_CONFIG_BUF_QUEUE               0
#endif

#ifndef YUNOS_CONFIG_MM_BLK
#define YUNOS_CONFIG_MM_BLK                  0
#endif

#ifndef YUNOS_CONFIG_MM_BYTE
#define YUNOS_CONFIG_MM_BYTE                 0
#endif

#ifndef YUNOS_CONFIG_MM_BESTFIT
#define YUNOS_CONFIG_MM_BESTFIT              1
#endif

#ifndef YUNOS_CONFIG_TASK_SEM
#define YUNOS_CONFIG_TASK_SEM                0
#endif

/* kernel task conf */
#ifndef YUNOS_CONFIG_TASK_SUSPEND
#define YUNOS_CONFIG_TASK_SUSPEND            0
#endif

#ifndef YUNOS_CONFIG_TASK_INFO
#define YUNOS_CONFIG_TASK_INFO               0
#endif

#ifndef YUNOS_CONFIG_TASK_INFO_NUM
#define YUNOS_CONFIG_TASK_INFO_NUM           2
#endif

#ifndef YUNOS_CONFIG_TASK_DEL
#define YUNOS_CONFIG_TASK_DEL                0
#endif

#ifndef YUNOS_CONFIG_TASK_WAIT_ABORT
#define YUNOS_CONFIG_TASK_WAIT_ABORT         0
#endif

#ifndef YUNOS_CONFIG_SCHED_RR
#define YUNOS_CONFIG_SCHED_RR                1
#endif

#ifndef YUNOS_CONFIG_TIME_SLICE_DEFAULT
#define YUNOS_CONFIG_TIME_SLICE_DEFAULT      50
#endif

#ifndef YUNOS_CONFIG_PRI_MAX
#define YUNOS_CONFIG_PRI_MAX                 62
#endif

#ifndef YUNOS_CONFIG_USER_PRI_MAX
#define YUNOS_CONFIG_USER_PRI_MAX            (YUNOS_CONFIG_PRI_MAX - 2)
#endif

/* kernel mm_region conf */
#ifndef YUNOS_CONFIG_MM_REGION_MUTEX
#define YUNOS_CONFIG_MM_REGION_MUTEX         1
#endif

/* kernel timer&tick conf */
#ifndef YUNOS_CONFIG_HW_COUNT
#define YUNOS_CONFIG_HW_COUNT                0
#endif

#ifndef YUNOS_CONFIG_TICK_TASK
#define YUNOS_CONFIG_TICK_TASK               0
#endif

#if (YUNOS_CONFIG_TICK_TASK > 0)

#ifndef YUNOS_CONFIG_TICK_TASK_STACK_SIZE
#define YUNOS_CONFIG_TICK_TASK_STACK_SIZE    256
#endif

#ifndef YUNOS_CONFIG_TICK_TASK_PRI
#define YUNOS_CONFIG_TICK_TASK_PRI           1
#endif

#endif /* YUNOS_CONFIG_TICK_TASK */

#ifndef YUNOS_CONFIG_DYNTICKLESS
#define YUNOS_CONFIG_DYNTICKLESS             0
#endif

#ifndef YUNOS_CONFIG_TICKS_PER_SECOND
#define YUNOS_CONFIG_TICKS_PER_SECOND        100
#endif

/*Must be 2^n size!, such as 1, 2, 4, 8, 16,32, etc.......*/
#ifndef YUNOS_CONFIG_TICK_HEAD_ARRAY
#define YUNOS_CONFIG_TICK_HEAD_ARRAY         8
#endif

#ifndef YUNOS_CONFIG_TIMER_TASK_STACK_SIZE
#define YUNOS_CONFIG_TIMER_TASK_STACK_SIZE   200
#endif

#ifndef YUNOS_CONFIG_TIMER_RATE
#define YUNOS_CONFIG_TIMER_RATE              1
#endif

#ifndef YUNOS_CONFIG_TIMER_TASK_PRI
#define YUNOS_CONFIG_TIMER_TASK_PRI          5
#endif

/* kernel intrpt conf */
#ifndef YUNOS_CONFIG_INTRPT_STACK_REMAIN_GET
#define YUNOS_CONFIG_INTRPT_STACK_REMAIN_GET 0
#endif

#ifndef YUNOS_CONFIG_INTRPT_MAX_NESTED_LEVEL
#define YUNOS_CONFIG_INTRPT_MAX_NESTED_LEVEL 188u
#endif

#ifndef YUNOS_CONFIG_INTRPT_GUARD
#define YUNOS_CONFIG_INTRPT_GUARD            0
#endif

/* kernel stack ovf check */
#ifndef YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK
#define YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK  0
#endif

#ifndef YUNOS_CONFIG_TASK_STACK_OVF_CHECK
#define YUNOS_CONFIG_TASK_STACK_OVF_CHECK    0
#endif

#ifndef YUNOS_CONFIG_STACK_OVF_CHECK_HW
#define YUNOS_CONFIG_STACK_OVF_CHECK_HW      0
#endif

/* kernel dyn alloc conf */
#ifndef YUNOS_CONFIG_KOBJ_DYN_ALLOC
#define YUNOS_CONFIG_KOBJ_DYN_ALLOC          0
#endif

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)

#ifndef YUNOS_CONFIG_K_DYN_QUEUE_MSG
#define YUNOS_CONFIG_K_DYN_QUEUE_MSG         30
#endif

#ifndef YUNOS_CONFIG_K_DYN_TASK_STACK
#define YUNOS_CONFIG_K_DYN_TASK_STACK        256
#endif

#ifndef YUNOS_CONFIG_K_DYN_MEM_TASK_PRI
#define YUNOS_CONFIG_K_DYN_MEM_TASK_PRI      YUNOS_CONFIG_USER_PRI_MAX
#endif

#endif /* YUNOS_CONFIG_KOBJ_DYN_ALLOC */

/* kernel idle conf */
#ifndef YUNOS_CONFIG_IDLE_TASK_STACK_SIZE
#define YUNOS_CONFIG_IDLE_TASK_STACK_SIZE    100
#endif

/* kernel hook conf */
#ifndef YUNOS_CONFIG_USER_HOOK
#define YUNOS_CONFIG_USER_HOOK               1
#endif

/* kernel stats conf */
#ifndef YUNOS_CONFIG_SYSTEM_STATS
#define YUNOS_CONFIG_SYSTEM_STATS            0
#endif

#ifndef YUNOS_CONFIG_DISABLE_SCHED_STATS
#define YUNOS_CONFIG_DISABLE_SCHED_STATS     0
#endif

#ifndef YUNOS_CONFIG_DISABLE_INTRPT_STATS
#define YUNOS_CONFIG_DISABLE_INTRPT_STATS    0
#endif

#ifndef YUNOS_CONFIG_CPU_USAGE_STATS
#define YUNOS_CONFIG_CPU_USAGE_STATS         0
#endif

#ifndef YUNOS_CONFIG_CPU_USAGE_TASK_PRI
#define YUNOS_CONFIG_CPU_USAGE_TASK_PRI      (YUNOS_CONFIG_PRI_MAX - 2)
#endif

#ifndef YUNOS_CONFIG_TASK_SCHED_STATS
#define YUNOS_CONFIG_TASK_SCHED_STATS        0
#endif

#ifndef YUNOS_CONFIG_CPU_USAGE_TASK_STACK
#define YUNOS_CONFIG_CPU_USAGE_TASK_STACK    256
#endif

/* kernel trace conf */
#ifndef YUNOS_CONFIG_TRACE
#define YUNOS_CONFIG_TRACE                   0
#endif

#if ((YUNOS_CONFIG_DYNTICKLESS >= 1) && (YUNOS_CONFIG_SCHED_RR != 0))
#error  "YUNOS_CONFIG_SCHED_RR should be 0 when YUNOS_CONFIG_DYNTICKLESS is enabled."
#endif

#if ((YUNOS_CONFIG_DYNTICKLESS >= 1) && (YUNOS_CONFIG_TICK_HEAD_ARRAY != 1))
#error  "YUNOS_CONFIG_TICK_HEAD_ARRAY must be 1 when YUNOS_CONFIG_DYNTICKLESS is enabled."
#endif

#if (YUNOS_CONFIG_PRI_MAX >= 256)
#error  "YUNOS_CONFIG_PRI_MAX must be <= 255."
#endif

#if ((YUNOS_CONFIG_SEM == 0) && (YUNOS_CONFIG_TASK_SEM >= 1))
#error  "you need enable YUNOS_CONFIG_SEM as well."
#endif

#if ((YUNOS_CONFIG_HW_COUNT == 0) && (YUNOS_CONFIG_TASK_SCHED_STATS >= 1))
#error  "you need enable YUNOS_CONFIG_HW_COUNT as well."
#endif

#if ((YUNOS_CONFIG_HW_COUNT == 0) && (YUNOS_CONFIG_DISABLE_SCHED_STATS >= 1))
#error  "you need enable YUNOS_CONFIG_HW_COUNT as well."
#endif

#if ((YUNOS_CONFIG_HW_COUNT == 0) && (YUNOS_CONFIG_DISABLE_INTRPT_STATS >= 1))
#error  "you need enable YUNOS_CONFIG_HW_COUNT as well."
#endif

#endif /* K_DEFAULT_CONFIG_H */

