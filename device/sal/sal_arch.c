/* system includes */
#include <aos/aos.h>
#include "sal_err.h"

#include "internal/sal_arch_internal.h"


static aos_mutex_t sal_arch_mutex;

//#define      NET_TASK_NUME 2
//#define      NET_TASK_STACK_SIZE 1024

//ktask_t       g_net_task[NET_TASK_NUME];
//cpu_stack_t  g_net_task_stack[NET_TASK_NUME][NET_TASK_STACK_SIZE];

/*-----------------------------------------------------------------------------------*/
/*
    err_t sal_sem_new(sal_sem_t *sem, uint8_t count)
    Creates a new semaphore.
*/
err_t sal_sem_new(sal_sem_t *sem, uint8_t count)
{
    err_t ret = ERR_MEM;
    int stat = aos_sem_new(sem,count);

    if (stat == 0) {
        ret = ERR_OK;
    }
    return ret;
}

/*-----------------------------------------------------------------------------------*/
/*
    void sal_sem_free(sal_sem_t *sem)

    Deallocates a semaphore.
*/
void sal_sem_free(sal_sem_t *sem)
{
    if ((sem != NULL)) {
        aos_sem_free(sem);
    }
}


/*-----------------------------------------------------------------------------------*/
/*
    void sal_sem_signal(sal_sem_t *sem)

    Signals a semaphore.
*/
void sal_sem_signal(sal_sem_t *sem)
{
    aos_sem_signal(sem);
}


/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SAL_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that SAL implements a function with a similar name,
  sal_sem_wait(), that uses the sal_arch_sem_wait() function.
*/
uint32_t sal_arch_sem_wait(sal_sem_t *sem, uint32_t timeout)
{
    uint32_t begin_ms, end_ms, elapsed_ms;
    uint32_t ret;

    if (sem == NULL)
        return SAL_ARCH_TIMEOUT;

    begin_ms = sal_now();

    if( timeout != 0UL ) {
        ret = aos_sem_wait(sem,timeout);
        if(ret == 0) {
            end_ms = sal_now();

            elapsed_ms = end_ms - begin_ms;

            ret = elapsed_ms;
        } else {
            ret = SAL_ARCH_TIMEOUT;
        }
    } else {
        while( !(aos_sem_wait(sem, AOS_WAIT_FOREVER) == 0));
        end_ms = sal_now();

        elapsed_ms = end_ms - begin_ms;

        if( elapsed_ms == 0UL ) {
            elapsed_ms = 1UL;
        }

        ret = elapsed_ms;
    }

    return ret;
}

/** Create a new mutex
 * @param mutex pointer to the mutex to create
 * @return a new mutex
 *
 **/
err_t sal_mutex_new(sal_mutex_t *mutex)
{
    err_t ret = ERR_MEM;
    int stat = aos_mutex_new(mutex);

    if (stat == 0) {
        ret = ERR_OK;
    }
    return ret;
}

/** Lock a mutex
 * @param mutex the mutex to lock
 **/
void sal_mutex_lock(sal_mutex_t *mutex)
{
    aos_mutex_lock(mutex,AOS_WAIT_FOREVER);
}

/** Unlock a mutex
 * @param mutex the mutex to unlock */
void sal_mutex_unlock(sal_mutex_t *mutex)
{
    aos_mutex_unlock(mutex);
}


/** Delete a semaphore
 * @param mutex the mutex to delete
 **/
void sal_mutex_free(sal_mutex_t *mutex)
{
    aos_mutex_free(mutex);
}

/*
    uint32_t sal_now(void)

    This optional function returns the current time in milliseconds (don't care  for wraparound,
    this is only used for time diffs).
*/
uint32_t sal_now(void)
{
    return aos_now_ms();
}



#if SAL_LIGHTWEIGHT_PROT
/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sal_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sal_arch_protect() is only required if your port is supporting an operating
  system.
*/
sal_prot_t sal_arch_protect(void)
{
    aos_mutex_lock(&sal_arch_mutex, AOS_WAIT_FOREVER);
    return 0;
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sal_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void sal_arch_unprotect(sal_prot_t pval)
{
    aos_mutex_unlock(&sal_arch_mutex);
}

#endif
/*
 * Prints an assertion messages and aborts execution.
 */
void sal_arch_assert(const char *file, int line)
{
}

/*
    void sal_mutet_init(void)

    Is called to initialize the sal_arch layer.
*/
void sal_mutex_init(void)
{
    aos_mutex_new(&sal_arch_mutex);
}

