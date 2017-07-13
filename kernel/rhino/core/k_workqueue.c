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

#if (YUNOS_CONFIG_WORKQUEUE > 0)
extern kworkqueue_t g_workqueue_default;
extern cpu_stack_t  g_workqueue_stack[YUNOS_CONFIG_WORKQUEUE_STACK_SIZE];

static kstat_t workqueue_is_exist(kworkqueue_t *workqueue)
{
    kworkqueue_t *wq = g_workqueue_head;

    if (g_workqueue_head == NULL) {
        return YUNOS_WORKQUEUE_NOT_EXIST;
    }

    do {
        if (wq == workqueue) {
            return YUNOS_WORKQUEUE_EXIST;
        }

        wq = yunos_list_entry(wq->workqueue_node.next, kworkqueue_t, workqueue_node);
    } while (wq != g_workqueue_head);

    return YUNOS_WORKQUEUE_NOT_EXIST;
}

static void worker_task(void *arg)
{
    kstat_t       ret;
    kwork_t      *work = NULL;
    kworkqueue_t *wq   = (kworkqueue_t *)arg;

    while (1) {
        ret = yunos_sem_take(&(wq->sem), YUNOS_WAIT_FOREVER);
        if (ret != YUNOS_SUCCESS) {
            return;
        }

        while (wq->work_head != NULL) {
            work = wq->work_head;

            yunos_mutex_lock(&(wq->work_mutex), YUNOS_WAIT_FOREVER);

            if (work->work_node.next == &(work->work_node)) {
                wq->work_head = NULL;
            } else {
                wq->work_head = yunos_list_entry(work->work_node.next, kwork_t, work_node);
                klist_rm(&(work->work_node));
            }

            work->running = 2;

            yunos_mutex_unlock(&(wq->work_mutex));

            work->handle(work->arg);
        }
    }
}

kstat_t yunos_workqueue_create(kworkqueue_t *workqueue, const name_t *name,
                               uint8_t pri, cpu_stack_t *stack_buf, size_t stack_size)
{
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

    yunos_mutex_lock(&g_workqueue_mutex, YUNOS_WAIT_FOREVER);

    ret = workqueue_is_exist(workqueue);
    if (ret == YUNOS_WORKQUEUE_EXIST) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return YUNOS_WORKQUEUE_EXIST;
    }

    klist_init(&(workqueue->workqueue_node));
    workqueue->work_head = NULL;
    workqueue->name      = name;

    ret = yunos_mutex_create(&(workqueue->work_mutex), "WORK-MUTEX");
    if (ret != YUNOS_SUCCESS) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return ret;
    }

    ret = yunos_sem_create(&(workqueue->sem), "WORKQUEUE-SEM", 0);
    if (ret != YUNOS_SUCCESS) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return ret;
    }

    ret = yunos_task_create(&(workqueue->worker), name, (void *)workqueue, pri,
                            0, stack_buf, stack_size, worker_task, 0);
    if (ret != YUNOS_SUCCESS) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return ret;
    }

    if (g_workqueue_head == NULL) {
        g_workqueue_head = workqueue;
    } else {
        klist_insert(&(g_workqueue_head->workqueue_node), &(workqueue->workqueue_node));
    }

    ret = yunos_task_resume(&(workqueue->worker));
    if (ret != YUNOS_SUCCESS) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return ret;
    }

    yunos_mutex_unlock(&g_workqueue_mutex);

    TRACE_WORKQUEUE_CREATE(g_active_task, workqueue);

    return YUNOS_SUCCESS;
}

kstat_t yunos_workqueue_del(kworkqueue_t *workqueue)
{
    kstat_t ret;

    NULL_PARA_CHK(workqueue);

    yunos_mutex_lock(&g_workqueue_mutex, YUNOS_WAIT_FOREVER);

    ret = workqueue_is_exist(workqueue);
    if (ret == YUNOS_WORKQUEUE_NOT_EXIST) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return YUNOS_WORKQUEUE_NOT_EXIST;
    }

    TRACE_WORKQUEUE_DEL(g_active_task, workqueue);

    if (workqueue->work_head != NULL) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return YUNOS_WORKQUEUE_BUSY;
    }

    if (workqueue == g_workqueue_head) {
        if (&(workqueue->workqueue_node) == workqueue->workqueue_node.next) {
            g_workqueue_head = NULL;
        } else {
            g_workqueue_head = yunos_list_entry(workqueue->workqueue_node.next,
                                                kworkqueue_t, workqueue_node);
            klist_rm(&(workqueue->workqueue_node));
        }
    } else {
        klist_rm(&(workqueue->workqueue_node));
    }

    workqueue->name = NULL;

    ret = yunos_mutex_del(&(workqueue->work_mutex));
    if (ret != YUNOS_SUCCESS) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return ret;
    }

    ret = yunos_task_del(&(workqueue->worker));
    if (ret != YUNOS_SUCCESS) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return ret;
    }

    ret = yunos_sem_del(&(workqueue->sem));
    if (ret != YUNOS_SUCCESS) {
        yunos_mutex_unlock(&g_workqueue_mutex);
        return ret;
    }

    yunos_mutex_unlock(&g_workqueue_mutex);

    return YUNOS_SUCCESS;
}

static void work_timer_cb(void *timer, void *arg)
{
    kstat_t       ret;
    kwork_t      *work = yunos_list_entry(timer, kwork_t, timer);
    kworkqueue_t *wq   = (kworkqueue_t *)arg;

    yunos_mutex_lock(&(wq->work_mutex), YUNOS_WAIT_FOREVER);

    if (work->running > 0) {
        yunos_mutex_unlock(&(wq->work_mutex));
        /* YUNOS_WORKQUEUE_WORK_EXIST */
        return;
    }

    if (wq->work_head == NULL) {
        wq->work_head = work;
    } else {
        klist_insert(&(wq->work_head->work_node), &(work->work_node));
    }

    work->wq = wq;

    work->running = 1;

    yunos_mutex_unlock(&(wq->work_mutex));

    ret = yunos_sem_give(&(wq->sem));
    if (ret != YUNOS_SUCCESS) {
        return;
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
    work->running = 0;
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

    TRACE_WORK_INIT(g_active_task, work);

    return YUNOS_SUCCESS;
}

kstat_t yunos_work_run(kworkqueue_t *workqueue, kwork_t *work)
{
    kstat_t ret;

    NULL_PARA_CHK(workqueue);
    NULL_PARA_CHK(work);

    if (work->dly == 0) {
        yunos_mutex_lock(&(workqueue->work_mutex), YUNOS_WAIT_FOREVER);

        if (work->running > 0) {
            yunos_mutex_unlock(&(workqueue->work_mutex));
            return YUNOS_WORKQUEUE_WORK_EXIST;
        }

        if (workqueue->work_head == NULL) {
            workqueue->work_head = work;
        } else {
            klist_insert(&(workqueue->work_head->work_node), &(work->work_node));
        }

        work->wq = workqueue;

        work->running = 1;

        yunos_mutex_unlock(&(workqueue->work_mutex));

        ret = yunos_sem_give(&(workqueue->sem));
        if (ret != YUNOS_SUCCESS) {
            return ret;
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
    kworkqueue_t *wq = (kworkqueue_t *)work->wq;

    if (wq == NULL) {
        if (work->dly > 0) {
            yunos_timer_stop(&(work->timer));
            yunos_timer_del(&(work->timer));
        }

        return YUNOS_SUCCESS;
    }

    yunos_mutex_lock(&(wq->work_mutex), YUNOS_WAIT_FOREVER);

    if (work->running == 1) {
        klist_rm(&(work->work_node));
        work->running = 0;
        work->wq      = NULL;
        yunos_mutex_unlock(&(wq->work_mutex));
    } else if (work->running == 2) {
        yunos_mutex_unlock(&(wq->work_mutex));
        return YUNOS_WORKQUEUE_WORK_RUNNING;
    }

    return YUNOS_SUCCESS;
}

void workqueue_init(void)
{
    g_workqueue_head = NULL;

    yunos_mutex_create(&g_workqueue_mutex, "WORKQUEUE-MUTEX");

    yunos_workqueue_create(&g_workqueue_default, "DEFAULT-WORKQUEUE",
                           YUNOS_CONFIG_WORKQUEUE_TASK_PRIO, g_workqueue_stack,
                           YUNOS_CONFIG_WORKQUEUE_STACK_SIZE);
}
#endif

