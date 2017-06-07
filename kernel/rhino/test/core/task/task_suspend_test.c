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

ktask_t *task_1_test;
ktask_t *task_2_test;

void task_suspend_entry(void *arg)
{
    while (1) {
        yunos_task_suspend(g_active_task);

        test_case_success++;
        PRINT_RESULT("task_suspend", PASS);

        next_test_case_notify();
        yunos_task_dyn_del(g_active_task);
    }
}

void task_resume_entry(void *arg)
{
    while (1) {
        yunos_task_resume(task_1_test);
        yunos_task_dyn_del(g_active_task);
    }
}

void task_suspend_test(void)
{
    yunos_task_dyn_create(&task_1_test, "task_suspend_test_1", 0, 10,
                          0, TASK_TEST_STACK_SIZE,
                          task_suspend_entry, 1);

    yunos_task_dyn_create(&task_2_test, "task_suspend_test_2", 0, 11,
                          0, TASK_TEST_STACK_SIZE,
                          task_resume_entry, 1);

}

