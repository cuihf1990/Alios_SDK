/**
 * YUNOS YOC sys arch for netproto
*/
#include "arch/sys_arch.h"
#include "arch/cc.h"

typedef void (*lwip_thread_fn)(void *arg);
typedef int err_t;
typedef unsigned char u8_t;
typedef unsigned int u32_t;

/*-----------------------------------------------------------------------------------*/
// Initialize sys arch
void _csp_netp_sys_init(void)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}

/*-----------------------------------------------------------------------------------*/
//  Creates and returns a new semaphore. The "count" argument specifies
//  the initial state of the semaphore. TBD finish and test
err_t _csp_netp_sys_sem_new(sys_sem_t *sem, u8_t count)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}


/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void _csp_netp_sys_sem_free(sys_sem_t *sem)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}


/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void _csp_netp_sys_sem_signal(sys_sem_t *sem)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
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
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t _csp_netp_sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t _csp_netp_sys_mbox_new(sys_mbox_t *mbox, int size)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void _csp_netp_sys_mbox_free(sys_mbox_t *mbox)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void _csp_netp_sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}

err_t _csp_netp_sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t _csp_netp_sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

u32_t _csp_netp_sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/** Create a new mutex
 * @param mutex pointer to the mutex to create
 * @return a new mutex */
err_t _csp_netp_sys_mutex_new(sys_mutex_t *pxMutex)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/** Lock a mutex
 * @param mutex the mutex to lock */
void _csp_netp_sys_mutex_lock(sys_mutex_t *pxMutex)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}

/** Unlock a mutex
 * @param mutex the mutex to unlock */
void _csp_netp_sys_mutex_unlock(sys_mutex_t *pxMutex)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}


/** Delete a semaphore
 * @param mutex the mutex to delete */
void _csp_netp_sys_mutex_free(sys_mutex_t *pxMutex)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}

u32_t _csp_netp_sys_now(void)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
// TBD
/*-----------------------------------------------------------------------------------*/
/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
sys_thread_t _csp_netp_sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/
sys_prot_t _csp_netp_sys_arch_protect(void)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
    return 0;
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void _csp_netp_sys_arch_unprotect(sys_prot_t pval)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}

/*
 * Prints an assertion messages and aborts execution.
 */
void _csp_netp_sys_arch_assert(const char *file, int line)
{
    printf("%s %d is not implemented yet.\n", __FUNCTION__, __LINE__);
}
