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
#include "mm_blk_test.h"

#define MODULE_NAME    "mm_blk_opr"
#define MODULE_NAME_CO "mm_blk_coopr"

static void *co_ptr;

static uint8_t mm_blk_opr_case1(void)
{
    void   *ptr;
    kstat_t ret;

    ret = yunos_mblk_pool_init(&mblk_pool_test, MODULE_NAME, (void *)mblk_pool,
                               MBLK_POOL_SIZE >> 2, MBLK_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mblk_alloc(&mblk_pool_test, &ptr);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mblk_free(&mblk_pool_test, ptr);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t mm_blk_func_runner[] = {
    mm_blk_opr_case1,
    NULL
};

void mm_blk_opr_test(void)
{
    kstat_t ret;

    task_mm_blk_entry_register(MODULE_NAME, (test_func_t *)mm_blk_func_runner,
                               sizeof(mm_blk_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_mm_blk, MODULE_NAME, 0, TASK_MM_BLK_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mm_blk_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

static void task_mm_blk_co1_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_mblk_pool_init(&mblk_pool_test, MODULE_NAME, (void *)mblk_pool,
                               MBLK_POOL_SIZE >> 2, MBLK_POOL_SIZE);
    if (ret != YUNOS_SUCCESS) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO, FAIL);
        return;
    }

    while (1) {
        yunos_mblk_alloc(&mblk_pool_test, &co_ptr);
        yunos_task_sleep(5);
        yunos_mblk_alloc(&mblk_pool_test, &co_ptr);
        yunos_task_sleep(5);
        yunos_mblk_alloc(&mblk_pool_test, &co_ptr);
        yunos_task_sleep(5);
        yunos_mblk_alloc(&mblk_pool_test, &co_ptr);
        yunos_task_sleep(5);

        break;
    }

    yunos_task_dyn_del(g_active_task);
}

static void task_mm_blk_co2_entry(void *arg)
{
    while (1) {
        yunos_mblk_free(&mblk_pool_test, co_ptr);
        yunos_task_sleep(5);
        yunos_mblk_free(&mblk_pool_test, co_ptr);
        yunos_task_sleep(5);
        yunos_mblk_free(&mblk_pool_test, co_ptr);
        yunos_task_sleep(5);
        yunos_mblk_free(&mblk_pool_test, co_ptr);
        yunos_task_sleep(5);

        break;
    }

    if (mblk_pool_test.blk_avail == (MBLK_POOL_SIZE / mblk_pool_test.blk_size)) {
        test_case_success++;
        PRINT_RESULT(MODULE_NAME_CO, PASS);
    } else {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO, FAIL);
    }

    next_test_case_notify();
    yunos_task_dyn_del(g_active_task);
}


void mm_blk_coopr_test(void)
{
    kstat_t ret;

    ret = yunos_task_dyn_create(&task_mm_blk, MODULE_NAME, 0, TASK_MM_BLK_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mm_blk_co1_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO, FAIL);
    }

    ret = yunos_task_dyn_create(&task_mm_blk_co, MODULE_NAME, 0,
                                TASK_MM_BLK_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_mm_blk_co2_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO, FAIL);
    }
}

