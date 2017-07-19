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

#if (YUNOS_CONFIG_BUF_QUEUE > 0)

static kstat_t buf_queue_create(kbuf_queue_t *queue, const name_t *name,
                                void *buf,
                                size_t size, size_t max_msg, uint8_t mm_alloc_flag)
{
    CPSR_ALLOC();

    size_t  buf_size;

    NULL_PARA_CHK(queue);
    NULL_PARA_CHK(buf);
    NULL_PARA_CHK(name);

    if (max_msg == 0u) {
        return YUNOS_INV_PARAM;
    }

    buf_size = size;

    if (buf_size == 0u) {
        return YUNOS_BUF_QUEUE_SIZE_ZERO;
    }

    /* init the queue blocked list */
    klist_init(&queue->blk_obj.blk_list);

    queue->cur_num           = 0u;
    queue->peak_num          = 0u;
    queue->max_msg_size       = max_msg;
    queue->blk_obj.name       = name;
    queue->blk_obj.blk_policy = BLK_POLICY_PRI;
#if (YUNOS_CONFIG_KOBJ_SET > 0)
    queue->blk_obj.handle = NULL;
#endif
    queue->mm_alloc_flag      = mm_alloc_flag;

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    YUNOS_CRITICAL_ENTER();
    klist_insert(&(g_kobj_list.buf_queue_head), &queue->buf_queue_item);
    YUNOS_CRITICAL_EXIT();
#endif

    queue->blk_obj.obj_type = YUNOS_BUF_QUEUE_OBJ_TYPE;

    ringbuf_init(&(queue->ringbuf), (uint8_t *)buf, size, RINGBUF_TYPE_DYN, 0);
    queue->min_free_buf_size  = queue->ringbuf.freesize;
    TRACE_BUF_QUEUE_CREATE(g_active_task, queue);

    return YUNOS_SUCCESS;
}

kstat_t yunos_buf_queue_create(kbuf_queue_t *queue, const name_t *name,
                               void *buf,
                               size_t size, size_t max_msg)
{
    return buf_queue_create(queue, name, buf, size, max_msg, K_OBJ_STATIC_ALLOC);
}

kstat_t yunos_buf_queue_del(kbuf_queue_t *queue)
{
    CPSR_ALLOC();

    klist_t *head;

    NULL_PARA_CHK(queue);

    INTRPT_NESTED_LEVEL_CHK();

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_BUF_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (queue->mm_alloc_flag != K_OBJ_STATIC_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    head = &queue->blk_obj.blk_list;

    queue->blk_obj.obj_type = YUNOS_OBJ_TYPE_NONE;

    /* all task blocked on this queue is waken up */
    while (!is_klist_empty(head)) {

        pend_task_rm(yunos_list_entry(head->next, ktask_t, task_list));
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&queue->buf_queue_item);
#endif

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
kstat_t yunos_buf_queue_dyn_create(kbuf_queue_t **queue, const name_t *name,
                                   size_t size, size_t max_msg)
{
    kstat_t      stat;
    kbuf_queue_t *queue_obj;

    NULL_PARA_CHK(queue);

    if (size == 0u) {
        return YUNOS_BUF_QUEUE_SIZE_ZERO;
    }

    queue_obj = yunos_mm_alloc(sizeof(kbuf_queue_t));

    if (queue_obj == NULL) {
        return YUNOS_NO_MEM;
    }

    queue_obj->buf = yunos_mm_alloc(size);

    if (queue_obj->buf == NULL) {
        yunos_mm_free(queue_obj);
        return YUNOS_NO_MEM;
    }

    stat = buf_queue_create(queue_obj, name, queue_obj->buf, size, max_msg,
                            K_OBJ_DYN_ALLOC);

    if (stat != YUNOS_SUCCESS) {
        yunos_mm_free(queue_obj->buf);
        yunos_mm_free(queue_obj);
        return stat;
    }

    *queue = queue_obj;

    return YUNOS_SUCCESS;
}

kstat_t yunos_buf_queue_dyn_del(kbuf_queue_t *queue)
{
    CPSR_ALLOC();
    klist_t *head;

    NULL_PARA_CHK(queue);

    INTRPT_NESTED_LEVEL_CHK();

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_BUF_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (queue->mm_alloc_flag != K_OBJ_DYN_ALLOC) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    head = &queue->blk_obj.blk_list;

    queue->blk_obj.obj_type = YUNOS_OBJ_TYPE_NONE;

    while (!is_klist_empty(head)) {
        pend_task_rm(yunos_list_entry(head->next, ktask_t, task_list));
    }

#if (YUNOS_CONFIG_SYSTEM_STATS > 0)
    klist_rm(&queue->buf_queue_item);
#endif

    YUNOS_CRITICAL_EXIT_SCHED();
    yunos_mm_free(queue->buf);
    yunos_mm_free(queue);

    return YUNOS_SUCCESS;
}
#endif

static kstat_t buf_queue_send(kbuf_queue_t *queue, void *msg, size_t msg_size,
                              int front)
{
    CPSR_ALLOC();

    klist_t *head;
    ktask_t *task;
    kstat_t  err;

    /* this is only needed when system zero interrupt feature is enabled */
#if (YUNOS_CONFIG_INTRPT_GUARD > 0)
    soc_intrpt_guard();
#endif

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_BUF_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (msg_size > queue->max_msg_size) {
        TRACE_BUF_QUEUE_MAX(g_active_task, queue, msg, msg_size);
        return YUNOS_BUF_QUEUE_MSG_SIZE_OVERFLOW;
    }

    if (msg_size == 0) {
        return YUNOS_INV_PARAM;
    }

    head = &queue->blk_obj.blk_list;

    /* buf queue is not full here, if there is no blocked receive task */
    if (is_klist_empty(head)) {
        if (front) {
            err = ringbuf_head_push(&(queue->ringbuf), msg, msg_size);
        } else {
            err = ringbuf_push(&(queue->ringbuf), msg, msg_size);
        }

        if (err != YUNOS_SUCCESS) {
            YUNOS_CRITICAL_EXIT();
            if (err ==  YUNOS_RINGBUF_FULL) {
                err = YUNOS_BUF_QUEUE_FULL;
            }
            return err;
        }

        queue->cur_num ++;

        if (queue->peak_num  < queue->cur_num) {
            queue->peak_num  = queue->cur_num;
        }

        if (queue->min_free_buf_size > queue->ringbuf.freesize) {
            queue->min_free_buf_size = queue->ringbuf.freesize;
        }

        TRACE_BUF_QUEUE_POST(g_active_task, queue, msg, msg_size);

        YUNOS_CRITICAL_EXIT();

#if (YUNOS_CONFIG_KOBJ_SET > 0)

        if (queue->blk_obj.handle != NULL) {
            queue->blk_obj.handle->notify((blk_obj_t *)queue, queue->blk_obj.handle);
        }

#endif
        return YUNOS_SUCCESS;
    }

    task = yunos_list_entry(head->next, ktask_t, task_list);
    memcpy(task->msg, msg, msg_size);
    task->bq_msg_size = msg_size;

    pend_task_wakeup(task);

    TRACE_BUF_QUEUE_TASK_WAKE(g_active_task, task, queue);

    YUNOS_CRITICAL_EXIT_SCHED();

    return YUNOS_SUCCESS;
}

kstat_t yunos_buf_queue_send(kbuf_queue_t *queue, void *msg, size_t size)
{
    NULL_PARA_CHK(queue);
    NULL_PARA_CHK(msg);

    return buf_queue_send(queue, msg, size, YUNOS_FALSE);
}

kstat_t yunos_buf_queue_send_front(kbuf_queue_t *queue, void *msg, size_t size)
{
    NULL_PARA_CHK(queue);
    NULL_PARA_CHK(msg);

    return buf_queue_send(queue, msg, size, YUNOS_TRUE);
}

kstat_t yunos_buf_queue_recv(kbuf_queue_t *queue, tick_t ticks, void *msg,
                             size_t *size)
{
    CPSR_ALLOC();

    kstat_t ret;

    if ((g_intrpt_nested_level > 0u) && (ticks != YUNOS_NO_WAIT)) {
        return YUNOS_NOT_CALLED_BY_INTRPT;
    }

    NULL_PARA_CHK(queue);
    NULL_PARA_CHK(msg);
    NULL_PARA_CHK(size);

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_BUF_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    if (!ringbuf_is_empty(&(queue->ringbuf))) {
        ringbuf_pop(&(queue->ringbuf), msg, size);
        queue->cur_num --;
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SUCCESS;
    }

    if (ticks == YUNOS_NO_WAIT) {
        *size = 0u;
        YUNOS_CRITICAL_EXIT();
        return YUNOS_NO_PEND_WAIT;
    }

    if (g_sched_lock > 0u) {
        *size = 0u;
        YUNOS_CRITICAL_EXIT();
        return YUNOS_SCHED_DISABLE;
    }

    g_active_task->msg = msg;
    pend_to_blk_obj((blk_obj_t *)queue, g_active_task, ticks);

    TRACE_BUF_QUEUE_GET_BLK(g_active_task, queue, ticks);

    YUNOS_CRITICAL_EXIT_SCHED();

    YUNOS_CPU_INTRPT_DISABLE();

#ifndef YUNOS_CONFIG_PERF_NO_PENDEND_PROC
    ret = pend_state_end_proc(g_active_task);

    switch (ret) {
        case YUNOS_SUCCESS:
            *size = g_active_task->bq_msg_size;
            break;

        default:
            *size = 0u;
            break;
    }

#else

    if (g_active_task->blk_state == BLK_FINISH) {
        *size = g_active_task->bq_msg_size;
    } else {
        *size = 0u;
    }

    ret = YUNOS_SUCCESS;
#endif

    YUNOS_CPU_INTRPT_ENABLE();

    return ret;
}

kstat_t yunos_buf_queue_flush(kbuf_queue_t *queue)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(queue);
    INTRPT_NESTED_LEVEL_CHK();

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_BUF_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    ringbuf_reset(&(queue->ringbuf));
    queue->cur_num           = 0u;
    queue->peak_num          = 0u;
    queue->min_free_buf_size  = queue->ringbuf.freesize;

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_buf_queue_info_get(kbuf_queue_t *queue, kbuf_queue_info_t *info)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(queue);
    NULL_PARA_CHK(info);

    YUNOS_CRITICAL_ENTER();

    if (queue->blk_obj.obj_type != YUNOS_BUF_QUEUE_OBJ_TYPE) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_TYPE_ERR;
    }

    info->free_buf_size = queue->ringbuf.freesize;
    info->min_free_buf_size = queue->min_free_buf_size;
    info->buf_size = queue->ringbuf.end - queue->ringbuf.buf;
    info->max_msg_size = queue->max_msg_size;
    info->cur_num = queue->cur_num;
    info->peak_num = queue->peak_num;

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}
#endif

