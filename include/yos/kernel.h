/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

/**
 * @file yos/kernel.h
 * @brief kernel API
 * @version since 5.5.0
 */

#ifndef YOS_KERNEL_H
#define YOS_KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#define YOS_WAIT_FOREVER        0xffffffffu
#define YOS_DEFAULT_APP_PRI    32

typedef struct {
    void *hdl;
} yos_hdl_t;

typedef yos_hdl_t yos_task_t;
typedef yos_hdl_t yos_mutex_t;
typedef yos_hdl_t yos_sem_t;
typedef yos_hdl_t yos_queue_t;
typedef yos_hdl_t yos_timer_t;
typedef yos_hdl_t yos_work_t;

typedef struct {
    void *hdl;
    void *stk;
} yos_workqueue_t;

typedef unsigned int yos_task_key_t;

/**
 * System reboot
 */
void yos_reboot(void);

/**
 * get HZ
 */
int yos_get_hz(void);

/**
 * Get kernel version
 *
 * @return the operation status, YUNOS_SUCCESS is OK, others is error
 */
const char *yos_version_get(void);

/**
 * get error information
 * @param[in] errnum the error number
 *
 * @return: error information
 */
const char *yos_strerror(int errnum);

/**
 * Create a task([thread--Rhino&other-kernel][MainLoop--noRTOS])
 *
 * @param[in] name task name, any string
 * @param[in] fn task function
 * @param[in] arg any pointer, will give to your task-function as argument
 * @param[in] stacksize stacksize in bytes
 *
 * @return: task code
 */
int yos_task_new(const char *name, void (*fn)(void *), void *arg,
                 int stack_size);

/**
 * Create a task([thread--Rhino&other-kernel][MainLoop--noRTOS])
 *
 * @param[in] task handle
 * @param[in] name task name, any string
 * @param[in] fn task function
 * @param[in] arg any pointer, will give to your task-function as argument
 * @param[in] stack_buf stack-buf: if stack_buf==NULL, provided by kernel
 * @param[in] stack_size stacksize in bytes
 * @param[in] prio priority value, smaller the stronger
 *
 * @return: task code
 */
int yos_task_new_ext(yos_task_t *task, const char *name, void (*fn)(void *), void *arg,
                     int stack_size, int prio);


/**
 * exit a task
 * @param[in] code the id which yos_task_new returned
 */
void yos_task_exit(int code);

/**
 */
const char *yos_task_name(void);

/** thread local storage
 */
int yos_task_key_create(yos_task_key_t *key);

/**
 */
void yos_task_key_delete(yos_task_key_t key);

/**
 */
int yos_task_setspecific(yos_task_key_t key, void *vp);

/**
 */
void *yos_task_getspecific(yos_task_key_t key);

/**
 * alloc a mutex
 * @param[in] mutex pointer of mutex object,mutex object must be alloced,
 * hdl pointer in yos_mutex_t will refer a kernel obj internally
 */
int yos_mutex_new(yos_mutex_t *mutex);

/**
 * free a mutex
 * @param[in] mutex mutex object,mem refered by hdl pointer in yos_mutex_t will
 * be freed internally
 */
void yos_mutex_free(yos_mutex_t *mutex);

/**
 * lock a mutex
 * @param[in] mutex mutex object,it contains kernel obj pointer which yos_mutex_new alloced
 */
int yos_mutex_lock(yos_mutex_t *mutex, unsigned int timeout);

/**
 * unlock a mutex
 * @param[in] mutex mutex object,,it contains kernel obj pointer which oc_mutex_new alloced
 */
int yos_mutex_unlock(yos_mutex_t *mutex);

/**
 * This function will check if mutex is valid
 * @param[in]   mutex    pointer to the mutex
 * @return  the check status, YUNOS_TRUE is OK, YUNOS_FALSE indicates invalid
 */
int yos_mutex_is_valid(yos_mutex_t *mutex);
/**
 * alloc a semaphore
 * @param[out] sem pointer of semaphore object,semaphore object must be alloced,
 * hdl pointer in yos_sem_t will refer a kernel obj internally
 * @param[in] count initial semaphore counter
 */
int yos_sem_new(yos_sem_t *sem, int count);

/**
 * destroy a semaphore
 * @param[in] sem pointer of semaphore object,mem refered by hdl pointer in yos_sem_t will be freed internally
 */
void yos_sem_free(yos_sem_t *sem);

/**
 * acquire a semaphore
 * @param[in] sem semaphore object,,it contains kernel obj pointer which yos_sem_new alloced
 * @param[in] timeout waiting until timeout in milliseconds
 */
int yos_sem_wait(yos_sem_t *sem, unsigned int timeout);

/**
 * release a semaphore
 * @param[in] sem semaphore object,,it contains kernel obj pointer which yos_sem_new alloced
 */
void yos_sem_signal(yos_sem_t *sem);

/**
 * This function will check if semaphore is valid
 * @param[in]   sem    pointer to the semaphore
 * @return  the check status, YUNOS_TRUE is OK, YUNOS_FALSE indicates invalid
 */
int yos_sem_is_vaid(yos_sem_t *sem);

/**
 * release all semaphore
 * @param[in] sem semaphore object,,it contains kernel obj pointer which yos_sem_new alloced
 */
void yos_sem_signal_all(yos_sem_t *sem);


/**
 * This function will create a queue
 * @param[in]  queue    pointer to the queue(the space is provided by user)
 * @param[in]  buf      buf of the queue(provided by user)
 * @param[in]  size     the bytes of the buf
 * @param[in]  max_msg  the max size of the msg
 * @return  the operation status, 0 is OK, others is error
 */
int yos_queue_new(yos_queue_t *queue, void *buf, unsigned int size,
                  int max_msg);

/**
 * This function will delete a queue
 * @param[in]  queue  pointer to the queue
 * @return  the operation status, YUNOS_SUCCESS is OK, others is error
 */
void yos_queue_free(yos_queue_t *queue);

/**
 * This function will send a msg to the front of a queue
 * @param[in]  queue  pointer to the queue
 * @param[in]  msg    msg to send
 * @param[in]  size   size of the msg
 * @return  the operation status, 0 is OK, others is error
 */
int yos_queue_send(yos_queue_t *queue, void *msg, unsigned int size);

/**
 * This function will receive msg from a queue
 * @param[in]   queue  pointer to the queue
 * @param[in]   ms     ms to wait before receive
 * @param[out]  msg    buf to save msg
 * @param[out]  size   size of the msg
 * @return  the operation status, 0 is OK, others is error
 */
int yos_queue_recv(yos_queue_t *queue, unsigned int ms, void *msg,
                   unsigned int *size);


/**
 * This function will check if queue is valid
 * @param[in]   queue    pointer to the queue
 * @return  the check status, YUNOS_TRUE is OK, YUNOS_FALSE indicates invalid
 */
int yos_queue_is_valid(yos_queue_t *queue);

/**
 * This function will return buf ptr if queue is inited
 * @param[in]   queue    pointer to the queue
 * @return  the check status, NULL is error
 */
void* yos_queue_buf_ptr(yos_queue_t *queue);

/**
 * This function will disable kernel sched
 * @return  the operation status, 0 is OK, others is error
 */
int yos_sched_disable(void);

/**
 * This function will enable kernel sched
 * @return  the operation status, 0 is OK, others is error
 */
int yos_sched_enable(void);

/**
 * This function will create a timer
 * @param[in]  timer   pointer to the timer
 * @param[in]  fn      callbak of the timer
 * @param[in]  arg     the argument of the callback
 * @param[in]  ms      ms of the normal timer triger
 * @param[in]  repeat  repeat or not when the timer is created
 * @return  the operation status, 0 is OK, others is error
 */
int yos_timer_new(yos_timer_t *timer, void (*fn)(void *, void *),
                  void *arg, int ms, int repeat);

/**
 * This function will delete a timer
 * @param[in]  timer  pointer to a timer
 * @return  the operation status, 0 is OK, others is error
 */
void yos_timer_free(yos_timer_t *timer);

/**
 * This function will start a timer
 * @param[in]  timer  pointer to the timer
 * @return  the operation status, 0 is OK, others is error
 */
int yos_timer_start(yos_timer_t *timer);

/**
 * This function will stop a timer
 * @param[in]  timer  pointer to the timer
 * @return  the operation status, 0 is OK, others is error
 */
int yos_timer_stop(yos_timer_t *timer);

/**
 * This function will change attributes of a timer
 * @param[in]  timer  pointer to the timer
 * @param[in]  ms     ms of the timer triger
 * @return  the operation status, 0 is OK, others is error
 */
int yos_timer_change(yos_timer_t *timer, int ms);

/**
 * This function will creat a workqueue
 * @param[in]  workqueue   the workqueue to be created
 * @param[in]  pri         the priority of the worker
 * @param[in]  stack_size  the size of the worker-stack
 * @return  the operation status, 0 is OK, others is error
 */
int yos_workqueue_create(yos_workqueue_t *workqueue, int pri, int stack_size);

/**
 * This function will delete a workqueue
 * @param[in]  workqueue  the workqueue to be deleted
 * @return  the operation status, 0 is OK, others is error
 */
void yos_workqueue_del(yos_workqueue_t *workqueue);

/**
 * This function will initialize a work
 * @param[in]  work  the work to be initialized
 * @param[in]  fn    the call back function to run
 * @param[in]  arg   the paraments of the function
 * @param[in]  dly   ms to delay before run
 * @return  the operation status, 0 is OK, others is error
 */
int yos_work_init(yos_work_t *work, void (*fn)(void *), void *arg, int dly);

/**
 * This function will destroy a work
 * @param[in]  work  the work to be destroied
 * @return None
 */
void yos_work_destroy(yos_work_t *work);

/**
 * This function will run a work on a workqueue
 * @param[in]  workqueue  the workqueue to run work
 * @param[in]  work       the work to run
 * @return  the operation status, 0 is OK, others is error
 */
int yos_work_run(yos_workqueue_t *workqueue, yos_work_t *work);

/**
 * This function will run a work on the default workqueue
 * @param[in]  work  the work to run
 * @return  the operation status, 0 is OK, others is error
 */
int yos_work_sched(yos_work_t *work);

/**
 * This function will cancel a work on the default workqueue
 * @param[in]  work  the work to cancel
 * @return  the operation status, 0 is OK, others is error
 */
int yos_work_cancel(yos_work_t *work);

/**
 * realloc memory
 * @param[in] mem, current memory address point
 * @param[in] size  new size of the mem to remalloc
 * @return  the operation status, NULL is error, others is memory address
 */
void *yos_realloc(void *mem, unsigned int size);

/**
 * alloc memory
 * @param[in] size size of the mem to malloc
 * @return  the operation status, NULL is error, others is memory address
 */
void *yos_malloc(unsigned int size);

/**
 * alloc memory and clear to zero
 * @param[in] size size of the mem to malloc
 * @return  the operation status, NULL is error, others is memory address
 */
void *yos_zalloc(unsigned int size);

/**
 * free memory
 * @param[in]  ptr address point of the mem
 */
void yos_free(void *mem);


/**
 * get current time in nano seconds
 * @return elapsed time in nano seconds from system starting
 */
long long yos_now(void);

/**
 * get current time in mini seconds
 * @return elapsed time in mini seconds from system starting
 */
long long yos_now_ms(void);

/**
 * msleep
 * @param[in] ms sleep time in milliseconds
 */
void yos_msleep(int ms);

#ifdef __cplusplus
}
#endif

#endif

