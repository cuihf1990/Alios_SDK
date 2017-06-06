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
#include "comb_test.h"

static ktask_t *task_mutex;
static ktask_t *task_buf_queue;
static ktask_t *task_buf_queue_trigger;

static kmutex_t     *test_mutex;
static kbuf_queue_t test_buf_queue;
static uint8_t buf_queue_test_buf[1];
static uint8_t buf_queue_recv[1];
static uint8_t buf_queue_send[1];

static uint8_t notify_flag;

#define MODULE_NAME    "mutex_queue_buf_opr"

static void task_mutex_opr_entry(void *arg)
{
    kstat_t ret;

    yunos_mutex_dyn_create(&test_mutex, "mutextest");

    yunos_mutex_lock(test_mutex, YUNOS_WAIT_FOREVER);

    while (notify_flag != 0x5a) {
        yunos_task_sleep(5);
    }

    ret = yunos_mutex_unlock(test_mutex);
    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT(MODULE_NAME, PASS);
    } else {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    next_test_case_notify();
    ret = yunos_mutex_dyn_del(test_mutex);
    if (ret != YUNOS_SUCCESS) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    yunos_task_dyn_del(g_active_task);
}

static void task_buf_queue_entry(void *arg)
{
    kstat_t ret;
    size_t  size;

    yunos_buf_queue_create(&test_buf_queue, "bugqueue", (void *)buf_queue_test_buf,
                           8, 1);

    ret = yunos_buf_queue_recv(&test_buf_queue, YUNOS_WAIT_FOREVER,
                               (void *)buf_queue_recv, &size);
    if ((ret == YUNOS_SUCCESS) && (*(uint8_t *)buf_queue_recv == 0x5a)) {
        notify_flag = 0x5a;
        yunos_buf_queue_del(&test_buf_queue);
        yunos_task_dyn_del(g_active_task);
    }
}

static void task_buf_queue_trigger_entry(void *arg)
{
    *(uint8_t *)buf_queue_send = 0x5a;

    yunos_buf_queue_send(&test_buf_queue, (void *)buf_queue_send, 1);
    yunos_task_dyn_del(g_active_task);
}

void mutex_buf_queue_coopr_test(void)
{
    kstat_t ret;

    ret = yunos_task_dyn_create(&task_mutex, MODULE_NAME, 0, TASK_COMB_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mutex_opr_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_buf_queue, MODULE_NAME, 0, TASK_COMB_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_buf_queue_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_buf_queue_trigger, MODULE_NAME, 0,
                                TASK_COMB_PRI + 2,
                                0, TASK_TEST_STACK_SIZE, task_buf_queue_trigger_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

