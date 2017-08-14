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

kstat_t mutex_create(kmutex_t *mutex, const name_t *name, uint8_t mm_alloc_flag)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(mutex);
    NULL_PARA_CHK(name);

    /* init the list */
    klist_init(&mutex->blk_obj.blk_list);
    mutex->blk_obj.blk_policy = BLK_POLICY_PRI;
#if (YUNOS_CONFIG_KOBJ_SET > 0)
    mutex->blk_obj.handle = NULL;
#endif
    mutex->blk_obj.name       = name;
    mutex->mutex_task         = NULL;
    mutex->mutex_list         = NULL;
    mutex->mm_alloc_flag      = mm_alloc_flag;

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    YUNOS_CRITICAL_ENTER();
    klist_insert(&(g_kobj_list.mutex_head), &mutex->mutex_item);
    YUNOS_CRITICAL_EXIT();
#endif

    mutex->blk_obj.obj_type = YUNOS_MUTEX_OBJ_TYPE;

    TRACE_MUTEX_CREATE(g_active_task, mutex, name);

    return YUNOS_SUCCESS;
}

kstat_t yunos_mutex_create(kmutex_t *mutex, const name_t *name)
{
    return mutex_create(mutex, name, K_OBJ_STATIC_ALLOC);
}

static void mutex_release(ktask_t *task, kmutex_t *mutex_rel)
{
    uint8_t new_pri;

    /* find suitable task prio */
    new_pri = mutex_pri_look(task, mutex_rel);
    if (new_pri != task->prio) {
        /* change prio */
        task_pri_change(task, new_pri);

        TRACE_MUTEX_RELEASE(g_active_task, task, new_pri);

    }
}

kstat_t yunos_mutex_del(kmutex_t *mutex)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    if (mutex == NULL) {
        return YUNOS_NULL_PTR;
    }

    NULL_PARA_CHK(mutex);

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (mutex->blk_obj.obj_type != YUNOS_MUTEX_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (mutex->mm_alloc_flag != K_OBJ_STATIC_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    blk_list_head = &mutex->blk_obj.blk_list;

    mutex->blk_obj.obj_type = YUNOS_OBJ_TYPE_NONE;

    if (mutex->mutex_task != NULL) {
        mutex_release(mutex->mutex_task, mutex);
    }

    /* all task blocked on this mutex is waken up */
    while (!is_klist_empty(blk_list_head)) {
        pend_task_rm(yunos_list_entry(blk_list_head->next, ktask_t, task_list));
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&mutex->mutex_item);
#endif

    TRACE_MUTEX_DEL(g_active_task, mutex);

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
kstat_t yunos_mutex_dyn_create(kmutex_t **mutex, const name_t *name)
{
    kstat_t  stat;
    kmutex_t *mutex_obj;

    if (mutex == NULL) {
        return YUNOS_NULL_PTR;
    }

    NULL_PARA_CHK(mutex);

    mutex_obj = yunos_mm_alloc(sizeof(kmutex_t));
    if (mutex_obj == NULL) {
        return YUNOS_NO_MEM;
    }

    stat = mutex_create(mutex_obj, name, K_OBJ_DYN_ALLOC);
    if (stat != YUNOS_SUCCESS) {
        yunos_mm_free(mutex_obj);
        return stat;
    }

    *mutex = mutex_obj;

    return stat;
}

kstat_t yunos_mutex_dyn_del(kmutex_t *mutex)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    if (mutex == NULL) {
        return YUNOS_NULL_PTR;
    }

    NULL_PARA_CHK(mutex);

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (mutex->blk_obj.obj_type != YUNOS_MUTEX_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (mutex->mm_alloc_flag != K_OBJ_DYN_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    blk_list_head = &mutex->blk_obj.blk_list;

    mutex->blk_obj.obj_type = YUNOS_OBJ_TYPE_NONE;

    if (mutex->mutex_task != NULL) {
        mutex_release(mutex->mutex_task, mutex);
    }

    /* all task blocked on this mutex is waken up */
    while (!is_klist_empty(blk_list_head)) {
        pend_task_rm(yunos_list_entry(blk_list_head->next, ktask_t, task_list));
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&mutex->mutex_item);
#endif

    TRACE_MUTEX_DEL(g_active_task, mutex);

    YUNOS_CRITICAL_EXIT_SCHED();

    yunos_mm_free(mutex);

    return YUNOS_SUCCESS;
}
#endif

uint8_t mutex_pri_limit(ktask_t *task, uint8_t pri)
{
    kmutex_t *mutex_tmp;
    uint8_t  high_pri;
    ktask_t  *first_blk_task;
    klist_t *blk_list_head;

    high_pri = pri;

    for (mutex_tmp = task->mutex_list; mutex_tmp != NULL;
         mutex_tmp = mutex_tmp->mutex_list) {
        blk_list_head = &mutex_tmp->blk_obj.blk_list;

        if (!is_klist_empty(blk_list_head)) {
            first_blk_task = yunos_list_entry(blk_list_head->next, ktask_t, task_list);
            pri = first_blk_task->prio;
        }

        /* can not set lower prio than the highest prio in all mutexes which hold lock */
        if (pri < high_pri) {
            high_pri = pri;
        }
    }

    return high_pri;
}

uint8_t mutex_pri_look(ktask_t *task, kmutex_t *mutex_rel)
{
    kmutex_t  *mutex_tmp;
    kmutex_t **prev;
    uint8_t   new_pri;
    uint8_t   pri;
    ktask_t  *first_blk_task;
    klist_t  *blk_list_head;

    /* the base prio of task */
    new_pri = task->b_prio;

    /* the highest prio in mutex which is locked */
    pri  = new_pri;
    prev = &task->mutex_list;

    while ((mutex_tmp = *prev) != NULL) {
        if (mutex_tmp == mutex_rel) {
            /* delete itself from list and make task->mutex_list point to next */
            *prev = mutex_tmp->mutex_list;
            continue;
        }

        blk_list_head = &mutex_tmp->blk_obj.blk_list;
        if (!is_klist_empty(blk_list_head)) {
            first_blk_task = yunos_list_entry(blk_list_head->next, ktask_t, task_list);
            pri = first_blk_task->prio;
        }

        if (new_pri > pri) {
            new_pri = pri;
        }

        prev = &mutex_tmp->mutex_list;
    }

    return new_pri;
}

void mutex_task_pri_reset(ktask_t *task)
{
    kmutex_t *mutex_tmp;
    ktask_t *mutex_task;

    if (task->blk_obj->obj_type == YUNOS_MUTEX_OBJ_TYPE) {
        mutex_tmp = (kmutex_t *)(task->blk_obj);
        mutex_task = mutex_tmp->mutex_task;

        /* the new highest prio task blocked on this mutex may decrease prio than before so reset the mutex task prio */
        if (mutex_task->prio == task->prio) {
            mutex_release(mutex_task, NULL);
        }
    }
}

kstat_t yunos_mutex_lock(kmutex_t *mutex, tick_t ticks)
{
    CPSR_ALLOC();

    kstat_t  ret;
    ktask_t *mutex_task;

    NULL_PARA_CHK(mutex);

    if (g_sys_stat == YUNOS_STOPPED) {
        return YUNOS_SUCCESS;
    }

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (mutex->blk_obj.obj_type != YUNOS_MUTEX_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    /* if the same task get the same mutex again, it causes mutex owner nested */
    if (g_active_task == mutex->mutex_task) {
        if (mutex->owner_nested == (mutex_nested_t) - 1) {
            /* fatal error here, system must be stoped here */
            k_err_proc(YUNOS_MUTEX_NESTED_OVF);
            YUNOS_CRITICAL_EXIT();
            return YUNOS_MUTEX_NESTED_OVF;
        } else {
            mutex->owner_nested++;
        }

        YUNOS_CRITICAL_EXIT();

        return YUNOS_MUTEX_OWNER_NESTED;
    }

    mutex_task = mutex->mutex_task;
    if (mutex_task == NULL) {
        /* get lock */
        mutex->mutex_task         = g_active_task;
        mutex->mutex_list         = g_active_task->mutex_list;
        g_active_task->mutex_list = mutex;
        mutex->owner_nested       = 1u;

        TRACE_MUTEX_GET(g_active_task, mutex, ticks);

        YUNOS_CRITICAL_EXIT();

        return YUNOS_SUCCESS;
    }

    /* can't get mutex, and return immediately if wait_option is YUNOS_NO_WAIT */
    if (ticks == YUNOS_NO_WAIT) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_NO_PEND_WAIT;
    }

    /* system is locked so task can not be blocked just return immediately */
    if (g_sched_lock > 0u) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_DISABLE;
    }

    /* if current task is a higher prio task and block on the mutex
       prio inverse condition happened, prio inherit method is used here */
    if (g_active_task->prio < mutex_task->prio) {
        task_pri_change(mutex_task, g_active_task->prio);

        TRACE_TASK_PRI_INV(g_active_task, mutex_task);

    }

    /* any way block the current task */
    pend_to_blk_obj((blk_obj_t *)mutex, g_active_task, ticks);

    TRACE_MUTEX_GET_BLK(g_active_task, mutex, ticks);

    YUNOS_CRITICAL_EXIT_SCHED();

#ifndef YUNOS_CONFIG_PERF_NO_PENDEND_PROC
    YUNOS_CPU_INTRPT_DISABLE();

    /* so the task is waked up, need know which reason cause wake up */
    ret = pend_state_end_proc(g_active_task);

    YUNOS_CPU_INTRPT_ENABLE();
#else
    ret = YUNOS_SUCCESS;
#endif
    return ret;
}

kstat_t yunos_mutex_unlock(kmutex_t *mutex)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;
    ktask_t *task;

    NULL_PARA_CHK(mutex);

    if (g_sys_stat == YUNOS_STOPPED) {
        return YUNOS_SUCCESS;
    }

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (mutex->blk_obj.obj_type != YUNOS_MUTEX_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    /* mutex must be released by itself */
    if (g_active_task != mutex->mutex_task) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_MUTEX_NOT_RELEASED_BY_OWNER;
    }

    mutex->owner_nested--;

    if (mutex->owner_nested > 0u) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_MUTEX_OWNER_NESTED;
    }

    mutex_release(g_active_task, mutex);

    blk_list_head = &mutex->blk_obj.blk_list;

    /* if no block task on this list just return */
    if (is_klist_empty(blk_list_head)) {
        /* No wait task */
        mutex->mutex_task = NULL;

        TRACE_MUTEX_RELEASE_SUCCESS(g_active_task, mutex);
        YUNOS_CRITICAL_EXIT();

        return YUNOS_SUCCESS;
    }

    /* there must have task blocked on this mutex object */
    task = yunos_list_entry(blk_list_head->next, ktask_t, task_list);

    /* wake up the occupy task, which is the highst prio task on the list */
    pend_task_wakeup(task);

    TRACE_MUTEX_TASK_WAKE(g_active_task, task, mutex);

    /* change mutex get task */
    mutex->mutex_task   = task;
    mutex->mutex_list   = task->mutex_list;
    task->mutex_list    = mutex;
    mutex->owner_nested = 1u;

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}


kstat_t yunos_mutex_is_valid(kmutex_t *mutex)
{
    NULL_PARA_CHK(mutex);

    if(mutex->blk_obj.obj_type != YUNOS_MUTEX_OBJ_TYPE)
    {
      return YUNOS_KOBJ_TYPE_ERR;
    }
    
    return YUNOS_SUCCESS;
}

