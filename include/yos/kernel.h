#ifndef __YOS_KERNEL_H__
#define __YOS_KERNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <csp.h>

typedef csp_sem_t yos_sem_t;
typedef csp_mutex_t yos_mutex_t;

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
int yos_task_new(const char *name, void (*fn)(void *), void *arg, int stacksize);

/**
 * Create a task([thread--Rhino&other-kernel][MainLoop--noRTOS])
 *
 * @param[in] name task name, any string
 * @param[in] fn task function
 * @param[in] arg any pointer, will give to your task-function as argument
 * @param[in] stacksize stacksize in bytes
 * @param[in] prio priority value, smaller the stronger
 *
 * @return: task code
 */
int yos_task_new_ext(const char *name, void (*fn)(void *), void *arg,
                     int stacksize, int prio);

/**
 * exit a task
 * @param[in] code the id which yos_task_new returned
 */
void yos_task_exit(int code);

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
uint32_t yos_mutex_lock(yos_mutex_t mutex);

/**
 * unlock a mutex
 * @param[in] mutex mutex object,,it contains kernel obj pointer which oc_mutex_new alloced
 */
uint32_t yos_mutex_unlock(yos_mutex_t mutex);

/**
 * alloc a semaphore
 * @param[out] sem pointer of semaphore object,semaphore object must be alloced,
 * hdl pointer in yos_sem_t will refer a kernel obj internally
 * @param[in] count initial semaphore counter
 */
int yos_sem_new(yos_sem_t *sem, int32_t count);

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
uint32_t yos_sem_wait(yos_sem_t sem, uint32_t timeout);

/**
 * release a semaphore
 * @param[in] sem semaphore object,,it contains kernel obj pointer which yos_sem_new alloced
 */
void yos_sem_signal(yos_sem_t sem);

/**
 * get current time in nano seconds
 * @return elapsed time in nano seconds from system starting
 */
uint64_t yos_now(void);

/**
 * msleep
 * @param[in] ms sleep time in milliseconds
 */
void yos_msleep(int ms);
/** thread local storage */
typedef unsigned int yos_task_key_t;
int yos_task_key_create(yos_task_key_t *key);
void yos_task_key_delete(yos_task_key_t key);
int yos_task_setspecific(yos_task_key_t key, void *vp);
void *yos_task_getspecific(yos_task_key_t key);

#ifdef __cplusplus
}
#endif

#endif
