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

#ifndef K_TASK_SEM_H
#define K_TASK_SEM_H

/**
 * This function will create a task-semaphore
 * @param[in]  task   pointer to the task
 * @param[in]  sem    pointer to the semaphore
 * @param[in]  name   name of the task-semaphore
 * @param[in]  count  count of the semaphore
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_task_sem_create(ktask_t *task, ksem_t *sem, const name_t *name,
                              size_t count);

/**
 * This function will delete a task-semaphore
 * @param[in]  task  pointer to the semaphore
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_task_sem_del(ktask_t *task);

/**
 * This function will give up a task-semaphore
 * @param[in]  task  pointer to the task
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_task_sem_give(ktask_t *task);

/**
 * This function will take a task-semaphore
 * @param[in]  ticks  ticks to wait before take the semaphore
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_task_sem_take(tick_t ticks);

/**
 * This function will set the count of a task-semaphore
 * @param[in]  task   pointer to the task
 * @param[in]  count  count of the semaphre to set
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_task_sem_count_set(ktask_t *task, sem_count_t count);

/**
 * This function will get task-semaphore count
 * @param[in]   task   pointer to the semphore
 * @param[out]  count  count of the semaphore
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
kstat_t yunos_task_sem_count_get(ktask_t *task, sem_count_t *count);

#endif /* K_TASK_SEM_H */

