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

#if (YUNOS_CONFIG_MM_TLF > 0)

static uint8_t mm_param_case1(void)
{
    kstat_t ret;

    ret = yunos_init_mm_head(NULL, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_init_mm_head(&pmmhead, NULL, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_init_mm_head(&pmmhead, (void *)mm_pool, MIN_FREE_MEMORY_SIZE + DEF_TOTAL_FIXEDBLK_SIZE-4);
    MYASSERT(ret == YUNOS_MM_POOL_SIZE_ERR);

    ret = yunos_init_mm_head(&pmmhead, (void *)(mm_pool+1), MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_INV_ALIGN);

    ret = yunos_init_mm_head(&pmmhead, (void *)mm_pool, MM_POOL_SIZE-1);
    MYASSERT(ret == YUNOS_INV_ALIGN);

    ret = yunos_init_mm_head(&pmmhead, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_add_mm_region(NULL, (void *)&mm_pool[MM_POOL_SIZE], MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_add_mm_region(pmmhead, NULL, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_add_mm_region(pmmhead, (void *)&mm_pool[MM_POOL_SIZE], 4);
    MYASSERT(ret == YUNOS_MM_POOL_SIZE_ERR);

    ret = yunos_add_mm_region(pmmhead, (void *)(&mm_pool[MM_POOL_SIZE]+1), MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_INV_ALIGN);

    ret = yunos_add_mm_region(pmmhead, (void *)&mm_pool[MM_POOL_SIZE], MM_POOL_SIZE-1);
    MYASSERT(ret == YUNOS_INV_ALIGN);

    ret = yunos_add_mm_region(pmmhead, (void *)&mm_pool[MM_POOL_SIZE], MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_add_mm_region(pmmhead, (void *)&mm_pool[MM_POOL_SIZE*3], MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_add_mm_region(pmmhead, (void *)&mm_pool[MM_POOL_SIZE*2], MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t mm_param_case2(void)
{
    void   *ptr;
    void   *tmp;

    ptr = k_mm_alloc( NULL,64);
    MYASSERT(ptr == NULL);

    ptr = k_mm_alloc(pmmhead, 0);
    MYASSERT(ptr == NULL);

    ptr = k_mm_alloc(pmmhead, 64);
    MYASSERT((ptr > (void*)mm_pool && ptr < (void *)mm_pool + MM_POOL_SIZE)
        ||(ptr > (void*)&mm_pool[MM_POOL_SIZE] && ptr < (void *)&mm_pool[MM_POOL_SIZE*2] ));

    k_mm_free(pmmhead, ptr);

    ptr = k_mm_alloc(pmmhead, 16);
    tmp = pmmhead->fixedmblk->mbinfo.buffer;
    MYASSERT((ptr > (void*)pmmhead->fixedmblk->mbinfo.buffer) && (ptr < ((void *)tmp + (pmmhead->fixedmblk->size & YUNOS_MM_BLKSIZE_MASK))));

    k_mm_free(pmmhead, ptr);

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

#endif

