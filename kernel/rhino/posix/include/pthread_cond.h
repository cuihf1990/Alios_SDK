#ifndef PTHREAD_COND_H
#define PTHREAD_COND_H
#include <k_api.h>
#include <pthread_mutex.h>
#include <time.h>

typedef struct new_cond
{
	kmutex_t *lock;
	int       waiting;
	int       signals;
	ksem_t   *wait_sem;
	ksem_t   *wait_done;
} pthread_cond_t;

typedef struct
{
  int __dummy;
} pthread_condattr_t;

//typedef struct new_cond *pthread_cond_t;

int pthread_condattr_init(pthread_condattr_t *attr);
int pthread_condattr_destroy(pthread_condattr_t *attr);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t        *cond,
                           pthread_mutex_t       *mutex,
                           const struct timespec *abstime);

#endif

