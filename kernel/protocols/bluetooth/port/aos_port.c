/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include <zephyr.h>
#include <misc/util.h>
#include <misc/dlist.h>

#include <aos/aos.h>

#define BT_DBG_ENABLED IS_ENABLED(CONFIG_BLUETOOTH_DEBUG_CORE)

#include <common/log.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "atomic.h"

#include <k_config.h>
#include <k_default_config.h>
#include <k_types.h>
#include <k_err.h>
#include <k_sys.h>
#include <k_list.h>
#include <k_ringbuf.h>
#include <k_obj.h>
#include <k_queue.h>
#include <k_stats.h>
#include <k_time.h>

extern kstat_t krhino_queue_dyn_create(kqueue_t **queue, const name_t *name, size_t msg_num);

void k_lifo_init(struct k_lifo *lifo)
{
    kstat_t ret;
    const char *name = "ble_lifo";
    size_t msg_num = 20;

    if (NULL == lifo) {
        BT_ERR("lifo is NULL");
        return;
    }

    ret = krhino_queue_dyn_create(&lifo->_queue, name, msg_num);
    if (RHINO_SUCCESS != ret) {
        BT_ERR("lifo %s %p creat fail,%d\n",name, lifo, ret);
    }

    sys_dlist_init(&lifo->poll_events);

#if LIFO_DEBUG
    lifo->total_count = msg_num;
    lifo->count = 0;
#endif
}

void k_lifo_put(struct k_lifo *lifo, void *data)
{
    kstat_t ret;

    if (NULL == lifo) {
        BT_ERR("lifo is NULL");
        return;
    }

    ret = krhino_queue_back_send(lifo->_queue, data);

    if (RHINO_SUCCESS != ret) {
#if LIFO_DEBUG
        BT_ERR("send msg to lifo %p count %d,total_count %d,fail,%d",
                lifo, lifo->count, lifo->total_count, ret);
#else
        BT_ERR("send msg to lifo %p fail,%d",lifo, ret);
#endif
        return;
    }
#if LIFO_DEBUG
    lifo->count++;
#endif
}

void *k_lifo_get(struct k_lifo *lifo, tick_t timeout)
{
    void *msg;
    tick_t t;

    if (NULL == lifo) {
        BT_ERR("lifo is NULL");
        return NULL;
    }

    if (timeout == K_FOREVER) {
        t = RHINO_WAIT_FOREVER;
    } else if (timeout == K_NO_WAIT) {
        t = RHINO_NO_WAIT;
    } else {
        t = krhino_ms_to_ticks(timeout);
    }

    krhino_queue_recv(lifo->_queue, t, &msg);

#if LIFO_DEBUG
    if (msg)
    {
        lifo->count--;
    }
#endif
    return msg;
}

void k_fifo_init(struct k_fifo *fifo)
{
    kstat_t ret;
    const char *name = "ble_fifo";
    size_t msg_len = 20;

    if (NULL == fifo) {
        BT_ERR("fifo is NULL");
        return;
    }
    ret = krhino_queue_dyn_create(&fifo->_queue, name, msg_len);

    if (ret) {
        BT_ERR("fifo %s %p creat fail,%d\n", name, fifo, ret);
        return;
    }

    sys_dlist_init(&fifo->poll_events);

#if FIFO_DEBUG
    fifo->total_count = msg_len;
    fifo->count = 0;
#endif
}

void *k_fifo_get(struct k_fifo *fifo, tick_t timeout)
{
    void *msg = NULL;
    tick_t t;

    if (NULL == fifo) {
        BT_ERR("fifo is NULL");
        return NULL;
    }

    if (timeout == K_FOREVER) {
        t = RHINO_WAIT_FOREVER;
    } else if (timeout == K_NO_WAIT) {
        t = RHINO_NO_WAIT;
    } else {
        t =  krhino_ms_to_ticks(timeout);
    }

    krhino_queue_recv(fifo->_queue, t, &msg);

#if FIFO_DEBUG
    if (msg)
    {
        fifo->count--;
    }
#endif
    return msg;
}

void k_fifo_put(struct k_fifo *fifo, void *msg)
{
    kstat_t ret;

    if (NULL == fifo) {
        BT_ERR("fifo is NULL");
        return;
    }

    ret = krhino_queue_back_send(fifo->_queue, msg);

    if (RHINO_SUCCESS != ret) {
#if FIFO_DEBUG
        BT_ERR("send msg to fifo %p count %d,total_count %d,fail,%d",
                        fifo, fifo->count, fifo->total_count, ret);
#else
        BT_ERR("send msg to fifo %p fail,%d", fifo, ret);
#endif
        return;
    }
#if FIFO_DEBUG
    fifo->count++;
#endif
}

void k_fifo_put_list(struct k_fifo *fifo, void *head, void *tail)
{
    struct net_buf *buf_tail = (struct net_buf *)head;

    for (buf_tail = (struct net_buf *)head; buf_tail; buf_tail = buf_tail->frags) {
        k_fifo_put(fifo, buf_tail);
    }
}

void k_fifo_cancel_wait(struct k_fifo *fifo)
{

}

int k_sem_init(struct k_sem *sem, unsigned int initial_count, unsigned int limit)
{
    kstat_t ret;

    if (NULL == sem) {
        BT_ERR("sem is NULL\n");
        return -EINVAL;
    }

    ret = aos_sem_new(&sem->sem, initial_count);
    sys_dlist_init(&sem->poll_events);
    return ret;
}

int k_sem_take(struct k_sem *sem, uint32_t timeout)
{
    unsigned int t = timeout;

    if (timeout == K_FOREVER) {
        t = AOS_WAIT_FOREVER;
    } else if (timeout == K_NO_WAIT) {
        t = AOS_NO_WAIT;
    }
    return aos_sem_wait(&sem->sem, t);
}

int k_sem_give(struct k_sem *sem)
{
    if (NULL == sem) {
        BT_ERR("sem is NULL\n");
        return -EINVAL;
    }

    aos_sem_signal(&sem->sem);
    return 0;
}

int k_sem_delete(struct k_sem *sem)
{
    if (NULL == sem) {
        BT_ERR("sem is NULL\n");
        return -EINVAL;
    }

    aos_sem_free(&sem->sem);
    return 0;
}

unsigned int k_sem_count_get(struct k_sem *sem)
{
    return aos_sem_count_get(&sem->sem);
}

int64_t k_uptime_get()
{
    return aos_now_ms();
}

typedef void (*task_entry_t)(void *args);
int k_thread_create(struct k_thread *new_thread, k_thread_stack_t *stack,
                    size_t stack_size, k_thread_entry_t entry,
                    void *p1, void *p2, void *p3,
                    int prio, u32_t options, s32_t delay)
{
    int ret;

    ret = aos_task_new_ext(&(new_thread->task), "ble", (task_entry_t)entry, p1, stack_size, prio);
    if (ret) {
        SYS_LOG_ERR("create ble task fail\n");
    }

    return ret;
}

int k_yield(void)
{
    return aos_task_yield();
}

unsigned int irq_lock(void)
{
    return (unsigned int)aos_irq_lock();
}

void irq_unlock(unsigned int key)
{
    aos_irq_unlock(key);
}

void _SysFatalErrorHandler(unsigned int reason,
                           const void *pEsf)
{
};

void k_timer_init(k_timer_t *timer, k_timer_handler_t handle, void *args)
{
    ASSERT(timer, "timer is NULL");
    BT_DBG("timer %p,handle %p,args %p", timer, handle, args);
    timer->handler = handle;
    timer->args = args;
    aos_timer_new(&timer->timer, timer->handler, args, 1000, 0);
}

void k_timer_start(k_timer_t *timer, uint32_t timeout)
{
    ASSERT(timer, "timer is NULL");
    BT_DBG("timer %p,timeout %u", timer, timeout);
    timer->timeout = timeout;
    timer->start_ms = aos_now_ms();
    aos_timer_stop(&timer->timer);
    aos_timer_change(&timer->timer, timeout);
    aos_timer_start(&timer->timer);
}

void k_timer_stop(k_timer_t *timer)
{
    ASSERT(timer, "timer is NULL");
    BT_DBG("timer %p", timer);
    aos_timer_stop(&timer->timer);
}

void k_sleep(s32_t duration)
{
    aos_msleep(duration);
}

unsigned int find_msb_set(u32_t data)
{
    uint32_t count = 0;
    uint32_t mask = 0x80000000;

    if (!data) {
        return 0;
    }
    while((data & mask) == 0) {
        count += 1u;
        mask = mask >> 1u;
    }
    return (32 - count);
}

unsigned int find_lsb_set(u32_t data)
{
    uint32_t count = 0;
    uint32_t mask = 0x00000001;

    if (!data) {
        return 0;
    }
    while((data & mask) == 0) {
        count += 1u;
        mask = mask >> 1u;
    }
    return (count);
}
