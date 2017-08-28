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

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <pthread.h>

#include <yos/kernel.h>

void yos_reboot(void)
{
    exit(0);
}

int yos_get_hz(void)
{
    return 100;
}

const char *yos_version_get(void)
{
    return "yos-linux-xxx";
}

const char *yos_strerror(int errnum)
{
    return strerror(errnum);
}

struct targ {
    const char *name;
    void (*fn)(void *);
    void *arg;
};

static void *dfl_entry(void *arg)
{
    struct targ *targ = arg;
    void (*fn)(void *) = targ->fn;
    void *farg = targ->arg;
    prctl(PR_SET_NAME, (unsigned long)targ->name, 0, 0, 0);
    free(targ);

    fn(farg);

    return 0;
}

int yos_task_new(const char *name, void (*fn)(void *), void *arg,
                 int stack_size)
{
    pthread_t th;
    struct targ *targ = malloc(sizeof(*targ));
    targ->name = strdup(name);
    targ->fn = fn;
    targ->arg = arg;
    return pthread_create(&th, NULL, dfl_entry, targ);
}

int yos_task_new_ext(yos_task_t *task, const char *name, void (*fn)(void *), void *arg,
                     int stack_size, int prio)
{
    return yos_task_new(name, fn, arg, stack_size);
}

void yos_task_exit(int code)
{
    pthread_exit((void *)code);
}

const char *yos_task_name(void)
{
    static char name[16];
    prctl(PR_GET_NAME, (unsigned long)name, 0, 0, 0);
    return name;
}

int yos_task_key_create(yos_task_key_t *key)
{
    return pthread_key_create(key, NULL);
}

void yos_task_key_delete(yos_task_key_t key)
{
    pthread_key_delete(key);
}

int yos_task_setspecific(yos_task_key_t key, void *vp)
{
    return pthread_setspecific(key, vp);
}

void *yos_task_getspecific(yos_task_key_t key)
{
    return pthread_getspecific(key);
}

int yos_mutex_new(yos_mutex_t *mutex)
{
    pthread_mutex_t *mtx = malloc(sizeof(*mtx));
    pthread_mutex_init(mtx, NULL);
    mutex->hdl = mtx;
    return 0;
}

void yos_mutex_free(yos_mutex_t *mutex)
{
    pthread_mutex_destroy(mutex->hdl);
    free(mutex->hdl);
}

int yos_mutex_lock(yos_mutex_t *mutex, unsigned int timeout)
{
    if (mutex)
        pthread_mutex_lock(mutex->hdl);
    return 0;
}

int yos_mutex_unlock(yos_mutex_t *mutex)
{
    if (mutex)
        pthread_mutex_unlock(mutex->hdl);
    return 0;
}

int yos_mutex_is_valid(yos_mutex_t *mutex)
{
    return mutex->hdl != NULL;
}

#include <semaphore.h>
int yos_sem_new(yos_sem_t *sem, int count)
{
    sem_t *s = malloc(sizeof(*s));
    sem_init(s, 0, count);
    sem->hdl = s;
    return 0;
}

void yos_sem_free(yos_sem_t *sem)
{
    sem_destroy(sem->hdl);
    free(sem->hdl);
}

int yos_sem_wait(yos_sem_t *sem, unsigned int timeout)
{
    return sem_wait(sem->hdl);
}

void yos_sem_signal(yos_sem_t *sem)
{
    sem_post(sem->hdl);
}

int yos_sem_is_valid(yos_sem_t *sem)
{
    return sem->hdl != NULL;
}

void yos_sem_signal_all(yos_sem_t *sem)
{
    sem_post(sem->hdl);
}

struct queue {
    int fds[2];
    void *buf;
    int size;
};

int yos_queue_new(yos_queue_t *queue, void *buf, unsigned int size, int max_msg)
{
    struct queue *q = malloc(sizeof(*q));
    pipe(q->fds);
    q->buf = buf;
    q->size = size;
    queue->hdl = q;
    return 0;
}

void yos_queue_free(yos_queue_t *queue)
{
    struct queue *q = queue->hdl;
    close(q->fds[0]);
    close(q->fds[1]);
    free(q);
}

int yos_queue_send(yos_queue_t *queue, void *msg, unsigned int size)
{
    struct queue *q = queue->hdl;
    write(q->fds[1], msg, size);
    return 0;
}

int yos_queue_recv(yos_queue_t *queue, unsigned int ms, void *msg,
                   unsigned int *size)
{
    struct queue *q = queue->hdl;
    *size = read(q->fds[0], msg, *size);
    return 0;
}

int yos_queue_is_valid(yos_queue_t *queue)
{
    return queue->hdl != NULL;
}

void *yos_queue_buf_ptr(yos_queue_t *queue)
{
    struct queue *q = queue->hdl;
    return q->buf;
}

int yos_sched_disable()
{
    return -1;
}

int yos_sched_enable()
{
    return -1;
}

int yos_timer_new(yos_timer_t *timer, void (*fn)(void *, void *),
                  void *arg, int ms, int repeat)
{
    return -1;
}

void yos_timer_free(yos_timer_t *timer)
{
}

int yos_timer_start(yos_timer_t *timer)
{
    return -1;
}

int yos_timer_stop(yos_timer_t *timer)
{
    return -1;
}

int yos_timer_change(yos_timer_t *timer, int ms)
{
    return -1;
}

int yos_workqueue_create(yos_workqueue_t *workqueue, int pri, int stack_size)
{
    return -1;
}

void yos_workqueue_del(yos_workqueue_t *workqueue)
{
}

struct work {
    void (*fn)(void *);
    void *arg;
    int dly;
};

int yos_work_init(yos_work_t *work, void (*fn)(void *), void *arg, int dly)
{
    struct work *w = malloc(sizeof(*w));
    w->fn = fn;
    w->arg = arg;
    w->dly = dly;
    work->hdl = w;
    return 0;
}

void yos_work_destroy(yos_work_t *work)
{
    free(work->hdl);
}

int yos_work_run(yos_workqueue_t *workqueue, yos_work_t *work)
{
    return yos_work_sched(work);
}

static void worker_entry(void *arg)
{
    struct work *w = arg;
    if (w->dly)
        usleep(w->dly * 1000);
    w->fn(w->arg);
}

int yos_work_sched(yos_work_t *work)
{
    struct work *w = work->hdl;
    return yos_task_new("worker", worker_entry, w, 8192);
}

int yos_work_cancel(yos_work_t *work)
{
    return -1;
}

void *yos_zalloc(unsigned int size)
{
    return calloc(size, 1);
}

void *yos_malloc(unsigned int size)
{
    return malloc(size);
}

void *yos_realloc(void *mem, unsigned int size)
{
    return realloc(mem, size);
}

void yos_alloc_trace(void *addr, size_t allocator)
{
}

void yos_free(void *mem)
{
    free(mem);
}

long long yos_now(void)
{
    struct timeval tv;
    long long ns;
    gettimeofday(&tv, NULL);
    ns = tv.tv_sec * 1000000LL + tv.tv_usec;
    return ns * 1000LL;
}

long long yos_now_ms(void)
{
    struct timeval tv;
    long long ms;
    gettimeofday(&tv, NULL);
    ms = tv.tv_sec * 1000LL + tv.tv_usec / 1000;
    return ms;
}

void yos_msleep(int ms)
{
    usleep(ms * 1000);
}

void yunos_init(void)
{
}

void yunos_start(void)
{
    while(1)
        usleep(1000 * 1000 * 100);
}

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
void dumpsys_task_func(void)
{
    DIR *proc = opendir("/proc/self/task");
    while (1) {
        struct dirent *ent = readdir(proc);
        if (!ent) break;
        if (ent->d_name[0] == '.') continue;

        char fn[128];
        snprintf(fn, sizeof fn, "/proc/self/task/%s/comm", ent->d_name);
        FILE *fp = fopen(fn, "r");
        if (!fp) continue;
        bzero(fn, sizeof fn);
        fread(fn, sizeof fn, 1, fp);
        fclose(fp);
        printf("%8s - %s", ent->d_name, fn);
    }
}

