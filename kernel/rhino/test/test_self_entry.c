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

#define      TEST_CASE_TASK_STACK_SIZE 512
ktask_t       test_case_task;
cpu_stack_t  test_case_task_stack[TEST_CASE_TASK_STACK_SIZE];


#define      TEST_MTBF_TASK_STACK_SIZE 512
ktask_t       test_mtbf_task;
cpu_stack_t  test_mtbf_task_stack[TEST_MTBF_TASK_STACK_SIZE];

void test_case_task_entry(void *arg)
{
#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
    yunos_cpu_usage_stats_init();
#endif
    test_case_init();

    int item;

    for (item = 0 ;; item++) {
        if (test_fw_map[item].fn == NULL) {
            break;
        }

        (*test_fw_map[item].fn)();
    }

    printf("all test finished, successed test case %d, failed test case %d\n",
           (int)test_case_success, (int)test_case_fail);

    while (1) {
        yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);
    }
}

void test_case_task_start(void)
{
    yunos_task_create(&test_case_task, "test_case_task", NULL, 5,
                      0, test_case_task_stack, TEST_CASE_TASK_STACK_SIZE,
                      test_case_task_entry, 1);
}

void test_mtbf_task_entry(void *arg)
{
#if (YUNOS_CONFIG_CPU_USAGE_STATS > 0)
    yunos_cpu_usage_stats_init();
#endif
    test_case_init();
    int counter = 0;

    int item;

    while (1) {
        for (item = 0 ; ; item++) {
            if (test_fw_map[item].fn == NULL) {
                break;
            }

            (*test_fw_map[item].fn)();
        }

        counter ++;

        if (counter > 1000)        {
            break;
        }
    }

    printf("all test finished, successed test case %d, failed test case %d\n",
           (int)test_case_success, (int)test_case_fail);

    while (1) {
        yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);
    }
}

void test_mtbf_task_start(void)
{
    yunos_task_create(&test_mtbf_task, "test_mtbf_task", NULL, 9,
                      0, test_mtbf_task_stack, TEST_MTBF_TASK_STACK_SIZE,
                      test_mtbf_task_entry, 1);
}

