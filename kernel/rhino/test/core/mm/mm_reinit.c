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
#include "mm_test.h"

#define MODULE_NAME "mm_reinit"

static uint8_t mm_reinit_case1(void)
{
    kstat_t ret;

    ret = yunos_mm_pool_init(&mm_pool_test, MODULE_NAME, (void *)mm_pool,
                             MM_POOL_MIN_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(mm_pool_test.fragments == 2);
    MYASSERT(mm_pool_test.pool_name != NULL);
    MYASSERT(mm_pool_test.avail == (MM_POOL_MIN_SIZE - (MM_HEAD_SIZE << 1)));

    ret = yunos_mm_pool_init(&mm_pool_test, MODULE_NAME, (void *)mm_pool,
                             MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(mm_pool_test.fragments == 2);
    MYASSERT(mm_pool_test.pool_name != NULL);
    MYASSERT(mm_pool_test.avail == (MM_POOL_SIZE - (MM_HEAD_SIZE << 1)));

    return 0;
}

static const test_func_t mm_func_runner[] = {
    mm_reinit_case1,
    NULL
};

void mm_reinit_test(void)
{
    kstat_t ret;

    task_mm_entry_register(MODULE_NAME, (test_func_t *)mm_func_runner,
                           sizeof(mm_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_mm, MODULE_NAME, 0, TASK_MM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mm_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

