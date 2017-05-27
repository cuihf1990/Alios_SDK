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

#ifndef K_ERR_H
#define K_ERR_H

typedef enum {
    YUNOS_SUCCESS                      = 0u,
    YUNOS_SYS_FATAL_ERR,
    YUNOS_SYS_SP_ERR,
    YUNOS_RUNNING,
    YUNOS_STOPPED,
    YUNOS_INV_PARAM,
    YUNOS_NULL_PTR,
    YUNOS_INV_ALIGN,
    YUNOS_KOBJ_TYPE_ERR,
    YUNOS_KOBJ_DEL_ERR,
    YUNOS_KOBJ_DOCKER_EXIST,
    YUNOS_KOBJ_BLK,
    YUNOS_KOBJ_SET_FULL,
    YUNOS_NOTIFY_FUNC_EXIST,

    YUNOS_MM_POOL_SIZE_ERR             = 100u,
    YUNOS_MM_ALLOC_SIZE_ERR,
    YUNOS_MM_FREE_ADDR_ERR,
    YUNOS_MM_CORRUPT_ERR,
    YUNOS_DYN_MEM_PROC_ERR,
    YUNOS_NO_MEM,
    YUNOS_RINGBUF_FULL,
    YUNOS_RINGBUF_EMPTY,

    YUNOS_SCHED_DISABLE                = 200u,
    YUNOS_SCHED_ALREADY_ENABLED,
    YUNOS_SCHED_LOCK_COUNT_OVF,
    YUNOS_INV_SCHED_WAY,

    YUNOS_TASK_INV_STACK_SIZE          = 300u,
    YUNOS_TASK_NOT_SUSPENDED,
    YUNOS_TASK_DEL_NOT_ALLOWED,
    YUNOS_TASK_SUSPEND_NOT_ALLOWED,
    YUNOS_SUSPENDED_COUNT_OVF,
    YUNOS_BEYOND_MAX_PRI,
    YUNOS_PRI_CHG_NOT_ALLOWED,
    YUNOS_INV_TASK_STATE,
    YUNOS_IDLE_TASK_EXIST,

    YUNOS_NO_PEND_WAIT                 = 400u,
    YUNOS_BLK_ABORT,
    YUNOS_BLK_TIMEOUT,
    YUNOS_BLK_DEL,
    YUNOS_BLK_INV_STATE,
    YUNOS_BLK_POOL_SIZE_ERR,

    YUNOS_TIMER_STATE_INV              = 500u,

    YUNOS_NO_THIS_EVENT_OPT            = 600u,

    YUNOS_BUF_QUEUE_INV_SIZE           = 700u,
    YUNOS_BUF_QUEUE_SIZE_ZERO,
    YUNOS_BUF_QUEUE_FULL,
    YUNOS_BUF_QUEUE_MSG_SIZE_OVERFLOW,
    YUNOS_QUEUE_FULL,
    YUNOS_QUEUE_NOT_FULL,

    YUNOS_SEM_OVF                      = 800u,
    YUNOS_SEM_TASK_WAITING,

    YUNOS_MUTEX_NOT_RELEASED_BY_OWNER  = 900u,
    YUNOS_MUTEX_OWNER_NESTED,
    YUNOS_MUTEX_NESTED_OVF,

    YUNOS_INTRPT_NESTED_LEVEL_OVERFLOW = 1000u,
    YUNOS_INV_INTRPT_NESTED_LEVEL,
    YUNOS_NOT_CALLED_BY_INTRPT,

    YUNOS_WORKQUEUE_EXIST              = 1100u,
    YUNOS_WORKQUEUE_NOT_EXIST,
    YUNOS_WORKQUEUE_WORK_EXIST,
    YUNOS_WORKQUEUE_BUSY,
    YUNOS_WORKQUEUE_WORK_RUNNING,

    YUNOS_TASK_STACK_OVF               = 1200u,
    YUNOS_INTRPT_STACK_OVF
} kstat_t;

typedef void (*yunos_err_proc_t)(kstat_t err);

extern yunos_err_proc_t g_err_proc;

#endif /* K_ERR_H */

