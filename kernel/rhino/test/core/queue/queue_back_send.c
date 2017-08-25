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

#include <stdio.h>
#include <k_api.h>
#include <test_fw.h>

#include "queue_test.h"

#define TEST_QUEUE_MSG0_SIZE 30

static ktask_t   *task_0_test;
static ktask_t   *task_1_test;
static void     *queue_send_msg = (void *)1;
static void     *queue_recv_msg = (void *)1;
static kqueue_t *g_test_queue0;

static void queue_back_send_param_test(void)
{
    kstat_t ret;
    ksem_t  sem;

    ret = yunos_queue_back_send(NULL, queue_send_msg);
    QUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);

    yunos_sem_create(&sem, "test_sem ", 0);
    ret = yunos_queue_back_send((kqueue_t *)&sem, queue_send_msg);
    QUEUE_VAL_CHK(ret == YUNOS_KOBJ_TYPE_ERR);
    yunos_sem_del(&sem);
}

static void yunos_queue_dyn_create_param_test(void)
{
    kstat_t ret;

    ret = yunos_queue_dyn_create(NULL, "test_queue0", TEST_QUEUE_MSG0_SIZE);
    QUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);
}

static void yunos_queue_dyn_del_param_test(void)
{
    kstat_t ret;
    ksem_t  sem;

    ret = yunos_queue_dyn_del(NULL);
    QUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);

    yunos_intrpt_enter();
    ret = yunos_queue_dyn_del(g_test_queue0);
    QUEUE_VAL_CHK(ret == YUNOS_NOT_CALLED_BY_INTRPT);
    yunos_intrpt_exit();

    ret = yunos_queue_del(g_test_queue0);
    QUEUE_VAL_CHK(ret == YUNOS_KOBJ_DEL_ERR);

    yunos_sem_create(&sem, "test_sem ", 0);
    ret = yunos_queue_dyn_del((kqueue_t *)&sem);
    QUEUE_VAL_CHK(ret == YUNOS_KOBJ_TYPE_ERR);
    yunos_sem_del(&sem);
}

static void task_queue0_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        /* check yunos_queue_back_send param */
        queue_back_send_param_test();

        /* check yunos_queue_back_send */
        ret = yunos_queue_recv(g_test_queue0, YUNOS_WAIT_FOREVER, &queue_recv_msg);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        if (queue_recv_msg == queue_send_msg) {
            test_case_success++;
            PRINT_RESULT("queue back send", PASS);
        } else {
            test_case_fail++;
            PRINT_RESULT("queue back send", FAIL);
        }

        yunos_queue_dyn_del_param_test();
        ret = yunos_queue_dyn_del(g_test_queue0);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);
        next_test_case_notify();
        yunos_task_dyn_del(task_0_test);
    }
}

static void task_queue1_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        /* check yunos_queue_back_send */
        ret = yunos_queue_back_send(g_test_queue0, queue_send_msg);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        yunos_task_dyn_del(task_1_test);
    }
}

kstat_t task_queue_back_send_test(void)
{
    kstat_t ret;

    yunos_queue_dyn_create_param_test();

    ret = yunos_queue_dyn_create(&g_test_queue0, "test_queue0",
                                 TEST_QUEUE_MSG0_SIZE);
    QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

    ret = yunos_task_dyn_create(&task_0_test, "task_queue0_test", 0, 10,
                                0, TASK_TEST_STACK_SIZE, task_queue0_entry, 1);
    QUEUE_VAL_CHK((ret == YUNOS_SUCCESS) || (ret == YUNOS_STOPPED));

    ret = yunos_task_dyn_create(&task_1_test, "task_queue1_test", 0, 11,
                                0, TASK_TEST_STACK_SIZE, task_queue1_entry, 1);
    QUEUE_VAL_CHK((ret == YUNOS_SUCCESS) || (ret == YUNOS_STOPPED));

    return 0;
}

