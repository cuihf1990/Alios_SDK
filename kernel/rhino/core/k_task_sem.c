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

#include <k_api.h>

#if (YUNOS_CONFIG_TASK_SEM > 0)
kstat_t yunos_task_sem_create(ktask_t *task, ksem_t *sem, const name_t *name, size_t count)
{
    kstat_t ret;

    if (task == NULL) {
        return YUNOS_NULL_PTR;
    }

    NULL_PARA_CHK(task);

    ret = yunos_sem_create(sem, name, count);
    if (ret == YUNOS_SUCCESS) {
        task->task_sem_obj = sem;
    } else {
        task->task_sem_obj = NULL;
    }

    return ret;
}

kstat_t yunos_task_sem_del(ktask_t *task)
{
    NULL_PARA_CHK(task);

    return yunos_sem_del(task->task_sem_obj);
}

kstat_t yunos_task_sem_give(ktask_t *task)
{
    NULL_PARA_CHK(task);

    return yunos_sem_give(task->task_sem_obj);
}

kstat_t yunos_task_sem_take(tick_t ticks)
{
    return yunos_sem_take(g_active_task->task_sem_obj, ticks);
}

kstat_t yunos_task_sem_count_set(ktask_t *task, sem_count_t count)
{
    NULL_PARA_CHK(task);

    return yunos_sem_count_set(task->task_sem_obj, count);
}

kstat_t yunos_task_sem_count_get(ktask_t *task, sem_count_t *count)
{
    NULL_PARA_CHK(task);

    return yunos_sem_count_get(task->task_sem_obj, count);
}
#endif

