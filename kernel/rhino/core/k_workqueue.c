#include <k_api.h>

#if (YUNOS_CONFIG_WORKQUEUE > 0)
extern kworkqueue_t g_workqueue_default;
extern cpu_stack_t  g_workqueue_stack[YUNOS_CONFIG_WORKQUEUE_STACK_SIZE];

static kstat_t workqueue_is_exist(kworkqueue_t *workqueue)
{
    CPSR_ALLOC();

    kworkqueue_t *current_queue = NULL;

    YUNOS_CRITICAL_ENTER();

    yunos_list_for_each_entry(current_queue, &g_workqueue_list_head, workqueue_node) {
        if (current_queue == workqueue) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_WORKQUEUE_EXIST;
        }
    }

    YUNOS_CRITICAL_EXIT();
    return YUNOS_WORKQUEUE_NOT_EXIST;
}

static void worker_task(void *arg)
{
    CPSR_ALLOC();

    kstat_t       ret;
    kwork_t      *work = NULL;
    kworkqueue_t *queue = (kworkqueue_t *)arg;

    while (1) {
        if (is_klist_empty(&(queue->work_list))) {
            ret = yunos_sem_take(&(queue->sem), YUNOS_WAIT_FOREVER);
            if (ret != YUNOS_SUCCESS) {
                k_err_proc(ret);
            }
        }

        YUNOS_CRITICAL_ENTER();
        if (!is_klist_empty(&(queue->work_list))) {
            /* have work to do. */
            work = yunos_list_entry(queue->work_list.next, kwork_t, work_node);
            klist_rm_init(&(work->work_node));
            queue->work_current = work;
            YUNOS_CRITICAL_EXIT();

            /* do work */
            work->handle(work->arg);
            YUNOS_CRITICAL_ENTER();
            /* clean current work */
            queue->work_current = NULL;
        }
        YUNOS_CRITICAL_EXIT();
    }
}

kstat_t yunos_workqueue_create(kworkqueue_t *workqueue, const name_t *name,
                               uint8_t pri, cpu_stack_t *stack_buf, size_t stack_size)
{
    CPSR_ALLOC();

    kstat_t ret;

    NULL_PARA_CHK(workqueue);
    NULL_PARA_CHK(name);
    NULL_PARA_CHK(stack_buf);

    if (pri >= YUNOS_CONFIG_PRI_MAX) {
        return YUNOS_BEYOND_MAX_PRI;
    }

    if (stack_size == 0u) {
        return YUNOS_TASK_INV_STACK_SIZE;
    }

    ret = workqueue_is_exist(workqueue);
    if (ret == YUNOS_WORKQUEUE_EXIST) {
        return YUNOS_WORKQUEUE_EXIST;
    }

    klist_init(&(workqueue->workqueue_node));
    klist_init(&(workqueue->work_list));
    workqueue->work_current = NULL;
    workqueue->name      = name;

    ret = yunos_sem_create(&(workqueue->sem), "WORKQUEUE-SEM", 0);
    if (ret != YUNOS_SUCCESS) {
        return ret;
    }

    ret = yunos_task_create(&(workqueue->worker), name, (void *)workqueue, pri,
                            0, stack_buf, stack_size, worker_task, 0);
    if (ret != YUNOS_SUCCESS) {
        yunos_sem_del(&(workqueue->sem));
        return ret;
    }

    YUNOS_CRITICAL_ENTER();
    klist_insert(&g_workqueue_list_head, &(workqueue->workqueue_node));
    YUNOS_CRITICAL_EXIT();

    ret = yunos_task_resume(&(workqueue->worker));
    if (ret != YUNOS_SUCCESS) {
        return ret;
    }

    TRACE_WORKQUEUE_CREATE(yunos_cur_task_get(), workqueue);

    return YUNOS_SUCCESS;
}

kstat_t yunos_workqueue_del(kworkqueue_t *workqueue)
{
    CPSR_ALLOC();

    kstat_t ret;

    NULL_PARA_CHK(workqueue);

    ret = workqueue_is_exist(workqueue);
    if (ret == YUNOS_WORKQUEUE_NOT_EXIST) {
        return YUNOS_WORKQUEUE_NOT_EXIST;
    }

    YUNOS_CRITICAL_ENTER();

    if (!is_klist_empty(&(workqueue->work_list))) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_WORKQUEUE_BUSY;
    }

    if (workqueue->work_current != NULL) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_WORKQUEUE_BUSY;
    }

    YUNOS_CRITICAL_EXIT();

    ret = yunos_task_del(&(workqueue->worker));
    if (ret != YUNOS_SUCCESS) {
        return ret;
    }

    ret = yunos_sem_del(&(workqueue->sem));
    if (ret != YUNOS_SUCCESS) {
        return ret;
    }

    YUNOS_CRITICAL_ENTER();
    klist_rm_init(&(workqueue->workqueue_node));
    TRACE_WORKQUEUE_DEL(g_active_task[cpu_cur_get()], workqueue);
    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

static void work_timer_cb(void *timer, void *arg)
{
    CPSR_ALLOC();

    kstat_t       ret;
    kwork_t      *work = yunos_list_entry(timer, kwork_t, timer);
    kworkqueue_t *wq   = (kworkqueue_t *)arg;

    YUNOS_CRITICAL_ENTER();
    if (wq->work_current == work) {
        YUNOS_CRITICAL_EXIT();
        return;
    }

    /* NOTE: the work MUST be initialized firstly */
    klist_rm_init(&(work->work_node));
    klist_insert(&(wq->work_list), &(work->work_node));

    work->wq = wq;

    if (wq->work_current == NULL) {
        YUNOS_CRITICAL_EXIT();
        ret = yunos_sem_give(&(wq->sem));
        if (ret != YUNOS_SUCCESS) {
            return;
        }
    } else {
        YUNOS_CRITICAL_EXIT();
    }
}

kstat_t yunos_work_init(kwork_t *work, work_handle_t handle, void *arg,
                        tick_t dly)
{
    kstat_t ret;

    if (work == NULL) {
        return YUNOS_NULL_PTR;
    }

    if (handle == NULL) {
        return YUNOS_NULL_PTR;
    }

    NULL_PARA_CHK(work);
    NULL_PARA_CHK(handle);

    klist_init(&(work->work_node));
    work->handle  = handle;
    work->arg     = arg;
    work->dly     = dly;
    work->wq      = NULL;

    if (dly > 0) {
        ret = yunos_timer_create(&(work->timer), "WORK-TIMER", work_timer_cb,
                                 work->dly, 0, (void *)work, 0);
        if (ret != YUNOS_SUCCESS) {
            return ret;
        }
    }

    TRACE_WORK_INIT(yunos_cur_task_get(), work);

    return YUNOS_SUCCESS;
}

kstat_t yunos_work_run(kworkqueue_t *workqueue, kwork_t *work)
{
    CPSR_ALLOC();

    kstat_t ret;

    NULL_PARA_CHK(workqueue);
    NULL_PARA_CHK(work);

    if (work->dly == 0) {
        YUNOS_CRITICAL_ENTER();
        if (workqueue->work_current == work) {
            YUNOS_CRITICAL_EXIT();
            return YUNOS_WORKQUEUE_WORK_RUNNING;
        }

        /* NOTE: the work MUST be initialized firstly */
        klist_rm_init(&(work->work_node));
        klist_insert(&(workqueue->work_list), &(work->work_node));

        work->wq = workqueue;

        if (workqueue->work_current == NULL) {
            YUNOS_CRITICAL_EXIT();
            ret = yunos_sem_give(&(workqueue->sem));
            if (ret != YUNOS_SUCCESS) {
                return ret;
            }
        } else {
            YUNOS_CRITICAL_EXIT();
        }
    } else {
        yunos_timer_stop(&(work->timer));
        work->timer.timer_cb_arg = (void *)workqueue;

        ret = yunos_timer_start(&(work->timer));
        if (ret != YUNOS_SUCCESS) {
            return ret;
        }
    }

    return YUNOS_SUCCESS;
}

kstat_t yunos_work_sched(kwork_t *work)
{
    return yunos_work_run(&g_workqueue_default, work);
}

kstat_t yunos_work_cancel(kwork_t *work)
{
    CPSR_ALLOC();

    NULL_PARA_CHK(work);

    kworkqueue_t *wq = (kworkqueue_t *)work->wq;

    if (wq == NULL) {
        if (work->dly > 0) {
            yunos_timer_stop(&(work->timer));
            yunos_timer_del(&(work->timer));
        }

        return YUNOS_SUCCESS;
    }

    YUNOS_CRITICAL_ENTER();
    if (wq->work_current == work) {
        YUNOS_CRITICAL_EXIT();
        return YUNOS_WORKQUEUE_WORK_RUNNING;
    }
    klist_rm_init(&(work->work_node));
    work->wq      = NULL;
    YUNOS_CRITICAL_EXIT();

    return YUNOS_SUCCESS;
}

void workqueue_init(void)
{
    klist_init(&g_workqueue_list_head);

    yunos_workqueue_create(&g_workqueue_default, "DEFAULT-WORKQUEUE",
                           YUNOS_CONFIG_WORKQUEUE_TASK_PRIO, g_workqueue_stack,
                           YUNOS_CONFIG_WORKQUEUE_STACK_SIZE);
}
#endif

