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

#define MODULE_NAME "mm_param"

static uint8_t mm_param_case1(void)
{
    kstat_t ret;

    ret = yunos_mm_pool_init(NULL, MODULE_NAME, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_pool_init(&mm_pool_test, NULL, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_pool_init(&mm_pool_test, MODULE_NAME, NULL, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_pool_init(&mm_pool_test, MODULE_NAME, (void *)mm_pool, MM_POOL_MIN_SIZE-4);
    MYASSERT(ret == YUNOS_MM_POOL_SIZE_ERR);

    ret = yunos_mm_pool_init(&mm_pool_test, MODULE_NAME, (void *)(mm_pool+1), MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_INV_ALIGN);

    ret = yunos_mm_pool_init(&mm_pool_test, MODULE_NAME, (void *)mm_pool, MM_POOL_SIZE+1);
    MYASSERT(ret == YUNOS_INV_ALIGN);

    return 0;
}

static uint8_t mm_param_case2(void)
{
    kstat_t ret;
    void   *ptr;

    ret = yunos_mm_pool_init(&mm_pool_test, MODULE_NAME, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mm_alloc(NULL, &ptr, 64);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_alloc(&mm_pool_test, NULL, 64);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_alloc(&mm_pool_test, &ptr, 0);
    MYASSERT(ret == YUNOS_MM_ALLOC_SIZE_ERR);

    ret = yunos_mm_free(NULL, ptr);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mm_free(&mm_pool_test, NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    return 0;
}

static const test_func_t mm_func_runner[] = {
    mm_param_case1,
    mm_param_case2,
    NULL
};

void mm_param_test(void)
{
    kstat_t ret;

    task_mm_entry_register(MODULE_NAME, (test_func_t *)mm_func_runner, sizeof(mm_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_mm, MODULE_NAME, 0, TASK_MM_PRI,
                                 0, TASK_TEST_STACK_SIZE, task_mm_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

