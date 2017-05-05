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

#include "buf_queue_test.h"

#define TEST_BUFQUEUE_MSG0_SIZE 8
#define TEST_BUFQUEUE_BUF0_SIZE 16
#define TEST_BUFQUEUE_MSG_MAX   8

static ktask_t     *task_0_test;
static char         g_test_send_msg0[TEST_BUFQUEUE_MSG0_SIZE]     = {0};
static char         g_test_bufqueue_buf0[TEST_BUFQUEUE_MSG0_SIZE] = {0};
static kbuf_queue_t g_test_bufqueue0;

static void buf_queue_flush_param_test(void)
{
    kstat_t ret;

    ksem_t  sem;

    ret = yunos_buf_queue_flush(NULL);
    BUFQUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);

    yunos_intrpt_enter();
    ret = yunos_buf_queue_flush(&g_test_bufqueue0);
    BUFQUEUE_VAL_CHK(ret == YUNOS_NOT_CALLED_BY_INTRPT);
    yunos_intrpt_exit();

    yunos_sem_create(& sem, "test_sem ", 0);
    ret = yunos_buf_queue_flush((kbuf_queue_t *)&sem);
    BUFQUEUE_VAL_CHK(ret == YUNOS_KOBJ_TYPE_ERR);
    yunos_sem_del(&sem);
}

static void task_queue0_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        ret = yunos_buf_queue_create(&g_test_bufqueue0, "test_bufqueue0",
                                     g_test_bufqueue_buf0,
                                     TEST_BUFQUEUE_BUF0_SIZE, TEST_BUFQUEUE_MSG_MAX);
        BUFQUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        /* check yunos_buf_queue_flush param */
        buf_queue_flush_param_test();

        ret = yunos_buf_queue_send(&g_test_bufqueue0, g_test_send_msg0, TEST_BUFQUEUE_MSG_MAX);
        BUFQUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        ret = yunos_buf_queue_send(&g_test_bufqueue0, g_test_send_msg0, TEST_BUFQUEUE_MSG_MAX);
        BUFQUEUE_VAL_CHK(ret == YUNOS_BUF_QUEUE_FULL);

        ret = yunos_buf_queue_flush(&g_test_bufqueue0);
        BUFQUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        ret = yunos_buf_queue_send(&g_test_bufqueue0, g_test_send_msg0, TEST_BUFQUEUE_MSG_MAX);
        BUFQUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        ret = yunos_buf_queue_del(&g_test_bufqueue0);
        BUFQUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        if (test_case_check_err == 0) {
            test_case_success++;
            PRINT_RESULT("buf queue flush", PASS);
        } else {
            test_case_check_err = 0;
            test_case_fail++;
            PRINT_RESULT("buf queue flush", FAIL);
        }

        next_test_case_notify();
        yunos_task_dyn_del(task_0_test);
    }
}

kstat_t task_buf_queue_flush_test(void)
{
    kstat_t ret;

    ret = yunos_task_dyn_create(&task_0_test, "task_bufqueue0_test", 0, 10,
                                0, TASK_TEST_STACK_SIZE, task_queue0_entry, 1);
    BUFQUEUE_VAL_CHK((ret == YUNOS_SUCCESS) || (ret == YUNOS_STOPPED));

    return 0;
}

