/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <yos/kernel.h>

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC == 0)
#error "YUNOS_CONFIG_KOBJ_DYN_ALLOC must be configured!"
#endif

#define MS2TICK(ms) yunos_ms_to_ticks(ms)

static unsigned int used_bitmap;

extern void hal_reboot(void);
void yos_reboot(void)
{
    hal_reboot();
}

int yos_get_hz(void)
{
    return YUNOS_CONFIG_TICKS_PER_SECOND;
}

const char *yos_version_get(void)
{
    return yunos_version_get();
}

const char *yos_strerror(int errnum)
{
    (void)errnum;
    return NULL;
}

int yos_task_new(const char *name, void (*fn)(void *), void *arg,
                 int stack_size)
{
    ktask_t *task_handle = NULL;

    return (int)yunos_task_dyn_create(&task_handle, name, arg, YOS_DEFAULT_APP_PRI, 0,
                                      stack_size / sizeof(cpu_stack_t), fn, 1u);
}

int yos_task_new_ext(yos_task_t *task, const char *name, void (*fn)(void *), void *arg,
                     int stack_size, int prio)
{
    return (int)yunos_task_dyn_create((ktask_t **)(&(task->hdl)), name, arg, prio, 0,
                                      stack_size / sizeof(cpu_stack_t), fn, 1u);
}

void yos_task_exit(int code)
{
    (void)code;

    yunos_task_dyn_del(NULL);
}

const char *yos_task_name(void)
{
    return yunos_cur_task_get()->task_name;
}

int yos_task_key_create(yos_task_key_t *key)
{
    int i;

    for (i = YUNOS_CONFIG_TASK_INFO_NUM - 1; i >= 0; i--) {
        if (!((1 << i) & used_bitmap)) {
            used_bitmap |= 1 << i;
            *key = i;

            return 0;
        }
    }

    return -1;
}

void yos_task_key_delete(yos_task_key_t key)
{
    if (key >= YUNOS_CONFIG_TASK_INFO_NUM) {
        return;
    }

    used_bitmap &= ~(1 << key);
}

int yos_task_setspecific(yos_task_key_t key, void *vp)
{
    return yunos_task_info_set(yunos_cur_task_get(), key, vp);
}

void *yos_task_getspecific(yos_task_key_t key)
{
    void *vp = NULL;

    yunos_task_info_get(yunos_cur_task_get(), key, &vp);

    return vp;
}

int yos_mutex_new(yos_mutex_t *mutex)
{
    kstat_t   ret;
    kmutex_t *m;

    if (mutex == NULL) {
        return YUNOS_NULL_PTR;
    }

    m = yos_malloc(sizeof(kmutex_t));
    if (m == NULL) {
        return YUNOS_NO_MEM;
    }

    ret = yunos_mutex_create(m, "YOS");
    if (ret != YUNOS_SUCCESS) {
        yos_free(m);
        return ret;
    }

    mutex->hdl = m;

    return ret;
}

void yos_mutex_free(yos_mutex_t *mutex)
{
    if (mutex == NULL) {
        return;
    }

    yunos_mutex_del(mutex->hdl);

    yos_free(mutex->hdl);

    mutex->hdl = NULL;
}

int yos_mutex_lock(yos_mutex_t *mutex, unsigned int timeout)
{
    kstat_t ret;

    if (mutex == NULL) {
        return YUNOS_NULL_PTR;
    }

    if (timeout == YOS_WAIT_FOREVER) {
        ret = yunos_mutex_lock(mutex->hdl, YUNOS_WAIT_FOREVER);
    } else {
        ret = yunos_mutex_lock(mutex->hdl, MS2TICK(timeout));
    }

    /* rhino allow nested */
    if (ret == YUNOS_MUTEX_OWNER_NESTED) {
        ret = YUNOS_SUCCESS;
    }

    return ret;
}

int yos_mutex_unlock(yos_mutex_t *mutex)
{
    kstat_t ret;

    if (mutex == NULL) {
        return YUNOS_NULL_PTR;
    }

    ret = yunos_mutex_unlock(mutex->hdl);
    /* rhino allow nested */
    if (ret == YUNOS_MUTEX_OWNER_NESTED) {
        ret = YUNOS_SUCCESS;
    }

    return ret;
}

int yos_mutex_is_valid(yos_mutex_t *mutex)
{
    if (mutex == NULL) {
        return YUNOS_FALSE;
    }

    return (yunos_mutex_is_valid(mutex->hdl) == YUNOS_SUCCESS) ? YUNOS_TRUE : YUNOS_FALSE;
}

int yos_sem_new(yos_sem_t *sem, int count)
{
    kstat_t ret;
    ksem_t *s;

    if (sem == NULL) {
        return YUNOS_NULL_PTR;
    }

    s = yos_malloc(sizeof(ksem_t));
    if (s == NULL) {
        return YUNOS_NO_MEM;
    }

    ret = yunos_sem_create(s, "YOS", count);
    if (ret != YUNOS_SUCCESS) {
        yos_free(s);
        return ret;
    }

    sem->hdl = s;

    return ret;
}

void yos_sem_free(yos_sem_t *sem)
{
    if (sem == NULL) {
        return;
    }

    yunos_sem_del(sem->hdl);

    yos_free(sem->hdl);

    sem->hdl = NULL;
}

int yos_sem_wait(yos_sem_t *sem, unsigned int timeout)
{
    kstat_t ret;

    if (sem == NULL) {
        return YUNOS_NULL_PTR;
    }

    if (timeout == YOS_WAIT_FOREVER) {
        ret = yunos_sem_take(sem->hdl, YUNOS_WAIT_FOREVER);
    } else {
        ret = yunos_sem_take(sem->hdl, MS2TICK(timeout));
    }

    return ret;
}

void yos_sem_signal(yos_sem_t *sem)
{
    if (sem == NULL) {
        return;
    }

    yunos_sem_give(sem->hdl);
}

int yos_sem_is_valid(yos_sem_t *sem)
{
    if (sem == NULL) {
        return YUNOS_FALSE;
    }

    return (yunos_sem_is_valid(sem->hdl) == YUNOS_SUCCESS) ? YUNOS_TRUE : YUNOS_FALSE;
}

void yos_sem_signal_all(yos_sem_t *sem)
{
    if (sem == NULL) {
        return;
    }

    yunos_sem_give_all(sem->hdl);
}

int yos_queue_new(yos_queue_t *queue, void *buf, unsigned int size, int max_msg)
{
    kstat_t       ret;
    kbuf_queue_t *q;

    if ((queue == NULL) || (buf == NULL)) {
        return YUNOS_NULL_PTR;
    }

    q = yos_malloc(sizeof(kbuf_queue_t));
    if (q == NULL) {
        return YUNOS_NO_MEM;
    }

    ret = yunos_buf_queue_create(q, "YOS", buf, size, max_msg);
    if (ret != YUNOS_SUCCESS) {
        yos_free(q);
        return ret;
    }

    queue->hdl = q;

    return ret;
}

void yos_queue_free(yos_queue_t *queue)
{
    if (queue == NULL) {
        return;
    }

    yunos_buf_queue_del(queue->hdl);

    yos_free(queue->hdl);

    queue->hdl = NULL;
}

int yos_queue_send(yos_queue_t *queue, void *msg, unsigned int size)
{
    if ((queue == NULL) || (msg == NULL)) {
        return YUNOS_NULL_PTR;
    }

    return yunos_buf_queue_send(queue->hdl, msg, size);
}

int yos_queue_recv(yos_queue_t *queue, unsigned int ms, void *msg,
                   unsigned int *size)
{
    if (queue == NULL) {
        return YUNOS_NULL_PTR;
    }

    return yunos_buf_queue_recv(queue->hdl, MS2TICK(ms), msg, size);
}

int yos_queue_is_valid(yos_queue_t *queue)
{
    if (queue == NULL) {
        return YUNOS_FALSE;
    }

    return (yunos_buf_queue_is_valid(queue->hdl) == YUNOS_SUCCESS) ? YUNOS_TRUE : YUNOS_FALSE;
}

void *yos_queue_buf_ptr(yos_queue_t *queue)
{
    if (yos_queue_is_valid(queue) != YUNOS_SUCCESS) {
        return NULL;
    }

    return  ((kbuf_queue_t *)queue->hdl)->buf;
}

int yos_sched_disable()
{
    return (int)yunos_sched_disable();
}

int yos_sched_enable()
{
    return (int)yunos_sched_enable();
}

int yos_timer_new(yos_timer_t *timer, void (*fn)(void *, void *),
                  void *arg, int ms, int repeat)
{
    kstat_t   ret;
    ktimer_t *t;

    if (timer == NULL) {
        return YUNOS_NULL_PTR;
    }

    t = yos_malloc(sizeof(ktimer_t));
    if (t == NULL) {
        return YUNOS_NO_MEM;
    }

    if (repeat == 0) {
        ret = yunos_timer_create(t, "YOS", (timer_cb_t)fn, MS2TICK(ms), 0, arg, 1);
    } else {
        ret = yunos_timer_create(t, "YOS", (timer_cb_t)fn, MS2TICK(ms), MS2TICK(ms),
                                 arg, 1);
    }

    if (ret != YUNOS_SUCCESS) {
        yos_free(t);
        return ret;
    }

    timer->hdl = t;

    return ret;
}

void yos_timer_free(yos_timer_t *timer)
{
    if (timer == NULL) {
        return;
    }

    yunos_timer_del(timer->hdl);

    yos_free(timer->hdl);

    timer->hdl = NULL;
}

int yos_timer_start(yos_timer_t *timer)
{
    if (timer == NULL) {
        return YUNOS_NULL_PTR;
    }

    return yunos_timer_start(timer->hdl);
}

int yos_timer_stop(yos_timer_t *timer)
{
    if (timer == NULL) {
        return YUNOS_NULL_PTR;
    }

    return yunos_timer_stop(timer->hdl);
}

int yos_timer_change(yos_timer_t *timer, int ms)
{
    if (timer == NULL) {
        return YUNOS_NULL_PTR;
    }

    return yunos_timer_change(timer->hdl, MS2TICK(ms), MS2TICK(ms));
}

int yos_workqueue_create(yos_workqueue_t *workqueue, int pri, int stack_size)
{
    kstat_t ret;

    cpu_stack_t  *stk;
    kworkqueue_t *wq;

    if (workqueue == NULL) {
        return YUNOS_NULL_PTR;
    }

    if (stack_size < sizeof(cpu_stack_t)) {
        return YUNOS_TASK_INV_STACK_SIZE;
    }

    stk = yos_malloc(stack_size);
    if (stk == NULL) {
        return YUNOS_NO_MEM;
    }

    wq = yos_malloc(sizeof(kworkqueue_t));
    if (wq == NULL) {
        yos_free(stk);
        return YUNOS_NO_MEM;
    }

    ret = yunos_workqueue_create(wq, "YOS", pri, stk,
                                 stack_size / sizeof(cpu_stack_t));
    if (ret != YUNOS_SUCCESS) {
        yos_free(wq);
        yos_free(stk);
        return ret;
    }

    workqueue->hdl = wq;
    workqueue->stk = stk;

    return ret;
}

void yos_workqueue_del(yos_workqueue_t *workqueue)
{
    if (workqueue == NULL) {
        return;
    }

    yunos_workqueue_del(workqueue->hdl);

    yos_free(workqueue->hdl);
    yos_free(workqueue->stk);

    workqueue->hdl = NULL;
    workqueue->stk = NULL;
}

int yos_work_init(yos_work_t *work, void (*fn)(void *), void *arg, int dly)
{
    kstat_t  ret;
    kwork_t *w;

    if (work == NULL) {
        return YUNOS_NULL_PTR;
    }

    w = yos_malloc(sizeof(kwork_t));
    if (w == NULL) {
        return YUNOS_NO_MEM;
    }

    ret = yunos_work_init(w, fn, arg, MS2TICK(dly));
    if (ret != YUNOS_SUCCESS) {
        yos_free(w);
        return ret;
    }

    work->hdl = w;

    return ret;
}

void yos_work_destroy(yos_work_t *work)
{
    if (work == NULL) {
        return;
    }

    yos_free(work->hdl);
    work->hdl = NULL;
}

int yos_work_run(yos_workqueue_t *workqueue, yos_work_t *work)
{
    if ((workqueue == NULL) || (work == NULL)) {
        return YUNOS_NULL_PTR;
    }

    return yunos_work_run(workqueue->hdl, work->hdl);
}

int yos_work_sched(yos_work_t *work)
{
    if (work == NULL) {
        return YUNOS_NULL_PTR;
    }

    return yunos_work_sched(work->hdl);
}

int yos_work_cancel(yos_work_t *work)
{
    if (work == NULL) {
        return YUNOS_NULL_PTR;
    }

    return yunos_work_cancel(work->hdl);
}

void *yos_zalloc(unsigned int size)
{
    void *tmp = NULL;
    if (size == 0) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        tmp = yunos_mm_alloc(size | YOS_UNSIGNED_INT_MSB);

#ifndef YOS_BINS
        yunos_owner_attach(g_kmm_head, tmp, (size_t)__builtin_return_address(0));
#endif
    } else {
        tmp = yunos_mm_alloc(size);
    }

#else
    tmp = yunos_mm_alloc(size);
#endif

    if (tmp) {
        bzero(tmp, size);
    }

    return tmp;
}

void *yos_malloc(unsigned int size)
{
    void *tmp = NULL;

    if (size == 0) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        tmp = yunos_mm_alloc(size | YOS_UNSIGNED_INT_MSB);

#ifndef YOS_BINS
        yunos_owner_attach(g_kmm_head, tmp, (size_t)__builtin_return_address(0));
#endif
    } else {
        tmp = yunos_mm_alloc(size);
    }

#else
    tmp = yunos_mm_alloc(size);
#endif

    return tmp;
}

void *yos_realloc(void *mem, unsigned int size)
{
    void *tmp = NULL;

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        tmp = yunos_mm_realloc(mem, size | YOS_UNSIGNED_INT_MSB);

#ifndef YOS_BINS
        yunos_owner_attach(g_kmm_head, tmp, (size_t)__builtin_return_address(0));
#endif
    } else {
        tmp = yunos_mm_realloc(mem, size);
    }

#else
    tmp = yunos_mm_realloc(mem, size);
#endif

    return tmp;
}

void yos_alloc_trace(void *addr, size_t allocator)
{
#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    yunos_owner_attach(g_kmm_head, addr, allocator);
#endif
}

void yos_free(void *mem)
{
    if (mem == NULL) {
        return;
    }

    yunos_mm_free(mem);
}

long long yos_now(void)
{
    return yunos_sys_time_get() * 1000 * 1000;
}

long long yos_now_ms(void)
{
    return yunos_sys_time_get();
}

void yos_msleep(int ms)
{
    yunos_task_sleep(MS2TICK(ms));
}
