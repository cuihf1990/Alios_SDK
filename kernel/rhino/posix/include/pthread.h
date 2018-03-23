#ifndef PTHREAD_H
#define PTHREAD_H

#include <k_api.h>

typedef ktask_t *pthread_t;

#define PTHREAD_SCOPE_PROCESS      0
#define PTHREAD_SCOPE_SYSTEM       1
#define PTHREAD_INHERIT_SCHED      1
#define PTHREAD_EXPLICIT_SCHED     2
#define PTHREAD_CREATE_DETACHED    0
#define PTHREAD_CREATE_JOINABLE    1

#define SCHED_OTHER                0
#define SCHED_FIFO                 1
#define SCHED_RR                   2

#define DEFAULT_THREAD_STACK_SIZE  2048
#define DEFAULT_THREAD_PRIORITY    30

struct sched_param {
  int sched_priority;           /* Process execution scheduling priority */
};

typedef struct {
  int is_initialized;
  void *stackaddr;
  int stacksize;
  int contentionscope;
  int inheritsched;
  int schedpolicy;
  struct sched_param schedparam;
  size_t guardsize;
  int  detachstate;
  size_t affinitysetsize;
} pthread_attr_t;

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg);
void pthread_exit(void *retval);
int pthread_detach(pthread_t thread);
int pthread_join(pthread_t thread, void **retval);
int  pthread_cancel(pthread_t thread);
void pthread_testcancel(void);
int pthread_setcancelstate(int state, int *oldstate);
int pthread_setcanceltype(int type, int *oldtype);
int  pthread_kill(pthread_t thread, int sig);

#endif

