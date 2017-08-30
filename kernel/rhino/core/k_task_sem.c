/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>

#if (YUNOS_CONFIG_TASK_SEM > 0)
kstat_t yunos_task_sem_create(ktask_t *task, ksem_t *sem, const name_t *name,
                              size_t count)
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
    return yunos_sem_take(yunos_cur_task_get()->task_sem_obj, ticks);
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

