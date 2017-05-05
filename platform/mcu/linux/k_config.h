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

#ifndef K_CONFIG_H
#define K_CONFIG_H

/* chip level conf */
#define YUNOS_CONFIG_LITTLE_ENDIAN           1
#define YUNOS_CONFIG_CPU_STACK_DOWN          1

/* kernel feature conf */
#define YUNOS_CONFIG_SEM                     1
#define YUNOS_CONFIG_QUEUE                   1
#define YUNOS_CONFIG_TASK_SEM                1
#define YUNOS_CONFIG_EVENT_FLAG              1
#define YUNOS_CONFIG_TIMER                   1
#define YUNOS_CONFIG_BUF_QUEUE               1
#define YUNOS_CONFIG_MM_BLK                  1
#define YUNOS_CONFIG_MM_FIRSTFIT             1
#define YUNOS_CONFIG_MM_BESTFIT              1
#define YUNOS_CONFIG_MM_DEBUG                1
#define YUNOS_CONFIG_KOBJ_SET                1
#define YUNOS_CONFIG_RINGBUF_VENDOR          1

/* kernel task conf */
#define YUNOS_CONFIG_TASK_SUSPEND            1
#define YUNOS_CONFIG_TASK_INFO               10
#define YUNOS_CONFIG_TASK_DEL                1
#define YUNOS_CONFIG_TASK_WAIT_ABORT         0
#define YUNOS_CONFIG_TASK_STACK_OVF_CHECK    0
#define YUNOS_CONFIG_SCHED_RR                1
#define YUNOS_CONFIG_TIME_SLICE_DEFAULT      10
#define YUNOS_CONFIG_PRI_MAX                 62
#define YUNOS_CONFIG_USER_PRI_MAX            (YUNOS_CONFIG_PRI_MAX - 2)

/* kernel workqueue conf */
#define YUNOS_CONFIG_WORKQUEUE               1

/* kernel mm_region conf */
#define YUNOS_CONFIG_MM_REGION_MUTEX         0

/* kernel timer&tick conf */
#define YUNOS_CONFIG_HW_COUNT                1
#define YUNOS_CONFIG_TICK_TASK               1
#if (YUNOS_CONFIG_TICK_TASK > 0)
#define YUNOS_CONFIG_TICK_TASK_STACK_SIZE    256
#define YUNOS_CONFIG_TICK_TASK_PRI           1
#endif
#define YUNOS_CONFIG_TICKLESS                0
#define YUNOS_CONFIG_TICKS_PER_SECOND        10
/* must be 2^n size!, such as 1, 2, 4, 8, 16,32, etc....... */
#define YUNOS_CONFIG_TICK_HEAD_ARRAY         8
#define YUNOS_CONFIG_TIMER_TASK_STACK_SIZE   200
#define YUNOS_CONFIG_TIMER_RATE              1
#define YUNOS_CONFIG_TIMER_TASK_PRI          5

/* kernel intrpt conf */
#define YUNOS_CONFIG_INTRPT_STACK_REMAIN_GET 1
#define YUNOS_CONFIG_INTRPT_STACK_OVF_CHECK  0
#define YUNOS_CONFIG_INTRPT_MAX_NESTED_LEVEL 188u
#define YUNOS_CONFIG_INTRPT_GUARD            0

/* kernel dyn alloc conf */
#define YUNOS_CONFIG_KOBJ_DYN_ALLOC          1
#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
#define YUNOS_CONFIG_K_DYN_QUEUE_MSG         30
#define YUNOS_CONFIG_K_DYN_TASK_STACK        256
#define YUNOS_CONFIG_K_DYN_MEM_TASK_PRI      YUNOS_CONFIG_USER_PRI_MAX
#endif

/* kernel idle conf */
#define YUNOS_CONFIG_IDLE_TASK_STACK_SIZE    100

/* kernel hook conf */
#define YUNOS_CONFIG_USER_HOOK               1

/* kernel stats conf */
#define YUNOS_CONFIG_SYSTEM_STATS            1
#define YUNOS_CONFIG_DISABLE_SCHED_STATS     1
#define YUNOS_CONFIG_DISABLE_INTRPT_STATS    1
#define YUNOS_CONFIG_CPU_USAGE_STATS         0
#define YUNOS_CONFIG_CPU_USAGE_TASK_PRI      (YUNOS_CONFIG_PRI_MAX - 2)
#define YUNOS_CONFIG_TASK_SCHED_STATS        0
#define YUNOS_CONFIG_CPU_USAGE_TASK_STACK    256

/* kernel trace conf */
#define YUNOS_CONFIG_TRACE                   0

#endif /* K_CONFIG_H */

