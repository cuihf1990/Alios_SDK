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

static kstat_t task_create(ktask_t *task, const name_t *name, void *arg,
                           uint8_t prio, tick_t ticks, cpu_stack_t *stack_buf,
                           size_t stack_size, task_entry_t entry, uint8_t autorun,
                           uint8_t mm_alloc_flag, uint8_t cpu_num, uint8_t cpu_binded)
{
    CPSR_ALLOC();

    cpu_stack_t *tmp;

    NULL_PARA_CHK(task);
    NULL_PARA_CHK(name);
    NULL_PARA_CHK(entry);
    NULL_PARA_CHK(stack_buf);

    if (stack_size == 0u) {
        return YUNOS_TASK_INV_STACK_SIZE;
    }

    if (prio >= YUNOS_CONFIG_PRI_MAX) {
        return YUNOS_BEYOND_MAX_PRI;
    }

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    /* idle task is only allowed to create once */
    if (prio == YUNOS_IDLE_PRI) {
        if (g_idle_task_spawned[cpu_num] > 0u) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_IDLE_TASK_EXIST;
        }

        g_idle_task_spawned[cpu_num] = 1u;
    }

    YUNOS_CRITICAL_EXIT();

    memset(task, 0, sizeof(ktask_t));

#if (YUNOS_CONFIG_SCHED_RR > 0)
    if (ticks > 0u) {
        task->time_total = ticks;
    } else {
        task->time_total = YUNOS_CONFIG_TIME_SLICE_DEFAULT;
    }

    task->time_slice   = task->time_total;
    task->sched_policy = KSCHED_RR;
#endif

    if (autorun > 0u) {
        task->task_state    = K_RDY;
    } else {
        task->task_state    = K_SUSPENDED;
        task->suspend_count = 1u;
    }

    /* init all the stack element to 0 */
    task->task_stack_base = stack_buf;
    tmp = stack_buf;

    memset(tmp, 0, stack_size * sizeof(cpu_stack_t));

    task->task_name     = name;
    task->prio          = prio;
    task->b_prio        = prio;
    task->stack_size    = stack_size;
    task->mm_alloc_flag = mm_alloc_flag;
    task->cpu_num       = cpu_num;
    cpu_binded          = cpu_binded;

#if (YUNOS_CONFIG_CPU_NUM > 1)
    task->cpu_binded    = cpu_binded;
#endif

#if (YUNOS_CONFIG_TASK_STACK_OVF_CHECK > 0)
#if (YUNOS_CONFIG_CPU_STACK_DOWN > 0)
    tmp  = task->task_stack_base;
    *tmp = YUNOS_TASK_STACK_OVF_MAGIC;
#else
    tmp  = (cpu_stack_t *)(task->task_stack_base) + task->stack_size - 1u;
    *tmp = YUNOS_TASK_STACK_OVF_MAGIC;
#endif
#endif

    task->task_stack = cpu_task_stack_init(stack_buf, stack_size, arg, entry);

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_task_create_hook(task);
#endif

    TRACE_TASK_CREATE(task);

    YUNOS_CRITICAL_ENTER();

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_insert(&(g_kobj_list.task_head), &task->task_stats_item);
#endif

    if (autorun > 0u) {
        ready_list_add_tail(&g_ready_queue, task);
        /* if system is not start,not call core_sched */
        if (g_sys_stat == YUNOS_RUNNING) {
            YUNOS_CRITICAL_EXIT_SCHED();
            return YUNOS_SUCCESS;
        }
    }

    YUNOS_CRITICAL_EXIT();
    return YUNOS_SUCCESS;
}

kstat_t yunos_task_create(ktask_t *task, const name_t *name, void *arg,
                          uint8_t prio, tick_t ticks, cpu_stack_t *stack_buf,
                          size_t stack_size, task_entry_t entry, uint8_t autorun)
{
    return task_create(task, name, arg, prio, ticks, stack_buf, stack_size, entry,
                       autorun, K_OBJ_STATIC_ALLOC, 0, 0);
}

#if (YUNOS_CONFIG_CPU_NUM > 1)
kstat_t yunos_task_cpu_create(ktask_t *task, const name_t *name, void *arg,
                              uint8_t prio, tick_t ticks, cpu_stack_t *stack_buf,
                              size_t stack_size, task_entry_t entry, uint8_t cpu_num,
                              uint8_t autorun)
{
    return task_create(task, name, arg, prio, ticks, stack_buf, stack_size, entry,
                       autorun, K_OBJ_STATIC_ALLOC, cpu_num, 1);
}
#endif

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
kstat_t task_dyn_create(ktask_t **task, const name_t *name, void *arg,
                        uint8_t pri, tick_t ticks, size_t stack, task_entry_t entry,
                        uint8_t cpu_num, uint8_t cpu_binded, uint8_t autorun)
{
    kstat_t      ret;
    cpu_stack_t *task_stack;
    ktask_t     *task_obj;

    NULL_PARA_CHK(task);

    if (stack == 0) {
        return YUNOS_INV_PARAM;
    }

    task_stack = yunos_mm_alloc(stack * sizeof(cpu_stack_t));
    if (task_stack == NULL) {
        return YUNOS_NO_MEM;
    }

    task_obj = yunos_mm_alloc(sizeof(ktask_t));
    if (task_obj == NULL) {
        yunos_mm_free(task_stack);
        return YUNOS_NO_MEM;
    }

    *task = task_obj;

    ret = task_create(task_obj, name, arg, pri, ticks, task_stack, stack, entry,
                      autorun, K_OBJ_DYN_ALLOC, cpu_num, cpu_binded);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        yunos_mm_free(task_stack);
        yunos_mm_free(task_obj);
        *task = NULL;
        return ret;
    }

    return ret;
}

kstat_t yunos_task_dyn_create(ktask_t **task, const name_t *name, void *arg,
                              uint8_t pri, tick_t ticks, size_t stack,
                              task_entry_t entry, uint8_t autorun)
{
    return task_dyn_create(task, name, arg, pri, ticks, stack, entry, 0, 0, autorun);
}

#endif

kstat_t yunos_task_sleep(tick_t ticks)
{
    CPSR_ALLOC();

    uint8_t cur_cpu_num;

    kstat_t ret;

    if (ticks == 0u) {
        return YUNOS_INV_PARAM;
    }

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    cur_cpu_num = cpu_cur_get();

    /* system is locked so task can not be blocked just return immediately */
    if (g_sched_lock[cur_cpu_num] > 0u) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_DISABLE;
    }

    g_active_task[cur_cpu_num]->task_state = K_SLEEP;

#if (YUNOS_CONFIG_DYNTICKLESS > 0)
    g_elapsed_ticks = soc_elapsed_ticks_get();
    tick_list_insert(g_active_task[cur_cpu_num], ticks + g_elapsed_ticks);
#else
    tick_list_insert(g_active_task[cur_cpu_num], ticks);
#endif

    ready_list_rm(&g_ready_queue, g_active_task[cur_cpu_num]);

    TRACE_TASK_SLEEP(g_active_task[cur_cpu_num], ticks);

    YUNOS_CRITICAL_EXIT_SCHED();

#ifndef YUNOS_CONFIG_PERF_NO_PENDEND_PROC
    YUNOS_CPU_INTRPT_DISABLE();

    /* is task timeout normally after sleep */
    ret = pend_state_end_proc(g_active_task[cpu_cur_get()]);

    YUNOS_CPU_INTRPT_ENABLE();
#else
    ret = YUNOS_SUCCESS;
#endif

    return ret;
}

kstat_t yunos_task_yield(void)
{
    CPSR_ALLOC();

    /* make current task to the end of ready list */
    YUNOS_CRITICAL_ENTER();

    ready_list_head_to_tail(&g_ready_queue, g_active_task[cpu_cur_get()]);

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_TASK_SUSPEND > 0)
kstat_t task_suspend(ktask_t *task)
{
    CPSR_ALLOC();

    uint8_t cur_cpu_num;

    YUNOS_CRITICAL_ENTER();

    cur_cpu_num = cpu_cur_get();

#if (YUNOS_CONFIG_CPU_NUM > 1)
    if (task->cpu_num != cur_cpu_num) {
        if (task->cur_exc == 1) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_TRY_AGAIN;
        }
    }
#endif

    if (task == g_active_task[cur_cpu_num]) {
        if (g_sched_lock[cur_cpu_num] > 0u) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_SCHED_DISABLE;
        }
    }

    switch (task->task_state) {
        case K_RDY:
            task->suspend_count = 1u;
            task->task_state    = K_SUSPENDED;
            ready_list_rm(&g_ready_queue, task);
            break;
        case K_SLEEP:
            task->suspend_count = 1u;
            task->task_state    = K_SLEEP_SUSPENDED;
            break;
        case K_PEND:
            task->suspend_count = 1u;
            task->task_state    = K_PEND_SUSPENDED;
            break;
        case K_SUSPENDED:
        case K_SLEEP_SUSPENDED:
        case K_PEND_SUSPENDED:
            if (task->suspend_count == (suspend_nested_t) - 1) {
                YUNOS_CRITICAL_EXIT();
                return YUNOS_SUSPENDED_COUNT_OVF;
            }

            task->suspend_count++;
            break;
        case K_SEED:
        default:
            YUNOS_CRITICAL_EXIT();
            return YUNOS_INV_TASK_STATE;
    }

    TRACE_TASK_SUSPEND(g_active_task[cur_cpu_num], task);

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

kstat_t yunos_task_suspend(ktask_t *task)
{
    if (task == NULL) {
        return YUNOS_NULL_PTR;
    }

    if (task->prio == YUNOS_IDLE_PRI) {
        return YUNOS_TASK_SUSPEND_NOT_ALLOWED;
    }

    return task_suspend(task);
}

kstat_t task_resume(ktask_t *task)
{
    CPSR_ALLOC();

    YUNOS_CRITICAL_ENTER();

    switch (task->task_state) {
        case K_RDY:
        case K_SLEEP:
        case K_PEND:
            YUNOS_CRITICAL_EXIT();
            return YUNOS_TASK_NOT_SUSPENDED;
        case K_SUSPENDED:
            task->suspend_count--;

            if (task->suspend_count == 0u) {
                /* Make task ready */
                task->task_state = K_RDY;
                ready_list_add(&g_ready_queue, task);
            }

            break;
        case K_SLEEP_SUSPENDED:
            task->suspend_count--;

            if (task->suspend_count == 0u) {
                task->task_state = K_SLEEP;
            }

            break;
        case K_PEND_SUSPENDED:
            task->suspend_count--;

            if (task->suspend_count == 0u) {
                task->task_state = K_PEND;
            }

            break;
        case K_SEED:
        default:
            YUNOS_CRITICAL_EXIT();
            return YUNOS_INV_TASK_STATE;
    }

    TRACE_TASK_RESUME(g_active_task[cpu_cur_get()], task);

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

kstat_t yunos_task_resume(ktask_t *task)
{
    NULL_PARA_CHK(task);

    return task_resume(task);
}
#endif

kstat_t yunos_task_stack_min_free(ktask_t *task, size_t *free)
{
    cpu_stack_t *task_stack;
    size_t free_stk = 0;

    NULL_PARA_CHK(task);
    NULL_PARA_CHK(free);

    if (task->task_state == K_DELETED) {
        return YUNOS_INV_TASK_STATE;
    }

#if (YUNOS_CONFIG_CPU_STACK_DOWN > 0)
    task_stack = task->task_stack_base + 1u;
    while (*task_stack++ == 0u) {
        free_stk++;
    }
#else
    task_stack = (cpu_stack_t *)(task->task_stack_base) + task->stack_size - 2u;
    while (*task_stack-- == 0u) {
        free_stk++;
    }
#endif

    *free = free_stk;

    return YUNOS_SUCCESS;
}

kstat_t yunos_task_stack_cur_free(ktask_t *task, size_t *free)
{
    CPSR_ALLOC();
    size_t sp = 0;

    YUNOS_CRITICAL_ENTER();

    if (task == NULL || task == g_active_task[cpu_cur_get()]) {
        task = g_active_task[cpu_cur_get()];
        if (soc_get_cur_sp) {
            sp = soc_get_cur_sp();
        }
    } else {
        sp = (size_t)task->task_stack;
    }

    if (sp == 0) {
        YUNOS_CRITICAL_EXIT();
        k_err_proc(YUNOS_SYS_SP_ERR);
        return YUNOS_SYS_SP_ERR;
    }

    if ((size_t)(task->task_stack_base + task->stack_size) < sp) {
        YUNOS_CRITICAL_EXIT();
        k_err_proc(YUNOS_TASK_STACK_OVF);
        return YUNOS_TASK_STACK_OVF;
    }

    *free = ((size_t)(task->task_stack_base + task->stack_size) - sp) / sizeof(
                cpu_stack_t);

    YUNOS_CRITICAL_EXIT();
    return YUNOS_SUCCESS;
}

kstat_t task_pri_change(ktask_t *task, uint8_t new_pri)
{
    uint8_t  old_pri;
    kmutex_t *mutex_tmp;
    ktask_t  *mutex_task;

    do {
        if (task->prio != new_pri) {
            switch (task->task_state) {
                case K_RDY:
                    ready_list_rm(&g_ready_queue, task);
                    task->prio = new_pri;

                    if (task == g_active_task[cpu_cur_get()]) {
                        ready_list_add_head(&g_ready_queue, task);
                    } else {
                        ready_list_add_tail(&g_ready_queue, task);
                    }

                    task = NULL;
                    break;
                case K_SLEEP:
                case K_SUSPENDED:
                case K_SLEEP_SUSPENDED:
                    /* set new task prio */
                    task->prio = new_pri;
                    task = NULL;
                    break;
                case K_PEND:
                case K_PEND_SUSPENDED:
                    old_pri    = task->prio;
                    task->prio = new_pri;
                    pend_list_reorder(task);

                    if (task->blk_obj->obj_type == YUNOS_MUTEX_OBJ_TYPE) {
                        mutex_tmp  = (kmutex_t *)(task->blk_obj);
                        mutex_task = mutex_tmp->mutex_task;

                        if (mutex_task->prio > task->prio) {
                            /* since the highest prio of the lock wait task
                            became higher, raise the lock get task prio
                            higher */
                            task = mutex_task;
                        } else if (mutex_task->prio == old_pri) {
                            /* find suitable tcb prio */
                            new_pri = mutex_pri_look(mutex_task, 0);

                            if (new_pri != mutex_task->prio) {
                                /* Change prio of lock get task */
                                task = mutex_task;
                            } else {
                                task = NULL;
                            }
                        } else {
                            task = NULL;
                        }
                    } else {
                        task = NULL;
                    }

                    break;
                default:
                    k_err_proc(YUNOS_INV_TASK_STATE);
                    return YUNOS_INV_TASK_STATE;
            }
        } else {
            task = NULL;
        }
    } while (task != NULL);

    return YUNOS_SUCCESS;
}

kstat_t yunos_task_pri_change(ktask_t *task, uint8_t pri, uint8_t *old_pri)
{
    CPSR_ALLOC();

    uint8_t pri_limit;
    kstat_t  error;

    NULL_PARA_CHK(task);
    NULL_PARA_CHK(old_pri);

    /* idle task is not allowed to change prio */
    if (task->prio >= YUNOS_IDLE_PRI) {
        return YUNOS_PRI_CHG_NOT_ALLOWED;
    }

    /* not allowed change to idle prio */
    if (pri >= YUNOS_IDLE_PRI) {
        return YUNOS_PRI_CHG_NOT_ALLOWED;
    }

    /* deleted task is not allowed to change prio */
    if (task->task_state == K_DELETED) {
        return YUNOS_INV_TASK_STATE;
    }

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    /* limit the prio change by mutex at task prio change */
    pri_limit = mutex_pri_limit(task, pri);

    task->b_prio = pri;
    /* new pripority may change here */
    pri      = pri_limit;
    *old_pri = task->prio;

    error = task_pri_change(task, pri);

    if (error != YUNOS_SUCCESS) {
        YUNOS_CRITICAL_EXIT();
        return error;
    }

    TRACE_TASK_PRI_CHANGE(g_active_task[cpu_cur_get()], task, pri);

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_TASK_WAIT_ABORT > 0)
kstat_t yunos_task_wait_abort(ktask_t *task)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(task);

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    switch (task->task_state) {
        case K_RDY:
            break;
        case K_SUSPENDED:
            /* change to ready state */
            task->task_state = K_RDY;
            ready_list_add(&g_ready_queue, task);
            break;
        case K_SLEEP:
        case K_SLEEP_SUSPENDED:
            /* change to ready state */
            tick_list_rm(task);
            ready_list_add(&g_ready_queue, task);
            task->task_state = K_RDY;
            task->blk_state  = BLK_ABORT;
            break;
        case K_PEND_SUSPENDED:
        case K_PEND:
            /* remove task on the tick list because task is woken up */
            tick_list_rm(task);
            /* remove task on the block list because task is woken up */
            klist_rm(&task->task_list);
            /* add to the ready list again */
            ready_list_add(&g_ready_queue, task);
            task->task_state = K_RDY;
            task->blk_state  = BLK_ABORT;

            mutex_task_pri_reset(task);
            task->blk_obj = NULL;

            break;
        default:
            YUNOS_CRITICAL_EXIT();
            return  YUNOS_INV_TASK_STATE;
    }

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_task_abort_hook(task);
#endif

    TRACE_TASK_WAIT_ABORT(g_active_task[cpu_cur_get()], task);

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}
#endif

#if (YUNOS_CONFIG_TASK_DEL > 0)
static void task_mutex_free(ktask_t *task)
{
    kmutex_t *mutex;
    kmutex_t *next_mutex;
    ktask_t  *next_task;
    klist_t *blk_list_head;

    next_mutex = task->mutex_list;

    while ((mutex = next_mutex) != NULL) {
        next_mutex = mutex->mutex_list;

        blk_list_head = &mutex->blk_obj.blk_list;

        if (!is_klist_empty(blk_list_head)) {
            next_task = yunos_list_entry(blk_list_head->next, ktask_t, task_list);

            /* wakeup wait task */
            pend_task_wakeup(next_task);

            /* change mutex get task */
            mutex->mutex_task     = next_task;
            mutex->mutex_list     = next_task->mutex_list;
            next_task->mutex_list = mutex;
        } else {
            /* no wait task */
            mutex->mutex_task = NULL;
        }
    }
}

kstat_t yunos_task_del(ktask_t *task)
{
    CPSR_ALLOC();

    uint8_t cur_cpu_num;

    YUNOS_CRITICAL_ENTER();

    cur_cpu_num = cpu_cur_get();

    INTRPT_NESTED_LEVEL_CHK();

    if (task == NULL) {
        task = g_active_task[cur_cpu_num];
    }

    if (task->prio == YUNOS_IDLE_PRI) {
        return YUNOS_TASK_DEL_NOT_ALLOWED;
    }

    if (task->mm_alloc_flag != K_OBJ_STATIC_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

#if (YUNOS_CONFIG_CPU_NUM > 1)
    if (task->cpu_num != cur_cpu_num) {
        if (task->cur_exc == 1) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_TRY_AGAIN;
        }
    }
#endif

    if (task == g_active_task[cpu_cur_get()]) {
        if (g_sched_lock[cpu_cur_get()] > 0u) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_SCHED_DISABLE;
        }
    }

    /* free all the mutex which task hold */
    task_mutex_free(task);

    switch (task->task_state) {
        case K_RDY:
            ready_list_rm(&g_ready_queue, task);
            task->task_state = K_DELETED;
            break;
        case K_SUSPENDED:
            task->task_state = K_DELETED;
            break;
        case K_SLEEP:
        case K_SLEEP_SUSPENDED:
            tick_list_rm(task);
            task->task_state = K_DELETED;
            break;
        case K_PEND:
        case K_PEND_SUSPENDED:
            tick_list_rm(task);
            klist_rm(&task->task_list);
            task->task_state = K_DELETED;

            mutex_task_pri_reset(task);
            break;
        default:
            YUNOS_CRITICAL_EXIT();
            return YUNOS_INV_TASK_STATE;
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&task->task_stats_item);
#endif

    TRACE_TASK_DEL(g_active_task[cur_cpu_num], task);

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_task_del_hook(task);
#endif

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
kstat_t yunos_task_dyn_del(ktask_t *task)
{
    CPSR_ALLOC();

    kstat_t ret;

    uint8_t cur_cpu_num;

    YUNOS_CRITICAL_ENTER();

    cur_cpu_num = cpu_cur_get();

    INTRPT_NESTED_LEVEL_CHK();

    if (task == NULL) {
        task = g_active_task[cur_cpu_num];
    }

    if (task->prio == YUNOS_IDLE_PRI) {
        YUNOS_CRITICAL_EXIT();

        return YUNOS_TASK_DEL_NOT_ALLOWED;
    }

    if (task->mm_alloc_flag != K_OBJ_DYN_ALLOC) {
        YUNOS_CRITICAL_EXIT();

        return YUNOS_KOBJ_DEL_ERR;
    }

#if (YUNOS_CONFIG_CPU_NUM > 1)
    if (task->cpu_num != cur_cpu_num) {
        if (task->cur_exc == 1) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_TRY_AGAIN;
        }
    }
#endif

    if (task == g_active_task[cpu_cur_get()]) {
        if (g_sched_lock[cpu_cur_get()] > 0u) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_SCHED_DISABLE;
        }
    }

    if (task->task_state == K_DELETED) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_INV_TASK_STATE;
    }

    g_sched_lock[cpu_cur_get()]++;
    ret = yunos_queue_back_send(&g_dyn_queue, task->task_stack_base);
    if (ret != YUNOS_SUCCESS) {
        g_sched_lock[cpu_cur_get()]--;
        YUNOS_CRITICAL_EXIT();
        return ret;
    }

    ret = yunos_queue_back_send(&g_dyn_queue, task);
    if (ret != YUNOS_SUCCESS) {
        g_sched_lock[cpu_cur_get()]--;
        YUNOS_CRITICAL_EXIT();
        return ret;
    }

    g_sched_lock[cpu_cur_get()]--;

    /* free all the mutex which task hold */
    task_mutex_free(task);

    switch (task->task_state) {
        case K_RDY:
            ready_list_rm(&g_ready_queue, task);
            task->task_state = K_DELETED;
            break;
        case K_SUSPENDED:
            task->task_state = K_DELETED;
            break;
        case K_SLEEP:
        case K_SLEEP_SUSPENDED:
            tick_list_rm(task);
            task->task_state = K_DELETED;
            break;
        case K_PEND:
        case K_PEND_SUSPENDED:
            tick_list_rm(task);
            klist_rm(&task->task_list);
            task->task_state = K_DELETED;
            mutex_task_pri_reset(task);
            break;
        case K_SEED:
        default:
            break;
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&task->task_stats_item);
#endif

    TRACE_TASK_DEL(g_active_task[cpu_cur_get()], task);

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_task_del_hook(task);
#endif

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}
#endif

#endif

#if (YUNOS_CONFIG_SCHED_RR > 0)
kstat_t yunos_task_time_slice_set(ktask_t *task, size_t slice)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(task);

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (slice > 0u) {
        /* assign the new time slice */
        task->time_total = slice;
    } else {
        /* assign the default time slice */
        task->time_total = YUNOS_CONFIG_TIME_SLICE_DEFAULT;
    }

    task->time_slice = task->time_total;

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_sched_policy_set(ktask_t *task, uint8_t policy)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(task);

    if ((policy != KSCHED_FIFO) && (policy != KSCHED_RR)) {
        return YUNOS_INV_SCHED_WAY;
    }

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    task->sched_policy = policy;
    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_sched_policy_get(ktask_t *task, uint8_t *policy)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(task);
    NULL_PARA_CHK(policy);

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    *policy = task->sched_policy;
    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}
#endif

#if (YUNOS_CONFIG_TASK_INFO > 0)
kstat_t yunos_task_info_set(ktask_t *task, size_t idx, void *info)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(task);

    if (idx >= YUNOS_CONFIG_TASK_INFO_NUM) {
        return YUNOS_INV_PARAM;
    }

    YUNOS_CPU_INTRPT_DISABLE();
    task->user_info[idx] = info;
    YUNOS_CPU_INTRPT_ENABLE();

    return YUNOS_SUCCESS;
}

kstat_t yunos_task_info_get(ktask_t *task, size_t idx, void **info)
{
    NULL_PARA_CHK(task);
    NULL_PARA_CHK(info);

    if (idx >= YUNOS_CONFIG_TASK_INFO_NUM) {
        return YUNOS_INV_PARAM;
    }

    *info = task->user_info[idx];

    return YUNOS_SUCCESS;
}

void  yunos_task_deathbed(void)
{
#if (YUNOS_CONFIG_TASK_DEL > 0)
    CPSR_ALLOC();

    ktask_t *task;

    YUNOS_CPU_INTRPT_DISABLE();
    task = g_active_task[cpu_cur_get()];
    YUNOS_CPU_INTRPT_ENABLE();

    if (task->mm_alloc_flag == K_OBJ_DYN_ALLOC) {
        /* del my self*/
#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
        yunos_task_dyn_del(NULL);
#endif
    } else {
        yunos_task_del(NULL);
    }
#else

    while (1) {
        yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND * 10);
    }
#endif
}
#endif

