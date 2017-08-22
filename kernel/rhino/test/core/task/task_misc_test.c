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

#include <k_api.h>
#include <test_fw.h>

#define TASK_TEST_STACK_SIZE 512
static ktask_t *task_misc;
static ktask_t  task_misc2;
static ktask_t *task_misc3;
static ktask_t *task_misc4;

static ksem_t *sem;
static ksem_t *sem2;
static ksem_t *sem3;

static ksem_t       *sem4;
static kmutex_t     *mutex4;
static kevent_t     *event4;
static ktimer_t     *timer4;
static kbuf_queue_t *buf_queue4;
static kqueue_t     *queue4;

static kmutex_t mutex;

void task_misc_entry2(void *arg)
{
    yunos_sem_dyn_create(&sem2, "sem_misc22", 0);
    yunos_sem_take(sem2, YUNOS_WAIT_FOREVER);

    yunos_mutex_lock(&mutex, YUNOS_WAIT_FOREVER);
    yunos_mutex_unlock(&mutex);

    yunos_sem_dyn_create(&sem, "sem_misc2", 0);
    yunos_sem_take(sem, YUNOS_CONFIG_TICKS_PER_SECOND);
    yunos_sem_take(sem, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex, YUNOS_WAIT_FOREVER);

    yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND / 10);

    yunos_task_del(NULL);
    yunos_task_dyn_del(&task_misc2);
    yunos_task_del(&task_misc2);
}

void task_misc_entry3(void *arg)
{
    yunos_sem_dyn_create(&sem3, "sem_misc33", 0);
    yunos_sem_take(sem3, YUNOS_WAIT_FOREVER);

    yunos_sched_disable();
    yunos_task_dyn_del(task_misc3);
    yunos_sched_enable();

    yunos_task_dyn_del(task_misc3);
}

void task_misc_entry4(void *arg)
{
    kstat_t ret;

    yunos_task_suspend(task_misc);
    ret = yunos_task_wait_abort(task_misc);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_wait_abort suspend", PASS);
    } else {
        PRINT_RESULT("yunos_task_wait_abort suspend", FAIL);
        test_case_fail++;
    }

    yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);
    yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);
    yunos_task_dyn_del(task_misc4);
}


static void timer_cb4(void *timer, void *arg)
{


}

void task_misc_entry(void *arg)
{
    size_t       task_free;
    uint8_t      old_pri;
    kstat_t      ret;
    uint8_t      policy;
    void        *info;
    cpu_stack_t *task_misc2_stack;
    ktask_t       tmp;

    CPSR_ALLOC();

    yunos_task_resume(task_misc3);
    yunos_task_del(task_misc3);

    yunos_sched_disable();
    yunos_task_suspend(yunos_cur_task_get());
    yunos_sched_enable();

    yunos_task_suspend(NULL);

    yunos_task_sleep(0);

    yunos_sched_disable();
    yunos_task_sleep(1);
    yunos_sched_enable();

    yunos_task_yield();

    yunos_task_stack_min_free(NULL, &task_free);
    yunos_task_stack_min_free(task_misc, NULL);
    yunos_task_stack_min_free(task_misc, &task_free);

    yunos_mutex_create(&mutex, "test");
    yunos_mutex_lock(&mutex, YUNOS_WAIT_FOREVER);

    ret = yunos_task_pri_change(task_misc, 15, &old_pri);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_pri_change", PASS);
    } else {
        PRINT_RESULT("yunos_task_pri_change", FAIL);
        test_case_fail++;
    }

    yunos_mutex_unlock(&mutex);

    ret = yunos_task_pri_change(NULL, 15, &old_pri);

    if (ret == YUNOS_NULL_PTR) {
        test_case_success++;
        PRINT_RESULT("yunos_task_pri_change para 1", PASS);
    } else {
        PRINT_RESULT("yunos_task_pri_change para 1", FAIL);
        test_case_fail++;
    }

    ret = yunos_task_pri_change(task_misc, 15, NULL);

    if (ret == YUNOS_NULL_PTR) {
        test_case_success++;
        PRINT_RESULT("yunos_task_pri_change para 2", PASS);
    } else {
        PRINT_RESULT("yunos_task_pri_change para 2", FAIL);
        test_case_fail++;
    }

    ret = yunos_task_pri_change(task_misc, YUNOS_IDLE_PRI, &old_pri);

    if (ret == YUNOS_PRI_CHG_NOT_ALLOWED) {
        test_case_success++;
        PRINT_RESULT("yunos_task_pri_change para 3", PASS);
    } else {
        PRINT_RESULT("yunos_task_pri_change para 3", FAIL);
        test_case_fail++;
    }

    task_misc2_stack = yunos_mm_alloc(TASK_TEST_STACK_SIZE);

    if (task_misc2_stack == NULL) {

        PRINT_RESULT("yunos_mm_alloc failed", FAIL);
    }

    yunos_mutex_lock(&mutex, YUNOS_WAIT_FOREVER);

    yunos_task_create(NULL, "task_misc2", NULL, 1,
                      0, task_misc2_stack, TASK_TEST_STACK_SIZE / sizeof(cpu_stack_t),
                      task_misc_entry2, 1);

    yunos_task_create(&task_misc2, NULL, NULL, 1,
                      0, task_misc2_stack, TASK_TEST_STACK_SIZE / sizeof(cpu_stack_t),
                      task_misc_entry2, 1);

    yunos_task_create(&task_misc2, "task_misc2", NULL, 1,
                      0, task_misc2_stack, TASK_TEST_STACK_SIZE / sizeof(cpu_stack_t),
                      NULL, 1);

    yunos_task_create(&task_misc2, "task_misc2", NULL, 1,
                      0, NULL, TASK_TEST_STACK_SIZE / sizeof(cpu_stack_t),
                      task_misc_entry2, 1);

    yunos_task_create(&task_misc2, "task_misc2", NULL, YUNOS_CONFIG_PRI_MAX,
                      0, task_misc2_stack, 0, task_misc_entry2, 1);

    yunos_task_create(&task_misc2, "task_misc2", NULL, YUNOS_CONFIG_PRI_MAX,
                      0, task_misc2_stack, 0, task_misc_entry2, 1);

    yunos_task_create(&task_misc2, "task_misc2", NULL, YUNOS_CONFIG_PRI_MAX,
                      0, task_misc2_stack, TASK_TEST_STACK_SIZE / sizeof(cpu_stack_t),
                      task_misc_entry2, 1);

    yunos_task_create(&task_misc2, "task_misc2", NULL, YUNOS_IDLE_PRI,
                      50, task_misc2_stack, TASK_TEST_STACK_SIZE / sizeof(cpu_stack_t),
                      task_misc_entry2, 1);

    yunos_task_create(&task_misc2, "task_misc2", NULL, 1,
                      50, task_misc2_stack, TASK_TEST_STACK_SIZE / sizeof(cpu_stack_t),
                      task_misc_entry2, 1);

    yunos_task_suspend(&task_misc2);
    yunos_sem_dyn_del(sem2);
    yunos_task_resume(&task_misc2);

    ret = yunos_task_pri_change(&task_misc2, 2, &old_pri);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_pri_change pend", PASS);
    } else {
        PRINT_RESULT("yunos_task_pri_change pend", FAIL);
        test_case_fail++;
    }

    yunos_mutex_unlock(&mutex);

    yunos_task_suspend(&task_misc2);
    yunos_task_suspend(&task_misc2);
    yunos_task_resume(&task_misc2);
    yunos_task_resume(&task_misc2);
    yunos_task_resume(NULL);

    yunos_task_suspend(&task_misc2);
    yunos_sem_give(sem);
    yunos_task_resume(&task_misc2);

    yunos_task_suspend(&task_misc2);
    yunos_task_suspend(&task_misc2);
    yunos_task_resume(&task_misc2);
    yunos_task_resume(&task_misc2);

    yunos_task_dyn_del(&task_misc2);

    yunos_sem_give(sem);
    yunos_task_suspend(&task_misc2);
    yunos_task_suspend(&task_misc2);
    yunos_task_resume(&task_misc2);
    yunos_task_resume(&task_misc2);

    yunos_mutex_lock(&mutex, YUNOS_WAIT_FOREVER);

    yunos_mm_free(task_misc2_stack);

    PRINT_RESULT("yunos_task_del", PASS);

    ret = yunos_task_time_slice_set(NULL, 30);

    if (ret == YUNOS_NULL_PTR) {
        test_case_success++;
        PRINT_RESULT("yunos_task_time_slice_set para 1", PASS);
    } else {
        PRINT_RESULT("yunos_task_time_slice_set para 1", FAIL);
        test_case_fail++;
    }

    ret = yunos_task_time_slice_set(task_misc, 0);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_time_slice_set para 2", PASS);
    } else {
        PRINT_RESULT("yunos_task_time_slice_set para 2", FAIL);
        test_case_fail++;
    }

    ret = yunos_task_time_slice_set(task_misc, 20);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_time_slice_set para 3", PASS);
    } else {
        PRINT_RESULT("yunos_task_time_slice_set para 3", FAIL);
        test_case_fail++;
    }

    yunos_sched_policy_set(task_misc, 0x11);

    ret = yunos_sched_policy_set(task_misc, KSCHED_FIFO);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_sched_policy_set para 1", PASS);
    } else {
        PRINT_RESULT("yunos_sched_policy_set para 1", FAIL);
        test_case_fail++;
    }

    ret = yunos_sched_policy_set(task_misc, KSCHED_RR);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_sched_policy_set para 2", PASS);
    } else {
        PRINT_RESULT("yunos_sched_policy_set para 2", FAIL);
        test_case_fail++;
    }

    ret = yunos_sched_policy_set(NULL, KSCHED_RR);

    if (ret == YUNOS_NULL_PTR) {
        test_case_success++;
        PRINT_RESULT("yunos_sched_policy_set para 3", PASS);
    } else {
        PRINT_RESULT("yunos_sched_policy_set para 3", FAIL);
        test_case_fail++;
    }

    ret = yunos_sched_policy_get(NULL, &policy);

    if (ret == YUNOS_NULL_PTR) {
        test_case_success++;
        PRINT_RESULT("yunos_sched_policy_get para 1", PASS);
    } else {
        PRINT_RESULT("yyunos_sched_policy_get para 1", FAIL);
        test_case_fail++;
    }

    ret = yunos_sched_policy_get(task_misc, NULL);

    if (ret == YUNOS_NULL_PTR) {
        test_case_success++;
        PRINT_RESULT("yunos_sched_policy_get para 2", PASS);
    } else {
        PRINT_RESULT("yunos_sched_policy_get para 2", FAIL);
        test_case_fail++;
    }

    ret = yunos_sched_policy_get(task_misc, &policy);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_sched_policy_get para 3", PASS);
    } else {
        PRINT_RESULT("yunos_sched_policy_get para 3", FAIL);
        test_case_fail++;
    }

#define INFO_IDX (YUNOS_CONFIG_TASK_INFO_NUM - 1)
    ret = yunos_task_info_set(NULL, INFO_IDX, (void *)0x111);

    if (ret == YUNOS_NULL_PTR) {
        test_case_success++;
        PRINT_RESULT("yunos_task_info_set para 2", PASS);
    } else {
        PRINT_RESULT("yunos_task_info_set para 2", FAIL);
        test_case_fail++;
    }

    ret = yunos_task_info_set(task_misc, INFO_IDX, (void *)0x111);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_info_set para 3", PASS);
    } else {
        PRINT_RESULT("yunos_task_info_set para 3", FAIL);
        test_case_fail++;
    }

    ret = yunos_task_info_get(task_misc, INFO_IDX, &info);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_info_get para 1", PASS);
    } else {
        PRINT_RESULT("yunos_task_info_get para 1", FAIL);
        test_case_fail++;
    }

    ret = yunos_task_info_get(NULL, INFO_IDX, &info);

    if (ret == YUNOS_NULL_PTR) {
        test_case_success++;
        PRINT_RESULT("yunos_task_info_get para 2", PASS);
    } else {
        PRINT_RESULT("yunos_task_info_get para 2", FAIL);
        test_case_fail++;
    }

#undef INFO_IDX

    task_free = yunos_global_space_get();

    if (task_free > 0u) {
        test_case_success++;
        PRINT_RESULT("yunos_global_space_get", PASS);
    } else {
        PRINT_RESULT("yunos_global_space_get", FAIL);
        test_case_fail++;
    }

    tmp.blk_state = BLK_ABORT;
#ifndef YUNOS_CONFIG_PERF_NO_PENDEND_PROC
    pend_state_end_proc(&tmp);
#endif
    tmp.blk_state = BLK_TIMEOUT;

#ifndef YUNOS_CONFIG_PERF_NO_PENDEND_PROC
    pend_state_end_proc(&tmp);
#endif

#if (YUNOS_CONFIG_HW_COUNT > 0)
    yunos_overhead_measure();
#endif
    yunos_sem_give(sem3);

    ret = yunos_sem_dyn_create(&sem4, "del", 0);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_sem_dyn_create", PASS);
    } else {
        PRINT_RESULT("yunos_sem_dyn_create", FAIL);
        test_case_fail++;
    }

    ret = yunos_sem_dyn_del(sem4);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_sem_dyn_del", PASS);
    } else {
        PRINT_RESULT("yunos_sem_dyn_del", FAIL);
        test_case_fail++;
    }

    ret = yunos_mutex_dyn_create(&mutex4, "mutex4");

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_mutex_dyn_create", PASS);
    } else {
        PRINT_RESULT("yunos_mutex_dyn_create", FAIL);
        test_case_fail++;
    }

    ret = yunos_mutex_dyn_del(mutex4);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_mutex_dyn_del", PASS);
    } else {
        PRINT_RESULT("yunos_mutex_dyn_del", FAIL);
        test_case_fail++;
    }

    ret = yunos_event_dyn_create(&event4, "event4", 0);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_event_dyn_create", PASS);
    } else {
        PRINT_RESULT("yunos_event_dyn_create", FAIL);
        test_case_fail++;
    }

    ret = yunos_event_dyn_del(event4);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_event_dyn_del", PASS);
    } else {
        PRINT_RESULT("yunos_event_dyn_del", FAIL);
        test_case_fail++;
    }

    ret = yunos_timer_dyn_create(&timer4, "timer4", timer_cb4, 10, 0, 0, 0);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_timer_dyn_create", PASS);
    } else {
        PRINT_RESULT("yunos_timer_dyn_create", FAIL);
        test_case_fail++;
    }

    ret = yunos_timer_dyn_del(timer4);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_timer_dyn_del", PASS);
    } else {
        PRINT_RESULT("yunos_timer_dyn_del", FAIL);
        test_case_fail++;
    }

    ret = yunos_buf_queue_dyn_create(&buf_queue4, "queue4", 100, 20);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_buf_queue_dyn_create", PASS);
    } else {
        PRINT_RESULT("yunos_buf_queue_dyn_create", FAIL);
        test_case_fail++;
    }

    ret = yunos_buf_queue_dyn_del(buf_queue4);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_buf_queue_dyn_del", PASS);
    } else {
        PRINT_RESULT("yunos_buf_queue_dyn_del", FAIL);
        test_case_fail++;
    }

    ret = yunos_queue_dyn_create(&queue4, "queue4", 10);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_queue_dyn_create", PASS);
    } else {
        PRINT_RESULT("yunos_queue_dyn_create", FAIL);
        test_case_fail++;
    }

    ret = yunos_queue_dyn_del(queue4);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_queue_dyn_del", PASS);
    } else {
        PRINT_RESULT("yunos_queue_dyn_del", FAIL);
        test_case_fail++;
    }

    ret = yunos_task_wait_abort(yunos_cur_task_get());

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_wait_abort ready", PASS);
    } else {
        PRINT_RESULT("yunos_task_wait_abort ready", FAIL);
        test_case_fail++;
    }

    yunos_task_dyn_create(&task_misc4, "task_misc_test4", 0, 1,
                          0, TASK_TEST_STACK_SIZE,
                          task_misc_entry4, 1);

    ret = yunos_task_wait_abort(task_misc4);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_wait_abort sleep", PASS);
    } else {
        PRINT_RESULT("yunos_task_wait_abort sleep", FAIL);
        test_case_fail++;
    }

    yunos_task_suspend(task_misc4);
    ret = yunos_task_wait_abort(task_misc4);

    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT("yunos_task_wait_abort suspend sleep", PASS);
    } else {
        PRINT_RESULT("yunos_task_wait_abort suspend sleep", FAIL);
        test_case_fail++;
    }

    next_test_case_notify();
    yunos_task_dyn_del(yunos_cur_task_get());
}

void task_misc_test(void)
{
    yunos_task_dyn_create(NULL, "task_misc_test", 0, 10,
                          0, TASK_TEST_STACK_SIZE,
                          task_misc_entry, 1);


    yunos_task_dyn_create(&task_misc, "task_misc_test", 0, 10,
                          0, TASK_TEST_STACK_SIZE * 10000,
                          task_misc_entry, 1);

    yunos_task_dyn_create(&task_misc, "task_misc_test", 0, 10,
                          0, TASK_TEST_STACK_SIZE,
                          task_misc_entry, 1);

    yunos_task_dyn_create(&task_misc3, "task_misc_test3", 0, 5,
                          0, TASK_TEST_STACK_SIZE,
                          task_misc_entry3, 1);
}

