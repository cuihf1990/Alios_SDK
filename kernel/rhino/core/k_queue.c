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

#if (YUNOS_CONFIG_QUEUE > 0)
YUNOS_INLINE void task_msg_recv(ktask_t *task, void *msg)
{
    task->msg = msg;
    pend_task_wakeup(task);
}

static kstat_t queue_create(kqueue_t *queue, const name_t *name, void **start,
                            size_t msg_num, uint8_t mm_alloc_flag)
{
    NULL_PARA_CHK(queue);
    NULL_PARA_CHK(start);
    NULL_PARA_CHK(name);

    if (msg_num == 0u) {
        return YUNOS_INV_PARAM;
    }

    /* init the queue blocked list */
    klist_init(&queue->blk_obj.blk_list);

    queue->blk_obj.name       = name;
    queue->blk_obj.blk_policy = BLK_POLICY_PRI;
#if (YUNOS_CONFIG_KOBJ_SET > 0)
    queue->blk_obj.handle     = NULL;
#endif

    queue->msg_q.queue_start  = start;

    ringbuf_init(&queue->ringbuf, (void *)start, msg_num * sizeof(void *),
                 RINGBUF_TYPE_FIX, sizeof(void *));

    queue->msg_q.size         = msg_num;
    queue->msg_q.cur_num      = 0u;
    queue->msg_q.peak_num     = 0u;
    queue->mm_alloc_flag      = mm_alloc_flag;

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_insert(&(g_kobj_list.queue_head), &queue->queue_item);
#endif

    queue->blk_obj.obj_type = YUNOS_QUEUE_OBJ_TYPE;

    return YUNOS_SUCCESS;
}

kstat_t yunos_queue_create(kqueue_t *queue, const name_t *name, void **start,
                           size_t msg_num)
{
    return queue_create(queue, name, start, msg_num, K_OBJ_STATIC_ALLOC);
}

kstat_t yunos_queue_del(kqueue_t *queue)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    NULL_PARA_CHK(queue);

    INTRPT_NESTED_LEVEL_CHK();

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (queue->mm_alloc_flag != K_OBJ_STATIC_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    blk_list_head = &queue->blk_obj.blk_list;

    queue->blk_obj.obj_type = YUNOS_OBJ_TYPE_NONE;

    /* all task blocked on this queue is waken up */
    while (!is_klist_empty(blk_list_head)) {
        pend_task_rm(yunos_list_entry(blk_list_head->next, ktask_t, task_list));
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&queue->queue_item);
#endif

    ringbuf_reset(&queue->ringbuf);

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
kstat_t yunos_queue_dyn_create(kqueue_t **queue, const name_t *name,
                               size_t msg_num)
{
    kstat_t   stat;
    kqueue_t *queue_obj;
    void     *msg_start;

    NULL_PARA_CHK(queue);

    queue_obj = yunos_mm_alloc(sizeof(kqueue_t));
    if (queue_obj == NULL) {
        return YUNOS_NO_MEM;
    }

    msg_start = yunos_mm_alloc(msg_num * sizeof(void *));
    if (msg_start == NULL) {
        yunos_mm_free(queue_obj);
        return YUNOS_NO_MEM;
    }

    stat = queue_create(queue_obj, name, (void **)msg_start, msg_num,
                        K_OBJ_DYN_ALLOC);
    if (stat != YUNOS_SUCCESS) {
        yunos_mm_free(msg_start);
        yunos_mm_free(queue_obj);
        return stat;
    }

    *queue = queue_obj;

    return stat;
}

kstat_t yunos_queue_dyn_del(kqueue_t *queue)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    NULL_PARA_CHK(queue);

    INTRPT_NESTED_LEVEL_CHK();

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (queue->mm_alloc_flag != K_OBJ_DYN_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    blk_list_head = &queue->blk_obj.blk_list;

    queue->blk_obj.obj_type = YUNOS_OBJ_TYPE_NONE;

    /* all task blocked on this queue is waken up */
    while (!is_klist_empty(blk_list_head)) {
        pend_task_rm(yunos_list_entry(blk_list_head->next, ktask_t, task_list));
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&queue->queue_item);
#endif

    ringbuf_reset(&queue->ringbuf);

    YUNOS_CRITICAL_EXIT_SCHED();

    yunos_mm_free(queue->msg_q.queue_start);
    yunos_mm_free(queue);

    return YUNOS_SUCCESS;
}
#endif

static kstat_t msg_send(kqueue_t *p_q, void *p_void, uint8_t opt_send_method,
                        uint8_t opt_wake_all)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    NULL_PARA_CHK(p_q);

    /* this is only needed when system zero interrupt feature is enabled */
#if (YUNOS_CONFIG_INTRPT_GUARD > 0)
    soc_intrpt_guard();
#endif

    YUNOS_CRITICAL_ENTER();

    if (p_q->blk_obj.obj_type != YUNOS_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (p_q->msg_q.cur_num >= p_q->msg_q.size) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_QUEUE_FULL;
    }

    blk_list_head = &p_q->blk_obj.blk_list;

    /* queue is not full here, if there is no blocked receive task */
    if (is_klist_empty(blk_list_head)) {
        p_q->msg_q.cur_num++;

        /* update peak_num for debug */
        if (p_q->msg_q.cur_num > p_q->msg_q.peak_num) {
            p_q->msg_q.peak_num = p_q->msg_q.cur_num;
        }

        if (opt_send_method == QMSG_SEND_TO_END) {
            ringbuf_push(&p_q->ringbuf, &p_void, sizeof(void *));
        } else {
            ringbuf_head_push(&p_q->ringbuf, &p_void, sizeof(void *));
        }

        YUNOS_CRITICAL_EXIT();

#if (YUNOS_CONFIG_KOBJ_SET > 0)
        if (p_q->blk_obj.handle != NULL) {
            p_q->blk_obj.handle->notify((blk_obj_t *)p_q, p_q->blk_obj.handle);
        }
#endif
        return YUNOS_SUCCESS;
    }

    /* wake all the task blocked on this queue */
    if (opt_wake_all) {
        while (!is_klist_empty(blk_list_head)) {
            task_msg_recv(yunos_list_entry(blk_list_head->next, ktask_t, task_list),
                          p_void);
        }
    } else {
        task_msg_recv(yunos_list_entry(blk_list_head->next, ktask_t, task_list),
                      p_void);
    }

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

kstat_t yunos_queue_front_send(kqueue_t *queue, void *msg)
{
    return msg_send(queue, msg, QMSG_SEND_TO_FRONT, WAKE_ONE_TASK);
}

kstat_t yunos_queue_back_send(kqueue_t *queue, void *msg)
{
    return msg_send(queue, msg, QMSG_SEND_TO_END, WAKE_ONE_TASK);
}

kstat_t yunos_queue_all_send(kqueue_t *queue, void *msg, uint8_t opt)
{
    return msg_send(queue, msg, opt, WAKE_ALL_TASK);
}

kstat_t yunos_queue_recv(kqueue_t *queue, tick_t ticks, void **msg)
{
    CPSR_ALLOC();

    kstat_t ret;

    NULL_PARA_CHK(queue);
    NULL_PARA_CHK(msg);

    if ((g_intrpt_nested_level > 0u) && (ticks != YUNOS_NO_WAIT)) {
        return YUNOS_NOT_CALLED_BY_INTRPT;
    }

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    /* if queue has msgs, just receive it */
    if (queue->msg_q.cur_num > 0u) {

        ringbuf_pop(&queue->ringbuf, msg, NULL);

        queue->msg_q.cur_num--;

        YUNOS_CRITICAL_EXIT();

        return YUNOS_SUCCESS;
    }

    if (ticks == YUNOS_NO_WAIT) {
        *msg = NULL;
        YUNOS_CRITICAL_EXIT();

        return YUNOS_NO_PEND_WAIT;
    }

    /* if system is locked, block operation is not allowed */
    if (g_sched_lock > 0u) {
        *msg = NULL;
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_DISABLE;
    }

    pend_to_blk_obj((blk_obj_t *)queue, g_active_task, ticks);

    YUNOS_CRITICAL_EXIT_SCHED();

    YUNOS_CPU_INTRPT_DISABLE();

#ifndef YUNOS_CONFIG_PERF_NO_PENDEND_PROC
    ret = pend_state_end_proc(g_active_task);

    switch (ret) {
        case YUNOS_SUCCESS:
            *msg = g_active_task->msg;
            break;
        default:
            *msg = NULL;
            break;
    }

#else
    if (g_active_task->blk_state == BLK_FINISH) {
        *msg = g_active_task->msg;
    } else {
        *msg = NULL;
    }

    ret = YUNOS_SUCCESS;
#endif

    YUNOS_CPU_INTRPT_ENABLE();

    return ret;
}

kstat_t yunos_queue_is_full(kqueue_t *queue)
{
    CPSR_ALLOC();

    kstat_t ret;

    NULL_PARA_CHK(queue);

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (queue->msg_q.cur_num >= queue->msg_q.size) {
        ret = YUNOS_QUEUE_FULL;
    } else {
        ret = YUNOS_QUEUE_NOT_FULL;
    }

    YUNOS_CRITICAL_EXIT();

    return ret;
}

kstat_t yunos_queue_flush(kqueue_t *queue)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(queue);

    INTRPT_NESTED_LEVEL_CHK();

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    queue->msg_q.cur_num = 0u;
    ringbuf_reset(&queue->ringbuf);

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_queue_info_get(kqueue_t *queue, msg_info_t *info)
{
    CPSR_ALLOC();

    klist_t *blk_list_head;

    if (queue == NULL) {
        return YUNOS_NULL_PTR;
    }

    if (info == NULL) {
        return YUNOS_NULL_PTR;
    }

    NULL_PARA_CHK(queue);
    NULL_PARA_CHK(info);

    YUNOS_CPU_INTRPT_DISABLE();

    if (queue->blk_obj.obj_type != YUNOS_QUEUE_OBJ_TYPE) {
        YUNOS_CPU_INTRPT_ENABLE();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    blk_list_head           = &queue->blk_obj.blk_list;

    info->msg_q.peak_num    = queue->msg_q.peak_num;
    info->msg_q.cur_num     = queue->msg_q.cur_num;
    info->msg_q.queue_start = queue->msg_q.queue_start;
    info->msg_q.size        = queue->msg_q.size;
    info->pend_entry        = blk_list_head->next;

    YUNOS_CPU_INTRPT_ENABLE();

    return YUNOS_SUCCESS;
}
#endif

