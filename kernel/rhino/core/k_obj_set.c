/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>

#if (YUNOS_CONFIG_KOBJ_SET > 0)
void yunos_kobj_set_notify(blk_obj_t *obj, kobj_set_t *handle)
{
    kstat_t stat;
    stat = yunos_queue_back_send(&(handle->queue), obj);

    if (stat != YUNOS_SUCCESS) {
        k_err_proc(stat);
    }
}

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
kstat_t yunos_kobj_set_dyn_create(kobj_set_t **handle, const name_t *name,
                                  size_t msg_num)
{
    kstat_t     stat;
    kobj_set_t *kobj_set;
    void       *start;

    if (handle == NULL) {
        return YUNOS_NULL_PTR;
    }

    kobj_set = yunos_mm_alloc(sizeof(kobj_set_t));

    if (kobj_set == NULL) {
        return YUNOS_NO_MEM;
    }

    start = yunos_mm_alloc(sizeof(void *)*msg_num);

    if (start == NULL) {
        return YUNOS_NO_MEM;
    }

    stat  = yunos_queue_create(&(kobj_set->queue), name, (void **)start, msg_num);

    if (stat != YUNOS_SUCCESS) {
        yunos_mm_free(kobj_set);
        yunos_mm_free(start);
        return stat;
    }

    kobj_set->notify = yunos_kobj_set_notify;

    *handle = kobj_set;

    return YUNOS_SUCCESS;
}

kstat_t yunos_kobj_set_dyn_del(kobj_set_t *handle)
{
    kstat_t stat;

    if (handle == NULL) {
        return YUNOS_NULL_PTR;
    }

    stat = yunos_queue_del(&(handle->queue));
    yunos_mm_free((void *)((handle->queue).msg_q.queue_start));
    yunos_mm_free((void *)handle);

    return stat;
}
#endif

kstat_t yunos_kobj_set_create(kobj_set_t *handle, const name_t *name,
                              void **start, size_t msg_num)
{
    kstat_t     stat;

    if (handle == NULL) {
        return YUNOS_NULL_PTR;
    }

    stat  = yunos_queue_create(&(handle->queue), name, start, msg_num);

    if (stat != YUNOS_SUCCESS) {
        return stat;
    }

    handle->notify = yunos_kobj_set_notify;

    return YUNOS_SUCCESS;
}

kstat_t yunos_kobj_set_del(kobj_set_t *handle)
{
    kstat_t stat;

    if (handle == NULL) {
        return YUNOS_NULL_PTR;
    }

    stat = yunos_queue_del(&(handle->queue));
    return stat;
}

static kstat_t kobj_internal_count_get(blk_obj_t *obj, size_t *count)
{
    /* now we support semaphone,queue,buf_queue */
    switch (obj->obj_type) {
        case YUNOS_SEM_OBJ_TYPE:
            *count = ((ksem_t *)obj)->count;
            break;

        case YUNOS_BUF_QUEUE_OBJ_TYPE:
            *count = ((kbuf_queue_t *)obj)->cur_num;
            break;

        case YUNOS_QUEUE_OBJ_TYPE:
            *count = ((kqueue_t *)obj)->msg_q.cur_num;
            break;

        default:
            *count = 0;
            return YUNOS_KOBJ_TYPE_ERR;
    }

    return YUNOS_SUCCESS;
}

static kstat_t kobj_insert_prepare(kobj_set_t *handle, blk_obj_t *obj)
{
    CPSR_ALLOC();

    klist_t   *blk_list_head;
    msg_info_t info;
    size_t     free;
    size_t     count;
    kstat_t    stat;

    YUNOS_CRITICAL_ENTER();

    blk_list_head = &(obj->blk_list);

    /* obj with blocked task can not treated */
    if (!is_klist_empty(blk_list_head)) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_BLK;
    }

    yunos_queue_info_get(&(handle->queue), &info);
    free = info.msg_q.size - info.msg_q.cur_num;

    stat = kobj_internal_count_get(obj, &count);

    if (stat != YUNOS_SUCCESS) {
        YUNOS_CRITICAL_EXIT();
        return stat;
    }

    if (count > free) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_SET_FULL;
    }

    for (; count > 0; count--) {
        yunos_queue_back_send(&(handle->queue), (void *) obj);
    }

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_kobj_set_insert(blk_obj_t *obj, kobj_set_t *handle)
{
    CPSR_ALLOC();
    kstat_t stat;

    if (obj ==  NULL) {
        return YUNOS_NULL_PTR;
    }

    if (handle ==  NULL) {
        return YUNOS_NULL_PTR;
    }

    stat = kobj_insert_prepare(handle, obj);

    if (stat != YUNOS_SUCCESS) {
        return stat;
    }

    YUNOS_CRITICAL_ENTER();

    obj->handle = handle;

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_kobj_set_rm(blk_obj_t *obj)
{
    CPSR_ALLOC();
    kstat_t stat;
    size_t  count;

    if (obj ==  NULL) {
        return YUNOS_NULL_PTR;
    }

    YUNOS_CRITICAL_ENTER();

    stat = kobj_internal_count_get(obj, &count);

    if (stat != YUNOS_SUCCESS) {
        YUNOS_CRITICAL_EXIT();
        return stat;
    }

    if (count != 0) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_KOBJ_DEL_ERR;
    }

    obj->handle = NULL;

    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

kstat_t yunos_kobj_select(kobj_set_t *handle, blk_obj_t **obj, tick_t ticks)
{
    kstat_t stat;

    if (handle ==  NULL) {
        return YUNOS_NULL_PTR;
    }

    stat = yunos_queue_recv(&(handle->queue), ticks, (void **)obj);
    return stat;
}
#endif  /* YUNOS_CONFIG_KOBJ_SET */

