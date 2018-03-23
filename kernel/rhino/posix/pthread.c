#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg)
{
    kstat_t ret = 0;

    if (attr == NULL) {
        ret = krhino_task_dyn_create(thread, "pthread_task", arg, 30, 0, 2048, (task_entry_t)start_routine, 1u);
    }

    return ret;
}

void pthread_exit(void *value)
{
    krhino_task_dyn_del(0);
}

int pthread_detach(pthread_t thread)
{
    return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
    return 0;
}

int pthread_cancel(pthread_t thread)
{
    return 0;
}

void pthread_testcancel(void)
{
    return;
}

int pthread_setcancelstate(int state, int *oldstate)
{
    return 0;
}

int pthread_setcanceltype(int type, int *oldtype)
{
    return 0;
}

int pthread_kill(pthread_t thread, int sig)
{
    krhino_task_dyn_del(thread);
    return 0;
}

int pthread_equal(pthread_t t1, pthread_t t2)
{
    return (int)(t1 == t2);
}


