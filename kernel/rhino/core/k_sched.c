/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
static void sched_disable_measure_start(void)
{
    /* start measure system lock time */
    if (g_sched_lock[cpu_cur_get()] == 0u) {
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

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (g_sched_lock[cpu_cur_get()] >= SCHED_MAX_LOCK_COUNT) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_LOCK_COUNT_OVF;
    }

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
    sched_disable_measure_start();
#endif

    g_sched_lock[cpu_cur_get()]++;

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_sched_enable(void)
{
    CPSR_ALLOC();

    YUNOS_CRITICAL_ENTER();

    INTRPT_NESTED_LEVEL_CHK();

    if (g_sched_lock[cpu_cur_get()] == 0u) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_ALREADY_ENABLED;
    }

    g_sched_lock[cpu_cur_get()]--;

    if (g_sched_lock[cpu_cur_get()] > 0u) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_DISABLE;
    }

#if (YUNOS_CONFIG_DISABLE_SCHED_STATS > 0)
    sched_disable_measure_stop();
#endif

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_CPU_NUM > 1)
void core_sched(void)
{
    uint8_t cur_cpu_num;

    cur_cpu_num = cpu_cur_get();

    if (g_intrpt_nested_level[cur_cpu_num] > 0u) {
        return;
    }

    if (g_sched_lock[cur_cpu_num] > 0u) {
        return;
    }

    preferred_cpu_ready_task_get(&g_ready_queue, cur_cpu_num);

    /* if preferred task is currently task, then no need to do switch and just return */
    if (g_preferred_ready_task[cur_cpu_num] == g_active_task[cur_cpu_num]) {
        return;
    }

    TRACE_TASK_SWITCH(g_active_task[cur_cpu_num], g_preferred_ready_task[cur_cpu_num]);

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_task_switch_hook(g_active_task[cur_cpu_num], g_preferred_ready_task[cur_cpu_num]);
#endif

    g_active_task[cur_cpu_num]->cur_exc = 0;

    cpu_task_switch();

#if (YUNOS_CONFIG_STACK_OVF_CHECK_HW != 0)
    cpu_task_stack_protect(g_preferred_ready_task->task_stack_base,
                           g_preferred_ready_task->stack_size);
#endif

}
#else
void core_sched(void)
{
    CPSR_ALLOC();
    uint8_t cur_cpu_num;

    YUNOS_CPU_INTRPT_DISABLE();

    cur_cpu_num = cpu_cur_get();

    if (g_intrpt_nested_level[cur_cpu_num] > 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    if (g_sched_lock[cur_cpu_num] > 0u) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    preferred_cpu_ready_task_get(&g_ready_queue, cur_cpu_num);

    /* if preferred task is currently task, then no need to do switch and just return */
    if (g_preferred_ready_task[cur_cpu_num] == g_active_task[cur_cpu_num]) {
        YUNOS_CPU_INTRPT_ENABLE();
        return;
    }

    TRACE_TASK_SWITCH(g_active_task[cur_cpu_num], g_preferred_ready_task[cur_cpu_num]);

#if (YUNOS_CONFIG_USER_HOOK > 0)
    yunos_task_switch_hook(g_active_task[cur_cpu_num], g_preferred_ready_task[cur_cpu_num]);
#endif

    cpu_task_switch();

#if (YUNOS_CONFIG_STACK_OVF_CHECK_HW != 0)
    cpu_task_stack_protect(g_preferred_ready_task->task_stack_base,
                           g_preferred_ready_task->stack_size);
#endif

    YUNOS_CPU_INTRPT_ENABLE();
}
#endif

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

YUNOS_INLINE void _ready_list_add_tail(runqueue_t *rq, ktask_t *task)
{
    if (is_ready_list_empty(task->prio)) {
        ready_list_init(rq, task);
        return;
    }

    klist_insert(rq->cur_list_item[task->prio], &task->task_list);
}

YUNOS_INLINE void _ready_list_add_head(runqueue_t *rq, ktask_t *task)
{
    if (is_ready_list_empty(task->prio)) {
        ready_list_init(rq, task);
        return;
    }

    klist_insert(rq->cur_list_item[task->prio], &task->task_list);
    rq->cur_list_item[task->prio] = &task->task_list;
}

#if (YUNOS_CONFIG_CPU_NUM > 1)
static void task_sched_to_cpu(runqueue_t *rq, ktask_t *task, uint8_t cur_cpu_num)
{
    (void)rq;
    uint8_t i;

    if (g_sys_stat == YUNOS_RUNNING) {
        if (task->cpu_binded == 1) {
            if (task->cpu_num != cur_cpu_num) {
                if (task->prio <= g_active_task[task->cpu_num]->prio) {
                    cpu_signal(task->cpu_num);
                }
            }
        } else {
            for (i = 0; i < YUNOS_CONFIG_CPU_NUM; i++) {
                if (g_active_task[i]->prio == YUNOS_IDLE_PRI) {
                    if (i != cur_cpu_num) {
                        cpu_signal(i);
                    }

                    return;
                }
            }

            for (i = 0; i < YUNOS_CONFIG_CPU_NUM; i++) {
                if (task->prio <= g_active_task[i]->prio) {
                    if (i != cur_cpu_num) {
                        cpu_signal(i);
                    }
                    return;
                }
            }
        }
    }
}

void ready_list_add_head(runqueue_t *rq, ktask_t *task)
{
    _ready_list_add_head(rq, task);
    task_sched_to_cpu(rq, task, cpu_cur_get());
}

void ready_list_add_tail(runqueue_t *rq, ktask_t *task)
{
    _ready_list_add_tail(rq, task);
    task_sched_to_cpu(rq, task, cpu_cur_get());
}

#else
void ready_list_add_head(runqueue_t *rq, ktask_t *task)
{
    _ready_list_add_head(rq, task);
}

void ready_list_add_tail(runqueue_t *rq, ktask_t *task)
{
    _ready_list_add_tail(rq, task);
}
#endif

void ready_list_add(runqueue_t *rq, ktask_t *task)
{
    /* if task prio is equal current task prio then add to the end */
    if (task->prio == g_active_task[cpu_cur_get()]->prio) {
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

#if (YUNOS_CONFIG_CPU_NUM > 1)
void preferred_cpu_ready_task_get(runqueue_t *rq, uint8_t cpu_num)
{
    klist_t *iter;
    ktask_t *task;
    uint32_t task_bit_map[NUM_WORDS];
    klist_t *node;
    uint8_t flag;
    uint8_t  highest_pri = rq->highest_pri;

    node = rq->cur_list_item[highest_pri];
    iter = node;
    memcpy(task_bit_map, rq->task_bit_map, NUM_WORDS * sizeof(uint32_t));

    while (1) {

        task = yunos_list_entry(iter, ktask_t, task_list);

        if (g_active_task[cpu_num] == task) {
            break;
        }

        flag = ((task->cur_exc == 0) && (task->cpu_binded == 0))
               || ((task->cur_exc == 0) && (task->cpu_binded == 1) && (task->cpu_num == cpu_num));

        if (flag > 0) {
            task->cpu_num = cpu_num;
            task->cur_exc = 1;
            g_preferred_ready_task[cpu_num] = task;
            break;
        }

        if (iter->next == rq->cur_list_item[highest_pri]) {
            task_bit_map[highest_pri / 32u] &= ~(1u << (31u - (highest_pri % 32u)));

            highest_pri = yunos_find_first_bit(task_bit_map);
            iter = rq->cur_list_item[highest_pri];
        } else {
            iter = iter->next;
        }
    }
}
#else
void preferred_cpu_ready_task_get(runqueue_t *rq, uint8_t cpu_num)
{
    klist_t *node = rq->cur_list_item[rq->highest_pri];
    /* get the highest prio task object */
    g_preferred_ready_task[cpu_num] = yunos_list_entry(node, ktask_t, task_list);
}
#endif

#if (YUNOS_CONFIG_SCHED_RR > 0)

#if (YUNOS_CONFIG_CPU_NUM > 1)

static void _time_slice_update(ktask_t *task, uint8_t i)
{
    klist_t *head;

    head = g_ready_queue.cur_list_item[task->prio];

    /* if ready list is empty then just return because nothing is to be caculated */
    if (is_ready_list_empty(task->prio)) {
        return;
    }

    if (task->sched_policy == KSCHED_FIFO) {
        return;
    }

    /* there is only one task on this ready list, so do not need to caculate time slice */
    /* idle task must satisfy this condition */
    if (head->next == head) {
        return;
    }

    if (task->time_slice > 0u) {
        task->time_slice--;
    }

    /* if current active task has time_slice, just return */
    if (task->time_slice > 0u) {
        return;
    }

    /* move current active task to the end of ready list for the same prio */
    ready_list_head_to_tail(&g_ready_queue, task);

    /* restore the task time slice */
    task->time_slice = task->time_total;

    if (i != cpu_cur_get()) {
        cpu_signal(i);
    }

}

void time_slice_update(uint8_t task_pri)
{
    CPSR_ALLOC();
    uint8_t i;

    (void)task_pri;

    YUNOS_CRITICAL_ENTER();

    for (i = 0; i < YUNOS_CONFIG_CPU_NUM; i++) {
        _time_slice_update(g_active_task[i], i);
    }

    YUNOS_CRITICAL_EXIT();
}


#else
void time_slice_update(uint8_t task_pri)
{
    ktask_t *task;
    klist_t *head;

    CPSR_ALLOC();

    YUNOS_CRITICAL_ENTER();

    head = g_ready_queue.cur_list_item[task_pri];

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

#endif

