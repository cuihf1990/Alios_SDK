#include <pthread.h>

int pthread_attr_init(pthread_attr_t *attr)
{
    attr->stacksize                 = DEFAULT_STACK_SIZE;
    attr->schedparam.sched_priority = DEFAULT_PRIORITY;
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    memset(attr, 0, sizeof(pthread_attr_t));
    return 0;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
    return 0;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
   *detachstate = 0;
    return 0;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
    attr->schedpolicy = policy;
    return 0;
}


int pthread_attr_getschedpolicy(pthread_attr_t const *attr, int *policy)
{
   *policy = (int)attr->schedpolicy;
    return 0;
}

int pthread_attr_setschedparam(pthread_attr_t *attr,
                                      const struct sched_param *param)
{
    attr->schedparam.sched_priority = param->sched_priority;
    return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr,
                                      struct sched_param *param)

{
    param->sched_priority = attr->priority;
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
    attr->stacksize = stack_size;
    return 0;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
   *stack_size = attr->stacksize;
    return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack_addr)
{
    attr->stackaddr = stack_addr;
    return 0;
}

int pthread_attr_getstackaddr(pthread_attr_t const *attr, void **stack_addr)
{
   *stack_addr = attr->stackaddr;
    return 0;
}

int pthread_attr_setstack(pthread_attr_t *attr,
                                 void *stackaddr, size_t stacksize)
{
    attr->stackaddr = stack_base;
    attr->stacksize = stack_size;
    return 0;
}

int pthread_attr_getstack(const pthread_attr_t *attr,
                                 void **stackaddr, size_t *stacksize)
{
   *stack_base = attr->stackaddr;
   *stack_size = attr->stacksize
    return 0;
}

int pthread_attr_setguardsize(pthread_attr_t *attr, size_t guard_size)
{
    return 0;
}

int pthread_attr_getguardsize(pthread_attr_t const *attr, size_t *guard_size)
{
    return 0;
}

int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
    return 0;
}

int pthread_attr_getscope(pthread_attr_t const *attr)
{
    return 0;
}

