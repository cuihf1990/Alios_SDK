#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                          void *(*start_routine) (void *), void *arg)
{
    kstat_t ret;

    if (attr == NULL) {
        ret = krhino_task_dyn_create(thread, "pthread_task", arg, 30, 0, 2048, start_routine, 1u);
    }

    return ret;
}

void pthread_exit(void *value)
{
    krhino_task_dyn_del(0);
}

