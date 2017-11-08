/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>

#if (RHINO_CONFIG_TIMER > 0)
static void timer_list_pri_insert(klist_t *head, ktimer_t *timer)
{
    tick_t    val;
    klist_t  *q;
    klist_t  *start;
    klist_t  *end;
    ktimer_t *task_iter_temp;

    start = end = head;
    val = timer->remain;

    for (q = start->next; q != end; q = q->next) {
        task_iter_temp = krhino_list_entry(q, ktimer_t, timer_list);
        if ((task_iter_temp->match - g_timer_count) > val) {
            break;
        }
    }

    klist_insert(q, &timer->timer_list);
}

static void timer_list_rm(ktimer_t *timer)
{
    klist_t *head;

    head = timer->to_head;
    if (head != NULL) {
        klist_rm(&timer->timer_list);
        timer->to_head = NULL;
    }
}

static kstat_t timer_create(ktimer_t *timer, const name_t *name, timer_cb_t cb,
                            tick_t first, tick_t round, void *arg, uint8_t auto_run,
                            uint8_t mm_alloc_flag)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(timer);
    NULL_PARA_CHK(name);
    NULL_PARA_CHK(cb);

    if (first == 0u) {
        return RHINO_INV_PARAM;
    }

    RHINO_CRITICAL_ENTER();
    INTRPT_NESTED_LEVEL_CHK();
    RHINO_CRITICAL_EXIT();

    timer->name          = name;
    timer->cb            = cb;
    timer->init_count    = first;
    timer->round_ticks   = round;
    timer->remain        = 0u;
    timer->match         = 0u;
    timer->timer_state   = TIMER_DEACTIVE;
    timer->to_head       = NULL;
    timer->mm_alloc_flag = mm_alloc_flag;
    timer->timer_cb_arg  = arg;
    klist_init(&timer->timer_list);

    timer->obj_type = RHINO_TIMER_OBJ_TYPE;

    if (auto_run > 0u) {
        krhino_timer_start(timer);
    }

    TRACE_TIMER_CREATE(krhino_cur_task_get(), timer);

    return RHINO_SUCCESS;
}

kstat_t krhino_timer_create(ktimer_t *timer, const name_t *name, timer_cb_t cb,
                            tick_t first, tick_t round, void *arg, uint8_t auto_run)
{
    return timer_create(timer, name, cb, first, round, arg, auto_run,
                        K_OBJ_STATIC_ALLOC);
}

kstat_t krhino_timer_del(ktimer_t *timer)
{
    NULL_PARA_CHK(timer);

    krhino_mutex_lock(&g_timer_mutex, RHINO_WAIT_FOREVER);

    if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_KOBJ_TYPE_ERR;
    }

    if (timer->timer_state != TIMER_DEACTIVE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_TIMER_STATE_INV;
    }

    if (timer->mm_alloc_flag != K_OBJ_STATIC_ALLOC) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_KOBJ_DEL_ERR;
    }

    timer->obj_type = RHINO_OBJ_TYPE_NONE;

    TRACE_TIMER_DEL(krhino_cur_task_get(), timer);

    krhino_mutex_unlock(&g_timer_mutex);

    return RHINO_SUCCESS;
}

#if (RHINO_CONFIG_KOBJ_DYN_ALLOC > 0)
kstat_t krhino_timer_dyn_create(ktimer_t **timer, const name_t *name,
                                timer_cb_t cb,
                                tick_t first, tick_t round, void *arg, uint8_t auto_run)
{
    kstat_t   ret;
    ktimer_t *timer_obj;

    NULL_PARA_CHK(timer);

    timer_obj = krhino_mm_alloc(sizeof(ktimer_t));
    if (timer_obj == NULL) {
        return RHINO_NO_MEM;
    }

    ret = timer_create(timer_obj, name, cb, first, round, arg, auto_run,
                       K_OBJ_DYN_ALLOC);
    if (ret != RHINO_SUCCESS) {
        krhino_mm_free(timer_obj);

        return ret;
    }

    *timer = timer_obj;

    return ret;
}

kstat_t krhino_timer_dyn_del(ktimer_t *timer)
{
    NULL_PARA_CHK(timer);

    krhino_mutex_lock(&g_timer_mutex, RHINO_WAIT_FOREVER);

    if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_KOBJ_TYPE_ERR;
    }

    if (timer->timer_state != TIMER_DEACTIVE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_TIMER_STATE_INV;
    }

    if (timer->mm_alloc_flag != K_OBJ_DYN_ALLOC) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_KOBJ_DEL_ERR;
    }

    timer->obj_type = RHINO_OBJ_TYPE_NONE;

    krhino_mutex_unlock(&g_timer_mutex);

    krhino_mm_free(timer);

    return RHINO_SUCCESS;
}
#endif

kstat_t krhino_timer_start(ktimer_t *timer)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(timer);

    RHINO_CRITICAL_ENTER();
    INTRPT_NESTED_LEVEL_CHK();
    RHINO_CRITICAL_EXIT();

    krhino_mutex_lock(&g_timer_mutex, RHINO_WAIT_FOREVER);

    if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_KOBJ_TYPE_ERR;
    }

    if (timer->timer_state == TIMER_ACTIVE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_TIMER_STATE_INV;
    }

    timer->match   = g_timer_count + timer->init_count;
    /* sort by remain time */
    timer->remain  = timer->init_count;
    /* used by timer delete */
    timer->to_head = &g_timer_head;

    timer_list_pri_insert(&g_timer_head, timer);
    timer->timer_state = TIMER_ACTIVE;

    krhino_mutex_unlock(&g_timer_mutex);

    return RHINO_SUCCESS;
}

kstat_t krhino_timer_stop(ktimer_t *timer)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(timer);

    RHINO_CRITICAL_ENTER();
    INTRPT_NESTED_LEVEL_CHK();
    RHINO_CRITICAL_EXIT();

    krhino_mutex_lock(&g_timer_mutex, RHINO_WAIT_FOREVER);

    if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_KOBJ_TYPE_ERR;
    }

    if (timer->timer_state == TIMER_DEACTIVE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_TIMER_STATE_INV;
    }

    timer_list_rm(timer);
    timer->timer_state = TIMER_DEACTIVE;

    krhino_mutex_unlock(&g_timer_mutex);

    return RHINO_SUCCESS;
}

kstat_t krhino_timer_change(ktimer_t *timer, tick_t first, tick_t round)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(timer);

    RHINO_CRITICAL_ENTER();
    INTRPT_NESTED_LEVEL_CHK();
    RHINO_CRITICAL_EXIT();

    krhino_mutex_lock(&g_timer_mutex, RHINO_WAIT_FOREVER);

    if (first == 0u) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_INV_PARAM;
    }

    if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_KOBJ_TYPE_ERR;
    }

    if (timer->timer_state != TIMER_DEACTIVE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_TIMER_STATE_INV;
    }

    timer->init_count  = first;
    timer->round_ticks = round;

    krhino_mutex_unlock(&g_timer_mutex);

    return RHINO_SUCCESS;
}

kstat_t krhino_timer_arg_change(ktimer_t *timer, void *arg)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(timer);

    RHINO_CRITICAL_ENTER();
    INTRPT_NESTED_LEVEL_CHK();
    RHINO_CRITICAL_EXIT();

    krhino_mutex_lock(&g_timer_mutex, RHINO_WAIT_FOREVER);

    if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_KOBJ_TYPE_ERR;
    }

    if (timer->timer_state != TIMER_DEACTIVE) {
        krhino_mutex_unlock(&g_timer_mutex);
        return RHINO_TIMER_STATE_INV;
    }

    timer->timer_cb_arg  = arg;

    krhino_mutex_unlock(&g_timer_mutex);

    return RHINO_SUCCESS;
}


static void timer_list_pri_insert(klist_t *head, ktimer_t *timer)
{
    tick_t    val;
    klist_t  *q;
    klist_t  *start;
    klist_t  *end;
    ktimer_t *task_iter_temp;

    start = end = head;
    val = timer->remain;

    for (q = start->next; q != end; q = q->next) {
        task_iter_temp = krhino_list_entry(q, ktimer_t, timer_list);
        if ((task_iter_temp->match - g_timer_count) > val) {
            break;
        }
    }

    klist_insert(q, &timer->timer_list);
}

static void timer_cb_proc(void)
{
    tick_t    val;
    klist_t  *q;
    klist_t  *start;
    klist_t  *end;
    ktimer_t *timer;
    int64_t   delta;

    start = end = &g_timer_head;

    for (q = start->next; q != end; q = q->next) {
        timer = krhino_list_entry(q, ktimer_t, timer_list);
        delta = (int64_t)(timer->match - g_timer_count);
        if (delta <= 0) {
            timer->cb(timer, timer->timer_cb_arg);
        }
    }
}

static void timer_cmd_proc(k_timer_queue_cb *cb)
{
    ktimer_t *timer;
    timer = cb->timer;

    switch (cb->cb_num) {
        case TIMER_CMD_START:
            if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {  
                break;
            }

            if (timer->timer_state == TIMER_ACTIVE) {      
                break;
            }

            timer->match   =  g_timer_count + timer->init_count;
            /* sort by remain time */
            timer->remain  =  timer->init_count;
            /* used by timer delete */
            timer->to_head = &g_timer_head;

            timer_list_pri_insert(&g_timer_head, timer);
            timer->timer_state = TIMER_ACTIVE; 
            break;
        case TIMER_CMD_STOP:
            if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
                break;
            }

            if (timer->timer_state == TIMER_DEACTIVE) {
                break;
            }
            timer_list_rm(timer);
            timer->timer_state = TIMER_DEACTIVE;
            break;
        case TIMER_CMD_CHG:
            if (cb->first == 0u) {
                break;
            }

            if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
                break;
            }

            if (timer->timer_state != TIMER_DEACTIVE) {
                break;
            }

            timer->init_count  = cb->first;
            timer->round_ticks = cb->round;
            break;
        case TIMER_CMD_DEL:
            if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
                break;               
            }

            if (timer->timer_state != TIMER_DEACTIVE) {
                break;                    
            } 

            if (timer->mm_alloc_flag != K_OBJ_STATIC_ALLOC) {
                break;
            }

            timer->obj_type = RHINO_OBJ_TYPE_NONE;
            TRACE_TIMER_DEL(krhino_cur_task_get(), timer);
            break;
        case TIMER_CMD_DYN_DEL:
            RHINO_CRITICAL_ENTER();
            if (timer->obj_type != RHINO_TIMER_OBJ_TYPE) {
                RHINO_CRITICAL_EXIT();
                break;
            }

            if (timer->timer_state != TIMER_DEACTIVE) {
                RHINO_CRITICAL_EXIT();
                break;
            }

            if (timer->mm_alloc_flag != K_OBJ_DYN_ALLOC) {
                RHINO_CRITICAL_EXIT();
                break;
            }

            timer->obj_type = RHINO_OBJ_TYPE_NONE;
            TRACE_TIMER_DEL(krhino_cur_task_get(), timer);
            krhino_mm_free(timer);
            break;
        default:
            k_err_proc(RHINO_SYS_FATAL_ERR);
            break;
    }   

}

static void timer_task(void *pa)
{
    ktimer_t         *timer;
    void             *msg;
    k_timer_queue_cb *cb;
    kstat_t           err;
    sys_time_t        tick_start;
    sys_time_t        tick_end;

    (void)pa;

    while (RHINO_TRUE) {
        tick_start = krhino_sys_tick_get();
        err = krhino_queue_recv(&g_timer_queue, RHINO_CONFIG_NEXT_INTRPT_TICKS, &msg);
        tick_end   = krhino_sys_tick_get();

        if (err = RHINO_BLK_TIMEOUT) {
            g_timer_count += RHINO_CONFIG_NEXT_INTRPT_TICKS;
            continue;
        }
        else {
            g_timer_count += (tick_t)(tick_end - tick_start);
        }

        cb = msg;
        timer_cmd_proc(cb);

        while (!is_klist_empty(&g_timer_head)) {
            timer = krhino_list_entry(&g_timer_head, ktimer_t, timer_list);
            tick_start = krhino_sys_tick_get();
            err = krhino_queue_recv(&g_timer_queue, timer->remain, &msg);
            tick_end   = krhino_sys_tick_get();

            if (err == RHINO_BLK_TIMEOUT) {
                g_timer_count += (tick_t)(tick_end - tick_start);
                timer_cb_proc();
            }
            else if (err == RHINO_NO_PEND_WAIT) {

            }
            else if (err == RHINO_SUCCESS) {
                g_timer_count += (tick_t)(tick_end - tick_start);
                cb = msg;
                timer_cb_proc();
                timer_cmd_proc(cb);
            }
        }
    }
}

void ktimer_init(void)
{
    g_timer_ctrl = RHINO_CONFIG_TIMER_RATE;

    klist_init(&g_timer_head);

    krhino_queue_create(&g_timer_queue, "timer_queue", (void **)&g_timer_msg, 20);

    krhino_mblk_pool_init(&g_timer_pool, "timer_blk_pool", timer_queue_cb, sizeof(k_timer_queue_cb), sizeof(timer_queue_cb));

    krhino_task_create(&g_timer_task, "timer_task", NULL,
                       RHINO_CONFIG_TIMER_TASK_PRI, 0u, g_timer_task_stack,
                       RHINO_CONFIG_TIMER_TASK_STACK_SIZE, timer_task, 1u);

    krhino_sem_create(&g_timer_sem, "timer_sem", 0u);

    krhino_mutex_create(&g_timer_mutex, "g_timer_mutex");
}
#endif /* RHINO_CONFIG_TIMER */

