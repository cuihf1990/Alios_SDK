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

#ifndef K_OBJ_H
#define K_OBJ_H

typedef enum {
    BLK_POLICY_PRI = 0u,
    BLK_POLICY_FIFO
} blk_policy_t;

typedef enum {
    BLK_FINISH = 0,
    BLK_ABORT,
    BLK_TIMEOUT,
    BLK_DEL,
    BLK_INVALID
} blk_state_t;

typedef enum {
    YUNOS_OBJ_TYPE_NONE = 0,
    YUNOS_SEM_OBJ_TYPE,
    YUNOS_MUTEX_OBJ_TYPE,
    YUNOS_QUEUE_OBJ_TYPE,
    YUNOS_BUF_QUEUE_OBJ_TYPE,
    YUNOS_TIMER_OBJ_TYPE,
    YUNOS_EVENT_OBJ_TYPE,
    YUNOS_MM_BLK_OBJ_TYPE,
    YUNOS_MM_OBJ_TYPE
} kobj_type_t;

#if (YUNOS_CONFIG_KOBJ_SET > 0)
struct kobj_set;
#endif

typedef struct blk_obj {
    klist_t          blk_list;
    const name_t    *name;
    blk_policy_t     blk_policy;
    kobj_type_t      obj_type;
#if (YUNOS_CONFIG_KOBJ_SET > 0)
    struct kobj_set *handle;
#endif
} blk_obj_t;

typedef struct {
    klist_t task_head;
    klist_t mutex_head;
#if (YUNOS_CONFIG_MM_FIRSTFIT > 0)
    klist_t mmpool_head;
#endif

#if (YUNOS_CONFIG_MM_BLK > 0)
    klist_t mblkpool_head;
#endif

#if (YUNOS_CONFIG_SEM > 0)
    klist_t sem_head;
#endif

#if (YUNOS_CONFIG_QUEUE > 0)
    klist_t queue_head;
#endif

#if (YUNOS_CONFIG_EVENT_FLAG > 0)
    klist_t event_head;
#endif

#if (YUNOS_CONFIG_BUF_QUEUE > 0)
    klist_t buf_queue_head;
#endif
} kobj_list_t;

#endif /* K_OBJ_H */

