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

#define MODULE_NAME    "mm_opr"
#define MODULE_NAME_CO "mm_coopr"
static void *co_ptr;

#if (YUNOS_CONFIG_MM_TLF > 0)

static uint8_t mm_opr_case1(void)
{
    void   *ptr;
    kstat_t ret;

    ret = yunos_init_mm_head(&pmmhead, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    ptr = k_mm_alloc(pmmhead, 64);
    MYASSERT(ptr != NULL);

    k_mm_free(pmmhead, ptr);

    ret = yunos_init_mm_head(&pmmhead, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t mm_opr_case2(void)
{
    void   *r_ptr[16];
    int8_t  cnt;
    kstat_t ret;

    ret = yunos_init_mm_head(&pmmhead, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    /* alloc out of mm pools then free all */
    cnt = 0;
    for (cnt = 0; cnt < 16; ++cnt) {
        r_ptr[cnt] = k_mm_alloc(pmmhead, 1023);
        if (r_ptr[cnt] ==  NULL) {
            break;
        }
    }

    cnt--;
    do {
        k_mm_free(pmmhead, r_ptr[--cnt]);

    } while (cnt > 0);

    return 0;
}

static const test_func_t mm_func_runner[] = {
    mm_opr_case1,
    mm_opr_case2,
    NULL
};

void mm_opr_test(void)
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

static void task_mm_co1_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_init_mm_head(&pmmhead, (void *)mm_pool, MM_POOL_SIZE);
    if (ret != YUNOS_SUCCESS) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO, FAIL);
        return;
    }

    while (1) {
        co_ptr = k_mm_alloc(pmmhead, 16);
        yunos_task_sleep(5);
        co_ptr = k_mm_alloc(pmmhead, 18);
        yunos_task_sleep(5);
        co_ptr = k_mm_alloc(pmmhead, 32);
        yunos_task_sleep(5);
        co_ptr = k_mm_alloc(pmmhead, 65);
        yunos_task_sleep(5);
        co_ptr = k_mm_alloc(pmmhead, 178);
        yunos_task_sleep(5);
        co_ptr = k_mm_alloc(pmmhead, 333);
        yunos_task_sleep(5);
        break;
    }

    yunos_task_dyn_del(g_active_task);
}

static void task_mm_co2_entry(void *arg)
{
    while (1) {
        k_mm_free(pmmhead, co_ptr);
        yunos_task_sleep(5);
        k_mm_free(pmmhead, co_ptr);
        yunos_task_sleep(5);
        k_mm_free(pmmhead, co_ptr);
        yunos_task_sleep(5);
        k_mm_free(pmmhead, co_ptr);
        yunos_task_sleep(5);
        k_mm_free(pmmhead, co_ptr);
        yunos_task_sleep(5);
        k_mm_free(pmmhead, co_ptr);
        yunos_task_sleep(5);
        break;
    }

    test_case_success++;
    PRINT_RESULT(MODULE_NAME_CO, PASS);

    next_test_case_notify();
    yunos_task_dyn_del(g_active_task);
}

void mm_coopr_test(void)
{
    kstat_t ret;

    ret = yunos_task_dyn_create(&task_mm, MODULE_NAME, 0, TASK_MM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mm_co1_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO, FAIL);
    }

    ret = yunos_task_dyn_create(&task_mm_co, MODULE_NAME, 0, TASK_MM_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_mm_co2_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO, FAIL);
    }
}

#endif

