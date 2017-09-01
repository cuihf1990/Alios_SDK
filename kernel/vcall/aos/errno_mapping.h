/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef ERRNO_MAPPING_H
#define ERRNO_MAPPING_H

#include <errno.h>
#include <yos/yos.h>

#define ERRNO_MAPPING(ret)                           \
    {                                                \
        switch(ret) {                                \
            case YUNOS_SYS_SP_ERR:                   \
            case YUNOS_NULL_PTR:                     \
            case YUNOS_MM_FREE_ADDR_ERR:             \
                return -EFAULT;                      \
            case YUNOS_INV_PARAM:                    \
            case YUNOS_INV_ALIGN:                    \
            case YUNOS_KOBJ_TYPE_ERR:                \
            case YUNOS_MM_POOL_SIZE_ERR:             \
            case YUNOS_MM_ALLOC_SIZE_ERR:            \
            case YUNOS_INV_SCHED_WAY:                \
            case YUNOS_TASK_INV_STACK_SIZE:          \
            case YUNOS_BEYOND_MAX_PRI:               \
            case YUNOS_BUF_QUEUE_INV_SIZE:           \
            case YUNOS_BUF_QUEUE_SIZE_ZERO:          \
            case YUNOS_BUF_QUEUE_MSG_SIZE_OVERFLOW:  \
            case YUNOS_QUEUE_FULL:                   \
            case YUNOS_QUEUE_NOT_FULL:               \
            case YUNOS_SEM_OVF:                      \
            case YUNOS_WORKQUEUE_EXIST:              \
            case YUNOS_WORKQUEUE_NOT_EXIST:          \
            case YUNOS_WORKQUEUE_WORK_EXIST:         \
                return -EINVAL;                      \
            case YUNOS_KOBJ_BLK:                     \
                return -EAGAIN;                      \
            case YUNOS_NO_MEM:                       \
                return -ENOMEM;                      \
            case YUNOS_KOBJ_DEL_ERR:                 \
            case YUNOS_SCHED_DISABLE:                \
            case YUNOS_SCHED_ALREADY_ENABLED:        \
            case YUNOS_SCHED_LOCK_COUNT_OVF:         \
            case YUNOS_TASK_NOT_SUSPENDED:           \
            case YUNOS_TASK_DEL_NOT_ALLOWED:         \
            case YUNOS_TASK_SUSPEND_NOT_ALLOWED:     \
            case YUNOS_SUSPENDED_COUNT_OVF:          \
            case YUNOS_PRI_CHG_NOT_ALLOWED:          \
            case YUNOS_NOT_CALLED_BY_INTRPT:         \
            case YUNOS_NO_THIS_EVENT_OPT:            \
            case YUNOS_TIMER_STATE_INV:              \
            case YUNOS_BUF_QUEUE_FULL:               \
            case YUNOS_SEM_TASK_WAITING:             \
            case YUNOS_MUTEX_NOT_RELEASED_BY_OWNER:  \
            case YUNOS_WORKQUEUE_WORK_RUNNING:       \
                return -EPERM;                       \
            case YUNOS_TRY_AGAIN:                    \
            case YUNOS_WORKQUEUE_BUSY:               \
                return -EAGAIN;                      \
            default:                                 \
                return -1;                           \
        }                                            \
    }

#endif /* ERRNO_MAPPING_H */
