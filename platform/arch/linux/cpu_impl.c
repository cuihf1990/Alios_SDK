/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
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

#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <yos/kernel.h>
#include <yos/list.h>

#include <cpu_event.h>
#include <k_api.h>
#include <time.h>
#include <ucontext.h>

#ifdef HAVE_VALGRIND_H
#include <valgrind.h>
#elif defined(HAVE_VALGRIND_VALGRIND_H)
#include <valgrind/valgrind.h>
#else
#define VALGRIND_STACK_REGISTER(...)
#endif

#define LOG(format, ...) //printf(format, __VA_ARGS__)

#ifdef GCOV_ENABLE
#define MIN_STACK_SIZE    8192
#else
#define MIN_STACK_SIZE    4096
#endif

typedef struct {
    ktask_t     *tcb;
    void        *arg;
    task_entry_t entry;
    ucontext_t  uctx;
    void        *real_stack;
    void        *real_stack_end;
    void        *orig_stack;
    int          orig_size;
    int          in_signals;
    int          int_lvl;
    int          saved_errno;
#if defined(HAVE_VALGRIND_H)||defined(HAVE_VALGRIND_VALGRIND_H)
    int          vid;
#endif
} task_ext_t;

static sigset_t cpu_sig_set;
static void cpu_sig_handler(int signo, siginfo_t *si, void *ucontext);
static void _cpu_task_switch(void);

static void task_proc(void);

/* data structures for cpu intr handling */
static ktask_t g_intr_task;
static ksem_t  g_intr_sem;
static klist_t g_event_list = { &g_event_list, &g_event_list };
static klist_t g_recycle_list = { &g_recycle_list, &g_recycle_list };
static dlist_t g_io_list = YOS_DLIST_INIT(g_io_list);
static int cpu_event_inited;

typedef struct {
    dlist_t node;
    void (*cb)(int, void *);
    void *arg;
} cpu_io_cb_t;

/*
 * this is tricky
 * as we need to compete with Rhino and non-Rhino tasks
 */
static pthread_mutex_t g_event_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t rhino_cpu_thread;

typedef struct {
    klist_t node;
    cpu_event_t event;
} cpu_event_internal_t;

static inline void enter_signal(int signo)
{
    task_ext_t *tcb_ext = (task_ext_t *)g_active_task->task_stack;
    tcb_ext->in_signals ++;
    assert(tcb_ext->in_signals == 1);
}

static inline void leave_signal(int signo)
{
    task_ext_t *tcb_ext = (task_ext_t *)g_active_task->task_stack;
    tcb_ext->in_signals --;
    assert(tcb_ext->in_signals >= 0);
}

static inline int in_signal(void)
{
    if (!g_active_task)
        return 0;

    task_ext_t *tcb_ext = (task_ext_t *)g_active_task->task_stack;
    return tcb_ext->in_signals;
}

sigset_t cpu_intrpt_save(void)
{
    sigset_t    oldset = {};
    if (in_signal())
        return oldset;

    sigprocmask(SIG_BLOCK, &cpu_sig_set, &oldset);
    if (g_active_task) {
        task_ext_t *tcb_ext = (task_ext_t *)g_active_task->task_stack;
        tcb_ext->int_lvl ++;
    }

    return oldset;
}

void cpu_intrpt_restore(sigset_t cpsr)
{
    if (in_signal())
        return;

    if (!g_active_task) {
        goto out;
    }

    task_ext_t *tcb_ext = (task_ext_t *)g_active_task->task_stack;
    tcb_ext->int_lvl --;
    if (tcb_ext->int_lvl)
        return;

out:
    sigprocmask(SIG_UNBLOCK, &cpu_sig_set, NULL);
}

void cpu_task_switch(void)
{
    task_ext_t *tcb_ext = (task_ext_t *)g_active_task->task_stack;
    _cpu_task_switch();
    assert(!in_signal());
    assert((void *)&tcb_ext >= tcb_ext->real_stack && (void *)&tcb_ext < tcb_ext->real_stack_end);
}

void cpu_intrpt_switch(void)
{
    _cpu_task_switch();
    assert(in_signal());
}

void cpu_first_task_start(void)
{
    ktask_t    *tcb     = g_preferred_ready_task;
    task_ext_t *tcb_ext = (task_ext_t *)tcb->task_stack;

    struct itimerval value;
    struct itimerval ovalue;

    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 1000000u / YUNOS_CONFIG_TICKS_PER_SECOND;
    value.it_value.tv_sec = 1;
    value.it_value.tv_usec = 0;

    setitimer(ITIMER_REAL, &value, &ovalue);

    setcontext(&tcb_ext->uctx);

    raise(SIGABRT);
}

void *cpu_task_stack_init(cpu_stack_t *base, size_t size, void *arg, task_entry_t entry)
{
    CPSR_ALLOC();

    size_t real_size = size > MIN_STACK_SIZE ? size : MIN_STACK_SIZE;
    real_size *= sizeof(cpu_stack_t);

    void *real_stack = yos_malloc(real_size);
    task_ext_t *tcb_ext = (task_ext_t *)base;
    cpu_stack_t *tmp;

    bzero(real_stack, real_size);

    tcb_ext->tcb   = NULL;
    tcb_ext->arg   = arg;
    tcb_ext->entry = entry;
    /* todo+ replace malloc with mmap */
    tcb_ext->real_stack = real_stack;
    tcb_ext->real_stack_end = real_stack + real_size;

#if (YUNOS_CONFIG_TASK_STACK_OVF_CHECK > 0)
#if (YUNOS_CONFIG_CPU_STACK_DOWN > 0)
    tmp  = tcb_ext->real_stack;
    *tmp = YUNOS_TASK_STACK_OVF_MAGIC;
#else
    tmp  = (cpu_stack_t *)(tcb_ext->real_stack) + (real_size/sizeof(cpu_stack_t)) - 1u;
    *tmp = YUNOS_TASK_STACK_OVF_MAGIC;
#endif
#endif

#if defined(HAVE_VALGRIND_H)||defined(HAVE_VALGRIND_VALGRIND_H)
    tcb_ext->vid = VALGRIND_STACK_REGISTER(tcb_ext->real_stack, (char *)(tcb_ext->real_stack) + real_size);
#endif

    YUNOS_CPU_INTRPT_DISABLE();

    int ret = getcontext(&tcb_ext->uctx);
    if (ret < 0) {
        YUNOS_CPU_INTRPT_ENABLE();
        yos_free(real_stack);
        return NULL;
    }

    tcb_ext->uctx.uc_stack.ss_sp = tcb_ext->real_stack;
    tcb_ext->uctx.uc_stack.ss_size = real_size;
    makecontext(&tcb_ext->uctx, task_proc, 0);

    YUNOS_CPU_INTRPT_ENABLE();

    return tcb_ext;
}

void cpu_task_create_hook(ktask_t *tcb)
{
    task_ext_t *tcb_ext = (task_ext_t *)tcb->task_stack;

    LOG("+++ Task '%-20s' [%3.1d] is created\n", tcb->task_name, tcb->prio);

    tcb_ext->tcb = tcb;
    tcb_ext->orig_stack = tcb->task_stack_base;
    tcb_ext->orig_size = tcb->stack_size;
    /* hack: replace task_stack_base for stack checking */
    tcb->task_stack_base = tcb_ext->real_stack;
    tcb->stack_size = tcb_ext->real_stack_end - tcb_ext->real_stack;
    tcb->stack_size /= sizeof(cpu_stack_t);
}

void cpu_task_del_hook(ktask_t *tcb)
{
    task_ext_t *tcb_ext = (task_ext_t *)tcb->task_stack;
    LOG("--- Task '%-20s' is deleted\n", tcb->task_name);
#if defined(HAVE_VALGRIND_H)||defined(HAVE_VALGRIND_VALGRIND_H)
    VALGRIND_STACK_DEREGISTER(tcb_ext->vid);
#endif
    g_sched_lock++;

    /*
     * ---- hack -----
     * for DYN_ALLOC case,
     * tcb->task_stack_base is replaced with real_stack in create_hook,
     * and tcb->task_stack_base is freed in yunos_task_dyn_del,
     * so we just need to free orig_stack
     * for STATIC_ALLOC case, need to free real_stack by ourself
     */
    if (tcb->mm_alloc_flag == K_OBJ_DYN_ALLOC)
        yunos_queue_back_send(&g_dyn_queue, tcb_ext->orig_stack);
    else
        yunos_queue_back_send(&g_dyn_queue, tcb_ext->real_stack);
    g_sched_lock--;

}

void task_proc(void)
{
    ktask_t *task_tcb;

    task_ext_t *tcb_ext = (task_ext_t *)g_active_task->task_stack;

    LOG("Task '%-20s' running\n", tcb_ext->tcb->task_name);

    /* signals blocked before makecontext, unblock here */
    sigprocmask(SIG_UNBLOCK, &cpu_sig_set, NULL);

    (tcb_ext->entry)(tcb_ext->arg);

    LOG("Task %-20s end\n", tcb_ext->tcb->task_name);

    task_tcb = tcb_ext->tcb;
    if (task_tcb->mm_alloc_flag == K_OBJ_STATIC_ALLOC) {
        yunos_task_del(tcb_ext->tcb);
    }
    else if (task_tcb->mm_alloc_flag == K_OBJ_DYN_ALLOC) {
        yunos_task_dyn_del(tcb_ext->tcb);
    }
    else {
        LOG("System crash, the mm_alloc_flag of task is %d\n", task_tcb->mm_alloc_flag);
        assert(0);
    }
}

static void _cpu_task_switch(void)
{
    ktask_t     *from_tcb;
    ktask_t     *to_tcb;
    task_ext_t  *from_tcb_ext;
    task_ext_t  *to_tcb_ext;

    from_tcb = g_active_task;
    from_tcb_ext = (task_ext_t *)from_tcb->task_stack;

    to_tcb = g_preferred_ready_task;
    to_tcb_ext = (task_ext_t *)to_tcb->task_stack;
    LOG("TASK SWITCH, '%-20s' (%d) -> '%-20s' (%d)\n",
        from_tcb->task_name, from_tcb->task_state,
        to_tcb->task_name, to_tcb->task_state);

    /* save errno */
    from_tcb_ext->saved_errno = errno;

#if (YUNOS_CONFIG_TASK_STACK_OVF_CHECK > 0)
    assert(*(g_active_task->task_stack_base) == YUNOS_TASK_STACK_OVF_MAGIC);
#endif

    g_active_task = g_preferred_ready_task;

    swapcontext(&from_tcb_ext->uctx, &to_tcb_ext->uctx);

    /* restore errno */
    errno = from_tcb_ext->saved_errno;
}

void cpu_idle_hook(void)
{
    usleep(10);
}

void *cpu_event_malloc(int size)
{
    return malloc(size);
}

void cpu_event_free(void *p)
{
    pthread_mutex_lock(&g_event_mutex);
    klist_insert(&g_recycle_list, (klist_t *)p);
    pthread_mutex_unlock(&g_event_mutex);
}

int cpu_notify_event(cpu_event_t *event)
{
    int          ret;
    cpu_event_internal_t *kevent = cpu_event_malloc(sizeof(*kevent));

    memcpy(&kevent->event, event, sizeof(*event));
    pthread_mutex_lock(&g_event_mutex);

    klist_insert(&g_event_list, &kevent->node);
    while (!is_klist_empty(&g_recycle_list)) {
        klist_t *node = g_recycle_list.next;
        klist_rm(node);
        free(node);
    }

    pthread_mutex_unlock(&g_event_mutex);

    if (!cpu_event_inited)
        return 0;

    ret = pthread_kill(rhino_cpu_thread, SIGUSR2);

    return ret;
}

static void cpu_intr_entry(void *arg)
{
    while (1) {
        while (!is_klist_empty(&g_event_list)) {
            cpu_event_internal_t *kevent;

            pthread_mutex_lock(&g_event_mutex);

            kevent = yunos_list_entry(g_event_list.next, cpu_event_internal_t, node);
            klist_rm(&kevent->node);

            pthread_mutex_unlock(&g_event_mutex);

            kevent->event.handler(kevent->event.arg);
            cpu_event_free(kevent);
        }

        yunos_sem_take(&g_intr_sem, YUNOS_WAIT_FOREVER);
    }
}

static void create_intr_task(void)
{
    static uint32_t stack[1024];

    yunos_sem_create(&g_intr_sem, "intr count", 0);
    yunos_task_create(&g_intr_task, "cpu_intr", 0, 0, 0, stack, 1024, cpu_intr_entry, 1);

    cpu_event_inited = 1;
}

void cpu_init_hook(void)
{
    int              ret;
    struct sigaction tick_sig_action = {
        .sa_flags = SA_SIGINFO,
        .sa_sigaction = cpu_sig_handler,
    };
    struct sigaction event_sig_action = {
        .sa_flags = SA_SIGINFO,
        .sa_sigaction = cpu_sig_handler,
    };
    struct sigaction event_io_action = {
        .sa_flags = SA_SIGINFO,
        .sa_sigaction = cpu_sig_handler,
    };

    rhino_cpu_thread = pthread_self();

    sigemptyset(&cpu_sig_set);
    sigaddset(&cpu_sig_set, SIGALRM);
    sigaddset(&cpu_sig_set, SIGIO);
    sigaddset(&cpu_sig_set, SIGUSR2);

    tick_sig_action.sa_mask = cpu_sig_set;
    event_sig_action.sa_mask = cpu_sig_set;
    event_io_action.sa_mask = cpu_sig_set;

    ret = sigaction(SIGALRM, &tick_sig_action, NULL);
    ret |= sigaction(SIGUSR2, &event_sig_action, NULL);
    ret |= sigaction(SIGIO, &event_io_action, NULL);
    if (ret != 0u) {
        raise(SIGABRT);
    }
}

void cpu_start_hook(void)
{
    create_intr_task();
}

void cpu_io_register(void (*f)(int, void *), void *arg)
{
    sigset_t cpsr;
    cpu_io_cb_t *pcb = yos_malloc(sizeof(*pcb));
    pcb->cb = f;
    pcb->arg = arg;

    cpsr = cpu_intrpt_save();
    dlist_add_tail(&pcb->node, &g_io_list);
    cpu_intrpt_restore(cpsr);
}

void cpu_io_unregister(void (*f)(int, void *), void *arg)
{
    sigset_t cpsr;
    cpu_io_cb_t *pcb;
    cpsr = cpu_intrpt_save();
    dlist_for_each_entry(&g_io_list, pcb, cpu_io_cb_t, node) {
        if (pcb->cb != f)
            continue;
        if (pcb->arg != arg)
            continue;
        dlist_del(&pcb->node);
        cpu_intrpt_restore(cpsr);
        yos_free(pcb);
        return;
    }
    cpu_intrpt_restore(cpsr);
}

static void trigger_io_cb(int fd)
{
    cpu_io_cb_t *pcb;
    dlist_for_each_entry(&g_io_list, pcb, cpu_io_cb_t, node) {
        pcb->cb(fd, pcb->arg);
    }
}

void cpu_sig_handler(int signo, siginfo_t *si, void *ucontext)
{
    if (pthread_self() != rhino_cpu_thread) {
        return;
    }

    enter_signal(signo);
    yunos_intrpt_enter();

    if (signo == SIGALRM)
        yunos_tick_proc();
    else if (signo == SIGUSR2)
        yunos_sem_give(&g_intr_sem);
    else if (signo == SIGIO) {
        trigger_io_cb(si->si_fd);
    }

    yunos_intrpt_exit();
    leave_signal(signo);
}

