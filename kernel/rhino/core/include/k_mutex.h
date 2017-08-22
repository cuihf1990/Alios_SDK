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

#ifndef K_MUTEX_H
#define K_MUTEX_H

typedef struct mutex_s {
    blk_obj_t       blk_obj;
    ktask_t        *mutex_task; /* mutex owner task */
    struct mutex_s *mutex_list; /* task mutex list */
    mutex_nested_t  owner_nested;

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_t         mutex_item;
#endif

    uint8_t         mm_alloc_flag;
} kmutex_t;

/**
 * This function will create a mutex
 * @param[in] mutex  pointer to the mutex(the space is provided by user)
 * @param[in] name   name of the mutex
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mutex_create(kmutex_t *mutex, const name_t *name);

/**
 * This function will delete a mutex
 * @param[in] mutex pointer to the mutex
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mutex_del(kmutex_t *mutex);

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
/**
 * This function will create a dyn mutex
 * @param[in]  mutex  pointer to the mutex(the space is provided by user)
 * @param[in]  name   name of the mutex
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mutex_dyn_create(kmutex_t **mutex, const name_t *name);

/**
 * This function will delete a dyn mutex
 * @param[in] mutex  pointer to the mutex
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mutex_dyn_del(kmutex_t *mutex);
#endif

/**
 * This function will lock mutex
 * @param[in]  mutex  pointer to the mutex
 * @param[in]  ticks  ticks to be wait for before lock
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mutex_lock(kmutex_t *mutex, tick_t ticks);

/**
 * This function will unlock a mutex
 * @param[in]  mutex  pointer to the mutex
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mutex_unlock(kmutex_t *mutex);

/**
 * This function will check if mutex is valid
 * @param[in]   mutex    pointer to the mutex
 * @return  the check status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_mutex_is_valid(kmutex_t *mutex);

#endif /* K_MUTEX_H */

