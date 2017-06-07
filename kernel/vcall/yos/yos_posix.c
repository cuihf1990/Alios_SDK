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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#include <yos/kernel.h>

#define MS2TICK(ms) ((ms * YUNOS_CONFIG_TICKS_PER_SECOND + 999) / 1000)

void yos_reboot(void)
{

}

const char *yos_version_get(void)
{
    return NULL;
}

const char *yos_strerror(int errnum)
{
    return NULL;
}

int yos_task_new(const char *name, void (*fn)(void *), void *arg,
                 int stack_size)
{
    pthread_t tid;

    return pthread_create(&tid, NULL, (void *(*)(void *))fn, arg);
}

int yos_task_new_ext(const char *name, void (*fn)(void *), void *arg,
                     int stack_size, int prio)
{
    pthread_t tid;

    return pthread_create(&tid, NULL, (void *(*)(void *))fn, arg);
}

void yos_task_exit(int code)
{
    pthread_exit(NULL);
}

int yos_task_key_create(yos_task_key_t *key)
{
    return -1;
}

void yos_task_key_delete(yos_task_key_t key)
{
    return -1;
}

int yos_task_setspecific(yos_task_key_t key, void *vp)
{
    return -1;
}

void *yos_task_getspecific(yos_task_key_t key)
{
    return NULL;
}

int yos_mutex_new(yos_mutex_t *mutex)
{
    int ret;

    pthread_mutex_t *m = malloc(sizeof(pthread_mutex_t));

    ret = pthread_mutex_init(m, NULL);

    mutex->hdl = m;

    return ret;
}

void yos_mutex_free(yos_mutex_t *mutex)
{
    pthread_mutex_t *m = mutex->hdl;

    pthread_mutex_destroy(m);

    mutex->hdl = NULL;

    free(m);
}

int yos_mutex_lock(yos_mutex_t mutex, int timeout)
{
    return pthread_mutex_lock(mutex.hdl);
}

int yos_mutex_unlock(yos_mutex_t mutex)
{
    return pthread_mutex_unlock(mutex.hdl);
}

int yos_sem_new(yos_sem_t *sem, int count)
{
    sem_t *s = malloc(sizeof(*s));

    sem_init(s, 0, count);

    sem->hdl = s;

    return 0;
}

void yos_sem_free(yos_sem_t *sem)
{
    sem_t *s = sem->hdl;

    sem_destroy(s);

    sem->hdl = NULL;

    free(s);
}

int yos_sem_wait(yos_sem_t sem, int timeout)
{
    int ret;

    struct timespec ts;

    if (!timeout || timeout == -1U) {
        return sem_wait(sem.hdl);
    }

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += timeout / 1000;
    timeout    %= 1000;
    ts.tv_nsec += timeout * 1000000;
    ts.tv_sec  += ts.tv_nsec / 1000000000;
    ts.tv_nsec %= 1000000000;

    while ((ret = sem_timedwait(sem.hdl, &ts)) != 0) {
        if (errno != EINTR) {
            break;
        }
    }

    return ret;
}

void yos_sem_signal(yos_sem_t sem)
{
    sem_post(sem.hdl);
}

int yos_queue_new(yos_queue_t *queue, void **start, intt msg_num)
{
}

int yos_queue_free(yos_queue_t *queue)
{
}

int yos_queue_send(yos_queue_t *queue, void *msg)
{
}

int yos_queue_recv(yos_queue_t *queue, int ticks, void **msg)
{
}

int yos_timer_new(yos_timer_t *timer, const name_t *name, void (*fn)(void *),
                  int round, void *arg, int repeat)
{
}

int yos_timer_free(yos_timer_t *timer)
{
}

int yos_timer_start(yos_timer_t *timer)
{
}

int yos_timer_stop(yos_timer_t *timer)
{
}

int yos_timer_change(yos_timer_t *timer, int first, int round)
{
}

int yos_workqueue_create(yos_workqueue_t *workqueue, const name_t *name,
                         int pri, char *stack_buf, int stack_size)
{
}

int yos_workqueue_del(yos_workqueue_t *workqueue)
{
}

int yos_work_init(yos_work_t *work, void (*fn)(void *), void *arg, int dly)
{
}

int yos_work_run(yos_workqueue_t *workqueue, yos_work_t *work)
{
}

int yos_work_sched(yos_work_t *work)
{
}

int yos_work_cancel(yos_work_t *work)
{
}

void *yos_malloc(int size)
{
    return malloc(size);
}

void yos_free(void *mem)
{
    free(mem);
}

long long yos_now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);

    return (((long long) ts.tv_sec) * ((long long) 1e9) + (long long) ts.tv_nsec);
}

void yos_msleep(int ms)
{
}

