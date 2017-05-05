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

#pragma  comment (lib, "winmm.lib")

#define  _WIN32_WINNT  0x0600
#define   WIN32_LEAN_AND_MEAN

#include <k_api.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <conio.h>
#include <stdarg.h>
#include <windows.h>
#include <mmsystem.h>
#include <assert.h>

#ifdef __cplusplus
extern  "C" {
#endif

static uint32_t firstly_use_printf;
kmutex_t mutext_printf;

void vc_port_printf(char *f, ...)
{
    va_list args;
    kstat_t  err;

    if (firstly_use_printf == 0) {
        yunos_mutex_create(&mutext_printf, "mutex1");
        firstly_use_printf = 1;
    }

    err = yunos_mutex_lock(&mutext_printf, YUNOS_WAIT_FOREVER);
    assert(err == YUNOS_SUCCESS);

    va_start(args, f);

#ifdef _MSC_VER
    vprintf(f, args);
#else
    vprintf(f, args); ;
#endif

    va_end(args);

    err = yunos_mutex_unlock(&mutext_printf);
    assert(err == YUNOS_SUCCESS);
}

typedef  void (*task_callback_fun)(void *arg);

typedef  enum {
    TASK_STATE_CREATED = 0x100,
    TASK_STATE_RUNNING,
    TASK_STATE_SUSPENDED,
    TASK_STATE_INTERRUPTED,
    TASK_STATE_TERMINATING,
    TASK_STATE_TERMINATED
} task_state_t;


typedef  struct  task_wrapper_s {
    struct  task_wrapper_s *next;
    struct  task_wrapper_s *prev;

    ktask_t                 *tcb;
    HANDLE                  task_handle;
    HANDLE                  signal_start;
    HANDLE                  signal;
    const name_t           *task_name;
    void                   *task_arg;
    task_callback_fun       task_cb;
    volatile  task_state_t  task_state;
    DWORD                   task_id;

} task_wrapper_t;

static task_wrapper_t *task_list;
static HANDLE          terminate_signal;
static HANDLE          tick_task;
static DWORD           tick_task_id;
static HANDLE          tick_signal;
static TIMECAPS        tick_caps;
static MMRESULT        tick_timer_id;

static HANDLE critical_section_mutex;

static DWORD WINAPI tick_proc(LPVOID arg);
static DWORD WINAPI task_wrapper(LPVOID arg);

static task_wrapper_t *task_get(ktask_t *tcb);
static void task_terminate(task_wrapper_t *task);

uint8_t cur_task_intrpt_resume(void);
uint8_t cur_task_intrpt_suspend(void);

static void task_terminate(task_wrapper_t  *task)
{
    task_wrapper_t *task_next;
    task_wrapper_t *task_prev;

    CloseHandle(task->signal_start);
    CloseHandle(task->signal);

    memset(task, 0, sizeof(task_wrapper_t));
    task->task_state = TASK_STATE_TERMINATED;
    
    cpu_intrpt_save();

    task_prev = task->prev;
    task_next = task->next;

    if (task_prev == (task_wrapper_t *)NULL) {
        task_list = task_next;

        if (task_next != (task_wrapper_t *)NULL) {
            task_next->prev = (task_wrapper_t *)NULL;
        }

        task->next = (task_wrapper_t *)NULL;
    } else if (task_next == (task_wrapper_t *)NULL) {
        task_prev->next = (task_wrapper_t *)NULL;
        task->prev      = (task_wrapper_t *)NULL;
    } else {
        task_prev->next =  task_next;
        task_next->prev =  task_prev;
        task->next      = (task_wrapper_t *)NULL;
        task->prev      = (task_wrapper_t *)NULL;
    }

    cpu_intrpt_restore(0);
}

static task_wrapper_t *task_get(ktask_t  *tcb)
{
    task_wrapper_t  *task;

    task = (task_wrapper_t *)tcb->task_stack;

    if (task != NULL) {
        return task;
    }

    task = task_list;

    while (task != NULL) {
        if (task->tcb == tcb) {
            return task;
        }

        task = task->next;
    }

    return (NULL);
}

static DWORD WINAPI task_wrapper(LPVOID  arg)
{
    task_wrapper_t *task;
    ktask_t         *tcb;

    tcb  = (ktask_t *)arg;
    task = task_get(tcb);

    task->task_state = TASK_STATE_SUSPENDED;
    WaitForSingleObject(task->signal, INFINITE);

    task->task_state = TASK_STATE_RUNNING;
    SetEvent(task->signal_start);

    task->task_cb(task->task_arg);

    yunos_task_del(tcb);

    return (0u);
}


void sys_init_hook(void)
{
    HANDLE handle;

    task_list        = NULL;
    terminate_signal = NULL;
    tick_task          = NULL;
    tick_signal      = NULL;

    critical_section_mutex = CreateMutex(NULL, FALSE, NULL);

    if (critical_section_mutex == NULL) {
        return;
    }

    handle = GetCurrentProcess();
    SetPriorityClass(handle, REALTIME_PRIORITY_CLASS);
    SetProcessAffinityMask(handle, 1);

    terminate_signal = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (terminate_signal == NULL) {
        return;
    }

    tick_task = CreateThread(NULL, 0, tick_proc, 0, CREATE_SUSPENDED, &tick_task_id);

    if (tick_task == NULL) {
        CloseHandle(terminate_signal);
        terminate_signal = NULL;
        return;
    }

    SetThreadPriority(tick_task, THREAD_PRIORITY_TIME_CRITICAL);
    SetThreadPriorityBoost(tick_task, TRUE);

    if (timeGetDevCaps(&tick_caps, sizeof(tick_caps)) != TIMERR_NOERROR) {
        CloseHandle(tick_task);
        CloseHandle(terminate_signal);

        tick_task          = NULL;
        terminate_signal = NULL;
        return;
    }

    tick_caps.wPeriodMin = 1u;

    if (timeBeginPeriod(tick_caps.wPeriodMin) != TIMERR_NOERROR) {
        CloseHandle(tick_task);
        CloseHandle(terminate_signal);

        tick_task          = NULL;
        terminate_signal = NULL;
        return;
    }

    tick_signal = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (tick_signal == NULL) {
        timeEndPeriod(tick_caps.wPeriodMin);
        CloseHandle(tick_task);
        CloseHandle(terminate_signal);

        tick_task        = NULL;
        terminate_signal = NULL;
        return;
    }

    tick_timer_id = timeSetEvent((UINT)(1000u / YUNOS_CONFIG_TICKS_PER_SECOND),
                                 (UINT) tick_caps.wPeriodMin,
                                 (LPTIMECALLBACK) tick_signal,
                                 0,
                                 (UINT)(TIME_PERIODIC | TIME_CALLBACK_EVENT_SET));

    if (tick_timer_id == 0u) {
        CloseHandle(tick_signal);
        timeEndPeriod(tick_caps.wPeriodMin);
        CloseHandle(tick_task);
        CloseHandle(terminate_signal);

        tick_signal      = NULL;
        tick_task          = NULL;
        terminate_signal = NULL;
        return;
    }
}

size_t cpu_intrpt_save(void)
{
    DWORD ret;

    ret = WaitForSingleObject(critical_section_mutex, INFINITE);

    return 0;
}

void cpu_intrpt_restore(size_t cpsr)
{
    DWORD ret;
    cpsr = cpsr;

    ret = ReleaseMutex(critical_section_mutex);
}

static DWORD WINAPI tick_proc(LPVOID  arg)
{
    uint8_t terminate;
    uint8_t suspended;
    HANDLE  wait_signal[2];

    wait_signal[0] = terminate_signal;
    wait_signal[1] = tick_signal;

    arg = arg;
    terminate = YUNOS_FALSE;

    while (!terminate) {
        switch (WaitForMultipleObjects(2, wait_signal, FALSE, INFINITE)) {
            case WAIT_OBJECT_0 + 1u:
                ResetEvent(tick_signal);

                cpu_intrpt_save();

                suspended = cur_task_intrpt_suspend();

                if (suspended == YUNOS_TRUE) {

                    yunos_intrpt_enter();
#if (YUNOS_CONFIG_TICKLESS == 0)
                    yunos_tick_proc();
#else
                    /* fix me,not support yet*/
                    yunos_tickless_proc(0);
#endif
                    yunos_intrpt_exit();

                    cur_task_intrpt_resume();
                }

                cpu_intrpt_restore(0);
                break;

            case WAIT_OBJECT_0 + 0u:
                terminate = YUNOS_TRUE;
                break;

            default:
                terminate = YUNOS_TRUE;
                break;
        }
    }

    return (0u);
}

void task_create_hook(ktask_t *tcb)
{
    task_wrapper_t *task;

    task            = task_get(tcb);
    task->task_name = tcb->task_name;

    task->signal = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (task->signal == NULL) {
        return;
    }

    task->signal_start = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (task->signal_start == NULL) {
        CloseHandle(task->signal);
        task->signal = NULL;
        return;
    }

    task->task_handle = CreateThread(NULL, 0, task_wrapper, tcb, CREATE_SUSPENDED, &task->task_id);
    SetThreadPriority(task->task_handle, THREAD_PRIORITY_IDLE);
    SetThreadPriorityBoost(task->task_handle, TRUE);

    if (task->task_handle == NULL) {
        CloseHandle(task->signal_start);
        CloseHandle(task->signal);

        task->signal_start = NULL;
        task->signal       = NULL;
        return;
    }

    task->task_state = TASK_STATE_CREATED;
    task->tcb        = tcb;

    cpu_intrpt_save();

    task->prev = (task_wrapper_t *)NULL;

    if (task_list == (task_wrapper_t *)NULL) {
        task->next = (task_wrapper_t *)NULL;
    } else {
        task->next        = task_list;
        task_list->prev = task;
    }

    task_list = task;

    cpu_intrpt_restore(0);
}

void task_del_hook(ktask_t  *tcb)
{
    task_wrapper_t *task;

    task = task_get(tcb);

    if (task == (task_wrapper_t *)NULL) {
        return;
    }

    switch (task->task_state) {
        case TASK_STATE_RUNNING:
            if (GetCurrentThreadId() == task->task_id) {
                task->task_state = TASK_STATE_TERMINATING;
            } else {
                TerminateThread(task->task_handle, 0);
                CloseHandle(task->task_handle);

                task_terminate(task);
            }

            break;

        case TASK_STATE_CREATED:
        case TASK_STATE_SUSPENDED:
        case TASK_STATE_INTERRUPTED:
            TerminateThread(task->task_handle, 0);
            CloseHandle(task->task_handle);

            task_terminate(task);
            break;

        default:
            break;
    }
}

void *cpu_task_stack_init(cpu_stack_t *base, size_t size, void *arg, task_entry_t entry)
{
    task_wrapper_t *task_info;

    task_info = (task_wrapper_t *)base;
    memset(task_info, 0, sizeof(task_wrapper_t));
    task_info->task_arg   = arg;
    task_info->task_cb    = entry;

    return (base);
}

void cpu_first_task_start(void)
{
    task_wrapper_t *task;
    ktask_t *tcb;

    task = task_get(g_preferred_ready_task);
    ResumeThread(task->task_handle);

    SignalObjectAndWait(task->signal, task->signal_start, INFINITE, FALSE);
    ResumeThread(tick_task);
    WaitForSingleObject(tick_task, INFINITE);

    timeKillEvent(tick_timer_id);
    timeEndPeriod(tick_caps.wPeriodMin);
    CloseHandle(tick_signal);
    CloseHandle(tick_task);
    CloseHandle(terminate_signal);

    yunos_sched_disable();

    cpu_intrpt_save();

    task = task_list;

    while (task != NULL) {
        tcb  = task->tcb;
        task = task->next;

        if (tcb == &g_idle_task) {
            task_del_hook(tcb);
        } else {
            yunos_task_del(tcb);
        }

        Sleep(1);
    }

    cpu_intrpt_restore(0);
    CloseHandle(critical_section_mutex);
}

void cpu_task_switch(void)
{
    task_wrapper_t *task_cur;
    task_wrapper_t *task_new;

    task_cur = task_get(g_active_task);

    yunos_task_sched_stats_get();

    g_active_task = g_preferred_ready_task;

    if (task_cur->task_state == TASK_STATE_RUNNING) {
        task_cur->task_state = TASK_STATE_SUSPENDED;
    }

    task_new = task_get(g_preferred_ready_task);

    switch (task_new->task_state) {
        case TASK_STATE_CREATED:
            ResumeThread(task_new->task_handle);
            SignalObjectAndWait(task_new->signal, task_new->signal_start, INFINITE, FALSE);
            break;

        case TASK_STATE_SUSPENDED:
            task_new->task_state = TASK_STATE_RUNNING;
            SetEvent(task_new->signal);
            break;

        case TASK_STATE_INTERRUPTED:
            task_new->task_state = TASK_STATE_RUNNING;
            ResumeThread(task_new->task_handle);
            break;

        default:
            return;
    }

    if (task_cur->task_state == TASK_STATE_TERMINATING) {
        task_terminate(task_cur);

        cpu_intrpt_restore(0);

        ExitThread(0);
        return;
    }

    cpu_intrpt_restore(0);
    WaitForSingleObject(task_cur->signal, INFINITE);
    cpu_intrpt_save();
}

void cpu_intrpt_switch()
{
    yunos_task_sched_stats_get();
    g_active_task = g_preferred_ready_task;
}

uint8_t cur_task_intrpt_suspend(void)
{
    ktask_t         *tcb;
    task_wrapper_t *task;
    uint8_t         ret;
    DWORD           win_ret;

    tcb  =  g_active_task;
    task = task_get(tcb);

    switch (task->task_state) {
        case TASK_STATE_RUNNING:
            win_ret = SuspendThread(task->task_handle);
            assert(win_ret != -1);
            task->task_state = TASK_STATE_INTERRUPTED;

            ret = YUNOS_TRUE;
            break;

        case TASK_STATE_TERMINATING:
            TerminateThread(task->task_handle, 0);
            CloseHandle(task->task_handle);

            task_terminate(task);

            ret = YUNOS_TRUE;
            break;

        default:
            ret = YUNOS_FALSE;
            break;
    }

    return (ret);
}

uint8_t cur_task_intrpt_resume(void)
{
    ktask_t         *tcb;
    task_wrapper_t *task;
    uint8_t         ret;

    tcb  = g_preferred_ready_task;
    task = task_get(tcb);

    switch (task->task_state) {
        case TASK_STATE_CREATED:
            ResumeThread(task->task_handle);
            SignalObjectAndWait(task->signal, task->signal_start, INFINITE, FALSE);
            ret = YUNOS_TRUE;
            break;

        case TASK_STATE_INTERRUPTED:
            task->task_state = TASK_STATE_RUNNING;
            ResumeThread(task->task_handle);
            ret = YUNOS_TRUE;
            break;

        case TASK_STATE_SUSPENDED:
            task->task_state = TASK_STATE_RUNNING;
            SetEvent(task->signal);
            ret = YUNOS_TRUE;
            break;

        default:
            ret = YUNOS_FALSE;
            break;
    }

    return (ret);
}

#ifdef __cplusplus
}
#endif

