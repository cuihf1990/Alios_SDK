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

#if (YUNOS_CONFIG_SEM > 0)
static kstat_t sem_create(ksem_t *sem, const name_t *name, sem_count_t count,
                          uint8_t mm_alloc_flag)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(sem);
    NULL_PARA_CHK(name);

    /* init the list */
    klist_init(&sem->blk_obj.blk_list);

    /* init resource */
    sem->count              = count;
    sem->peak_count         = count;
    sem->blk_obj.name       = name;
    sem->blk_obj.blk_policy = BLK_POLICY_PRI;
#if (YUNOS_CONFIG_KOBJ_SET > 0)
    sem->blk_obj.handle = NULL;
#endif
    sem->mm_alloc_flag      = mm_alloc_flag;

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    YUNOS_CRITICAL_ENTER();
    klist_insert(&(g_kobj_list.sem_head), &sem->sem_item);
    YUNOS_CRITICAL_EXIT();
#endif

    sem->blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;

    TRACE_SEM_CREATE(g_active_task, sem);

    return YUNOS_SUCCESS;
}

kstat_t yunos_sem_create(ksem_t *sem, const name_t *name, sem_count_t count)
{
    return sem_create(sem, name, count, K_OBJ_STATIC_ALLOC);
}

kstat_t yunos_sem_del(ksem_t *sem)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    NULL_PARA_CHK(sem);

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (sem->blk_obj.obj_type != YUNOS_SEM_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (sem->mm_alloc_flag != K_OBJ_STATIC_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    blk_list_head = &sem->blk_obj.blk_list;
    sem->blk_obj.obj_type = YUNOS_OBJ_TYPE_NONE;

    /* all task blocked on this queue is waken up */
    while (!is_klist_empty(blk_list_head)) {
        pend_task_rm(yunos_list_entry(blk_list_head->next, ktask_t, task_list));
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&sem->sem_item);
#endif

    TRACE_SEM_DEL(g_active_task, sem);
    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
kstat_t yunos_sem_dyn_create(ksem_t **sem, const name_t *name,
                             sem_count_t count)
{
    kstat_t stat;
    ksem_t  *sem_obj;

    NULL_PARA_CHK(sem);

    sem_obj = yunos_mm_alloc(sizeof(ksem_t));

    if (sem_obj == NULL) {
        return YUNOS_NO_MEM;
    }

    stat = sem_create(sem_obj, name, count, K_OBJ_DYN_ALLOC);

    if (stat != YUNOS_SUCCESS) {
        yunos_mm_free(sem_obj);
        return stat;
    }

    *sem = sem_obj;

    return stat;
}

kstat_t yunos_sem_dyn_del(ksem_t *sem)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    NULL_PARA_CHK(sem);

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (sem->blk_obj.obj_type != YUNOS_SEM_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (sem->mm_alloc_flag != K_OBJ_DYN_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    blk_list_head = &sem->blk_obj.blk_list;
    sem->blk_obj.obj_type = YUNOS_OBJ_TYPE_NONE;

    /* all task blocked on this queue is waken up */
    while (!is_klist_empty(blk_list_head)) {
        pend_task_rm(yunos_list_entry(blk_list_head->next, ktask_t, task_list));
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&sem->sem_item);
#endif

    TRACE_SEM_DEL(g_active_task, sem);
    YUNOS_CRITICAL_EXIT_SCHED();

    yunos_mm_free(sem);

    return YUNOS_SUCCESS;
}

#endif

static kstat_t sem_give(ksem_t *sem, uint8_t opt_wake_all)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    /* this is only needed when system zero interrupt feature is enabled */
#if (YUNOS_CONFIG_INTRPT_GUARD > 0)
    soc_intrpt_guard();
#endif

    YUNOS_CRITICAL_ENTER();

    if (sem->blk_obj.obj_type != YUNOS_SEM_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    blk_list_head = &sem->blk_obj.blk_list;

    if (is_klist_empty(blk_list_head)) {
        if (sem->count == (sem_count_t) - 1) {

            TRACE_SEM_OVERFLOW(g_active_task, sem);
            YUNOS_CRITICAL_EXIT();

            return YUNOS_SEM_OVF;
        }

        /* increase resource */
        sem->count++;

        if (sem->count > sem->peak_count) {
            sem->peak_count = sem->count;
        }

        TRACE_SEM_CNT_INCREASE(g_active_task, sem);
        YUNOS_CRITICAL_EXIT();

#if (YUNOS_CONFIG_KOBJ_SET > 0)
        if (sem->blk_obj.handle != NULL) {
            sem->blk_obj.handle->notify((blk_obj_t *)sem, sem->blk_obj.handle);
        }
#endif
        return YUNOS_SUCCESS;
    }

    /* wake all the task blocked on this semaphore */
    if (opt_wake_all) {
        while (!is_klist_empty(blk_list_head)) {
            TRACE_SEM_TASK_WAKE(g_active_task, yunos_list_entry(blk_list_head->next,
                                                                ktask_t, task_list),
                                sem, opt_wake_all);

            pend_task_wakeup(yunos_list_entry(blk_list_head->next, ktask_t, task_list));
        }

    } else {
        TRACE_SEM_TASK_WAKE(g_active_task, yunos_list_entry(blk_list_head->next,
                                                            ktask_t, task_list),
                            sem, opt_wake_all);

        /* wake up the highest prio task block on the semaphore */
        pend_task_wakeup(yunos_list_entry(blk_list_head->next, ktask_t, task_list));
    }

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

kstat_t yunos_sem_give(ksem_t *sem)
{
    NULL_PARA_CHK(sem);

    return sem_give(sem, WAKE_ONE_SEM);
}

kstat_t yunos_sem_give_all(ksem_t *sem)
{
    NULL_PARA_CHK(sem);

    return sem_give(sem, WAKE_ALL_SEM);
}

kstat_t yunos_sem_take(ksem_t *sem, tick_t ticks)
{
    CPSR_ALLOC();

    kstat_t stat;

    NULL_PARA_CHK(sem);

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (sem->blk_obj.obj_type != YUNOS_SEM_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (sem->count > 0u) {
        sem->count--;

        TRACE_SEM_GET_SUCCESS(g_active_task, sem);
        YUNOS_CRITICAL_EXIT();

        return YUNOS_SUCCESS;
    }

    /* can't get semphore, and return immediately if wait_option is  YUNOS_NO_WAIT */
    if (ticks == YUNOS_NO_WAIT) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_NO_PEND_WAIT;
    }

    if (g_sched_lock > 0u) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_DISABLE;
    }

    pend_to_blk_obj((blk_obj_t *)sem, g_active_task, ticks);

    TRACE_SEM_GET_BLK(g_active_task, sem, ticks);

    YUNOS_CRITICAL_EXIT_SCHED();

#ifndef YUNOS_CONFIG_PERF_NO_PENDEND_PROC
    YUNOS_CPU_INTRPT_DISABLE();

    stat = pend_state_end_proc(g_active_task);

    YUNOS_CPU_INTRPT_ENABLE();
#else
    stat = YUNOS_SUCCESS;
#endif

    return stat;
}

kstat_t yunos_sem_count_set(ksem_t *sem, sem_count_t sem_count)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    NULL_PARA_CHK(sem);

    blk_list_head = &sem->blk_obj.blk_list;

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (sem->blk_obj.obj_type != YUNOS_SEM_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    /* set new count */
    if (sem->count > 0u) {
        sem->count = sem_count;
    } else {
        if (is_klist_empty(blk_list_head)) {
            sem->count = sem_count;
        } else {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_SEM_TASK_WAITING;
        }
    }

    /* update sem peak count if need */
    if (sem->count > sem->peak_count) {
        sem->peak_count = sem->count;
    }

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_sem_count_get(ksem_t *sem, sem_count_t *count)
{
    NULL_PARA_CHK(sem);
    NULL_PARA_CHK(count);
    *count = sem->count;

    return YUNOS_SUCCESS;
}

kstat_t yunos_sem_is_valid(ksem_t *sem)
{
    NULL_PARA_CHK(sem);

    if(sem->blk_obj.obj_type != YUNOS_SEM_OBJ_TYPE)
    {
      return YUNOS_KOBJ_TYPE_ERR;
    }
    
    return YUNOS_SUCCESS;
}

#endif /* YUNOS_CONFIG_SEM */

