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

#define MODULE_NAME "mm_break"

static uint8_t mm_break_case1(void)
{
    void   *ptr;
    kstat_t ret;

    ret = yunos_mm_pool_init(&mm_pool_test, MODULE_NAME, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(mm_pool_test.obj_type == YUNOS_MM_OBJ_TYPE);

    /* check mm pool object type after change it */
    mm_pool_test.obj_type = YUNOS_MM_BLK_OBJ_TYPE;
    ret = yunos_mm_alloc(&mm_pool_test, &ptr, 8);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    /* check mm pool object type after recover it */
    mm_pool_test.obj_type = YUNOS_MM_OBJ_TYPE;
    ret = yunos_mm_alloc(&mm_pool_test, &ptr, 8);
    MYASSERT(ret == YUNOS_SUCCESS);

    /* check mm pool object type after change it */
    mm_pool_test.obj_type = YUNOS_MM_BLK_OBJ_TYPE;
    ret = yunos_mm_free(&mm_pool_test, ptr);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    /* check mm pool object type after recover it */
    mm_pool_test.obj_type = YUNOS_MM_OBJ_TYPE;
    ret = yunos_mm_free(&mm_pool_test, ptr);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t mm_func_runner[] = {
    mm_break_case1,
    NULL
};

void mm_break_test(void)
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

