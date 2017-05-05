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

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
static void sched_disable_measure_start(void)
{
    /* start measure system lock time */
    if (g_sched_lock == 0u) {
        g_sched_disable_time_start = HR_COUNT_GET();
    }
}

static void sched_disable_measure_stop(void)
{
    hr_timer_t diff;

    /* stop measure system lock time, g_sched_lock is always zero here */
    diff = HR_COUNT_GET() - g_sched_disable_time_start;

    if (g_sched_disable_max_time < diff) {
        g_sched_disable_max_time = diff;
    }

    if (g_cur_sched_disable_max_time < diff) {
        g_cur_sched_disable_max_time = diff;
    }
}
#endif

kstat_t yunos_sched_disable(void)
{
    CPSR_ALLOC();

    if (g_intrpt_nested_level > 0u) {
        return YUNOS_NOT_CALLED_BY_INTRPT;
    }

    if (g_sched_lock >= SCHED_MAX_LOCK_COUNT) {
        return YUNOS_SCHED_LOCK_COUNT_OVF;
    }

    YUNOS_CRITICAL_ENTER();

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
    sched_disable_measure_start();
#endif

    g_sched_lock++;

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_sched_enable(void)
{
    CPSR_ALLOC();

    if (g_intrpt_nested_level > 0u) {
        return YUNOS_NOT_CALLED_BY_INTRPT;
    }

    if (g_sched_lock == 0u) {
        return YUNOS_SCHED_ALREADY_ENABLED;
    }

    YUNOS_CRITICAL_ENTER();

    g_sched_lock--;

    if (g_sched_lock > 0u) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_DISABLE;
    }

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
    sched_disable_measure_stop();
#endif

    YUNOS_CRITICAL_EXIT();

    core_sched();

    return YUNOS_SUCCESS;
}

void core_sched(void)
{
    CPSR_ALLOC();

    YUNOS_CPU_INTRPT_DISABLE();

    if (g_intrpt_nested_level > 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    if (g_sched_lock > 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    preferred_ready_task_get(&g_ready_queue);

    /* if preferred task is currently task, then no need to do switch and just return */
    if (g_preferred_ready_task == g_active_task) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    TRACE_TASK_SWITCH(g_active_task, g_preferred_ready_task);

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_task_switch_hook(g_active_task,g_preferred_ready_task);
#endif

    cpu_task_switch();

#if (YUNOS_CONFIG_STACK_OVF_CHECK_HW != 0)
    cpu_task_stack_protect(g_preferred_ready_task->task_stack_base, g_preferred_ready_task->stack_size);
#endif

    YUNOS_CPU_INTRPT_ENABLE();
}

void runqueue_init(runqueue_t *rq)
{
    uint8_t prio;

    rq->highest_pri = YUNOS_CONFIG_PRI_MAX;

    for (prio = 0; prio < YUNOS_CONFIG_PRI_MAX; prio++) {
        rq->cur_list_item[prio] = NULL;
    }
}

YUNOS_INLINE void ready_list_init(runqueue_t *rq, ktask_t *task)
{
    rq->cur_list_item[task->prio] = &task->task_list;
    klist_init(rq->cur_list_item[task->prio]);
    yunos_bitmap_set(rq->task_bit_map, task->prio);

    if ((task->prio) < (rq->highest_pri)) {
        rq->highest_pri = task->prio;
    }
}

YUNOS_INLINE uint8_t is_ready_list_empty(uint8_t prio)
{
    return (g_ready_queue.cur_list_item[prio] == NULL);
}

void ready_list_add_head(runqueue_t *rq, ktask_t *task)
{
    if (is_ready_list_empty(task->prio)) {
        ready_list_init(rq, task);
        return;
    }

    klist_insert(rq->cur_list_item[task->prio], &task->task_list);
    rq->cur_list_item[task->prio] = &task->task_list;
}

void ready_list_add_tail(runqueue_t *rq, ktask_t *task)
{
    if (is_ready_list_empty(task->prio)) {
        ready_list_init(rq, task);
        return;
    }

    klist_insert(rq->cur_list_item[task->prio], &task->task_list);
}

void ready_list_add(runqueue_t *rq, ktask_t *task)
{
    /* if task prio is equal current task prio then add to the end */
    if (task->prio == g_active_task->prio) {
        ready_list_add_tail(rq, task);
    } else {
        ready_list_add_head(rq, task);
    }
}

void ready_list_rm(runqueue_t *rq, ktask_t *task)
{
    int32_t  i;
    uint8_t  pri = task->prio;

    /* if the ready list is not only one, we do not need to update the highest prio */
    if ((rq->cur_list_item[pri]) != (rq->cur_list_item[pri]->next)) {
        if (rq->cur_list_item[pri] == &task->task_list) {
            rq->cur_list_item[pri] = rq->cur_list_item[pri]->next;
        }

        klist_rm(&task->task_list);
        return;
    }

    /* only one item,just set cur item ptr to NULL */
    rq->cur_list_item[pri] = NULL;

    yunos_bitmap_clear(rq->task_bit_map, pri);

    /* if task prio not equal to the highest prio, then we do not need to update the highest prio */
    /* this condition happens when a current high prio task to suspend a low priotity task */
    if (pri != rq->highest_pri) {
        return;
    }

    /* find the highest ready task */
    i = yunos_find_first_bit(rq->task_bit_map);

    /* update the next highest prio task */
    if (i >= 0) {
        rq->highest_pri = i;
    } else {
        k_err_proc(YUNOS_SYS_FATAL_ERR);
    }
}

void ready_list_head_to_tail(runqueue_t *rq, ktask_t *task)
{
    rq->cur_list_item[task->prio] = rq->cur_list_item[task->prio]->next;
}

void preferred_ready_task_get(runqueue_t *rq)
{
    klist_t *node = rq->cur_list_item[rq->highest_pri];
    /* get the highest prio task object */
    g_preferred_ready_task = yunos_list_entry(node, ktask_t, task_list);
}

#if (YUNOS_CONFIG_SCHED_RR > 0)
void time_slice_update(uint8_t task_pri)
{
    ktask_t *task;
    klist_t *head;

    CPSR_ALLOC();

    head = g_ready_queue.cur_list_item[task_pri];

    YUNOS_CRITICAL_ENTER();

    /* if ready list is empty then just return because nothing is to be caculated */
    if (is_ready_list_empty(task_pri)) {
        YUNOS_CRITICAL_EXIT();
        return;
    }

    /* Always look at the first task on the ready list */
    task = yunos_list_entry(head, ktask_t, task_list);

    if (task->sched_policy == KSCHED_FIFO) {
        YUNOS_CRITICAL_EXIT();
        return;
    }

    /* there is only one task on this ready list, so do not need to caculate time slice */
    /* idle task must satisfy this condition */
    if (head->next == head) {
        YUNOS_CRITICAL_EXIT();
        return;
    }

    if (task->time_slice > 0u) {
        task->time_slice--;
    }

    /* if current active task has time_slice, just return */
    if (task->time_slice > 0u) {
        YUNOS_CRITICAL_EXIT();
        return;
    }

    /* move current active task to the end of ready list for the same prio */
    ready_list_head_to_tail(&g_ready_queue, task);

    /* restore the task time slice */
    task->time_slice = task->time_total;

    YUNOS_CRITICAL_EXIT();
}
#endif

