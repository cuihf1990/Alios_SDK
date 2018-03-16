#include <pthread.h>
#include <pthread_mutex.h>

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    kstat_t ret;

    ret = krhino_mutex_dyn_create(mutex, "mutex");

    if (ret == RHINO_SUCCESS) {
        return 0;
    }

    return 1;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    kstat_t ret;

    ret = krhino_mutex_dyn_del(*mutex);

    if (ret == RHINO_SUCCESS) {
        return 0;
    }

    return 1;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    kstat_t ret;
    ret = krhino_mutex_lock(*mutex, RHINO_WAIT_FOREVER);

    if (ret == RHINO_SUCCESS) {
        return 0;
    }

    return 1;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    kstat_t ret;

    ret = krhino_mutex_unlock(*mutex);

    if (ret == RHINO_SUCCESS) {
        return 0;
    }

    return 1;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    kstat_t ret;
    
    ret = krhino_mutex_lock(*mutex, 0);

    if (ret == RHINO_SUCCESS) {
        return 0;
    }

    return 1;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    return 0;
}

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int  pshared)
{
    return 0;
}

int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared)
{
    return 0;
}

