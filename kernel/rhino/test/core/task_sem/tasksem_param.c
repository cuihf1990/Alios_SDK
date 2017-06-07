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
#include "tasksem_test.h"

#define MODULE_NAME "tasksem_param"

static uint8_t tasksem_param_case1(void)
{
    kstat_t ret;

    ret = yunos_task_sem_create(NULL, &test_tasksem, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_task_sem_create(task_tasksem, NULL, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_task_sem_create(task_tasksem, &test_tasksem, NULL, 0);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_task_sem_create(task_tasksem, &test_tasksem, MODULE_NAME,
                                (sem_count_t) - 1);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_task_sem_create(task_tasksem, &test_tasksem, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_task_sem_del(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    return 0;
}

static uint8_t tasksem_param_case2(void)
{
    kstat_t     ret;
    sem_count_t count;

    ret = yunos_task_sem_create(task_tasksem, &test_tasksem, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_task_sem_count_get(NULL, &count);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_task_sem_count_get(task_tasksem, NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_task_sem_count_set(NULL, 3);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_task_sem_count_set(task_tasksem, (sem_count_t) - 1);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}


static const test_func_t tasksem_func_runner[] = {
    tasksem_param_case1,
    tasksem_param_case2,
    NULL
};

void tasksem_param_test(void)
{
    kstat_t ret;

    task_tasksem_entry_register(MODULE_NAME, (test_func_t *)tasksem_func_runner,
                                sizeof(tasksem_func_runner) / sizeof(test_case_t));

    ret = yunos_task_dyn_create(&task_tasksem, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_tasksem_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

