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
#define LOOP_CNT 1

static ktask_t *task_del_test_0;
static ktask_t *task_del_test_1;
static ktask_t *task_del_test_2;

static void task_del_entry_0(void *arg)
{
    while (1) {
        yunos_task_dyn_del(NULL);
        yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);
    }
}

static void task_del_entry_1(void *arg)
{
    while (1) {
        yunos_task_dyn_del(NULL);
        yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);
    }
}

static void task_del_entry_2(void *arg)
{
    while (1) {
        yunos_task_dyn_del(NULL);
        yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);
    }
}

void task_del_test()
{
    test_case_check_err = 0;

    if (yunos_task_dyn_create(&task_del_test_0, "task_del_test0", NULL, 7,
                              0, TASK_TEST_STACK_SIZE,
                              task_del_entry_0, 1) != YUNOS_SUCCESS) {
        test_case_check_err++;
        printf("task_del_test 0 creat fail \n");
    }

    if (yunos_task_dyn_create(&task_del_test_1, "task_del_test1", NULL, 5,
                              0, TASK_TEST_STACK_SIZE,
                              task_del_entry_1, 1) != YUNOS_SUCCESS) {
        test_case_check_err++;
        printf("task_del_test1 creat fail \n");
    }

    if (yunos_task_dyn_create(&task_del_test_2, "task_del_test2", NULL, 59,
                              0, TASK_TEST_STACK_SIZE,
                              task_del_entry_2, 1) != YUNOS_SUCCESS) {
        test_case_check_err++;
        printf("task_del_test2 creat fail \n");
    }

    if (test_case_check_err != 0) {
        test_case_fail ++;
        PRINT_RESULT("task_del", FAIL);
    } else {
        test_case_success++;
        PRINT_RESULT("task_del", PASS);
    }

    next_test_case_notify();
}

