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

#include <k_api.h>
#include <test_fw.h>

#define TASK_TEST_STACK_SIZE 512

extern ktask_t *task_1_test;
extern ktask_t *task_2_test;


void task_yield_1_entry(void *arg)
{
#if (YUNOS_CONFIG_SCHED_RR > 0)
    yunos_sched_policy_set(yunos_cur_task_get(), KSCHED_FIFO);
#endif

    while (1) {
        yunos_task_yield();
        while (1);
    }

}


void task_yield_2_entry(void *arg)
{
#if (YUNOS_CONFIG_SCHED_RR > 0)
    yunos_sched_policy_set(yunos_cur_task_get(), KSCHED_FIFO);
#endif

    while (1) {
        test_case_success++;
        PRINT_RESULT("task_yield", PASS);
        next_test_case_notify();
        yunos_task_dyn_del(task_1_test);
        yunos_task_dyn_del(yunos_cur_task_get());

    }

}


void task_yield_test(void)
{
    yunos_task_dyn_create(&task_1_test, "task_yield_test_1", 0, 10,
                          0, TASK_TEST_STACK_SIZE,
                          task_yield_1_entry, 1);

    yunos_task_dyn_create(&task_2_test, "task_yield_test_2", 0, 10,
                          0, TASK_TEST_STACK_SIZE,
                          task_yield_2_entry, 1);
}


