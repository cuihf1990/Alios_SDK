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

#if (YUNOS_CONFIG_MM_TLF > 0)

static uint8_t mm_break_case1(void)
{
    void   *ptr, *newptr;
    kstat_t ret;
    char   *ptrarray[10];
    int     i;
    size_t  oldsize;
    ret = yunos_init_mm_head(&pmmhead, (void *)mm_pool, MM_POOL_SIZE);
    MYASSERT(ret == YUNOS_SUCCESS);

#if (K_MM_STATISTIC > 0)

    VGF(VALGRIND_MAKE_MEM_DEFINED(pmmhead, sizeof(k_mm_head)));
    oldsize = pmmhead->used_size;
    ptr = k_mm_alloc(pmmhead, 8);
    MYASSERT(ptr != NULL);

    VGF(VALGRIND_MAKE_MEM_DEFINED(pmmhead, sizeof(k_mm_head)));
    MYASSERT((pmmhead->used_size - oldsize ) == DEF_FIX_BLK_SIZE);

    k_mm_free(pmmhead, ptr);

    VGF(VALGRIND_MAKE_MEM_DEFINED(pmmhead, sizeof(k_mm_head)));
    oldsize = pmmhead->used_size;
    ptr = k_mm_alloc(pmmhead, 32);
    MYASSERT(ptr != NULL);
    VGF(VALGRIND_MAKE_MEM_DEFINED(pmmhead, sizeof(k_mm_head)));
    MYASSERT((pmmhead->used_size - oldsize ) == DEF_FIX_BLK_SIZE);
    k_mm_free(pmmhead, ptr);

    VGF(VALGRIND_MAKE_MEM_DEFINED(pmmhead, sizeof(k_mm_head)));
    oldsize = pmmhead->used_size;
    ptr = k_mm_alloc(pmmhead, DEF_FIX_BLK_SIZE + 1);

    VGF(VALGRIND_MAKE_MEM_DEFINED(pmmhead, sizeof(k_mm_head)));
    MYASSERT(ptr != NULL);
    MYASSERT((pmmhead->used_size - oldsize ) == (DEF_FIX_BLK_SIZE + 4 +
                                                 MMLIST_HEAD_SIZE));
    k_mm_free(pmmhead, ptr);
#endif


    ptr = k_mm_alloc(pmmhead, DEF_FIX_BLK_SIZE / 2);
    MYASSERT(ptr != NULL);

    newptr = k_mm_realloc(pmmhead, ptr, DEF_FIX_BLK_SIZE / 2 + 1 );
    MYASSERT(newptr == ptr);

    newptr = k_mm_realloc(pmmhead, ptr, DEF_FIX_BLK_SIZE - 1);
    MYASSERT(newptr == ptr);

    ptr = newptr;
    newptr = k_mm_realloc(pmmhead, ptr, DEF_FIX_BLK_SIZE + 1);
    MYASSERT(newptr != ptr);

    ptr = newptr;
    newptr = k_mm_realloc(pmmhead, ptr, DEF_FIX_BLK_SIZE + 2);
    MYASSERT(newptr == ptr);

    ptr = newptr;
    newptr = k_mm_realloc(pmmhead, ptr, DEF_FIX_BLK_SIZE * 4);
    MYASSERT(newptr == ptr);

    ptr = newptr;
    newptr = k_mm_realloc(pmmhead, ptr, DEF_FIX_BLK_SIZE * 3);
    MYASSERT(newptr == ptr);

    newptr = k_mm_realloc(pmmhead, ptr, 0);
    MYASSERT(newptr == NULL);

    ptr =  k_mm_realloc(pmmhead, NULL, DEF_FIX_BLK_SIZE * 3);
    MYASSERT(ptr != NULL);

    k_mm_free(pmmhead, ptr);

    for (i = 0; i < 10; i++) {
        ptrarray[i] =  k_mm_alloc(pmmhead, (i + 1) * 32);
        MYASSERT(ptrarray[i]);
    }

    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            k_mm_free(pmmhead, ptrarray[i]);
        }
    }

    for (i = 0; i < 10; i++) {
        if (i % 2 != 0) {
            ptrarray[i] = k_mm_realloc(pmmhead, ptrarray[i], (i + 1) * 96);
        }
        MYASSERT(ptrarray[i]);
    }

    for (i = 0; i < 10; i++) {
        if (i % 2 != 0) {
            k_mm_free(pmmhead, ptrarray[i]);
        }
    }
    yunos_deinit_mm_head(pmmhead);

    return 0;
}

static const test_func_t mm_func_runner[] = {
    mm_break_case1,
    NULL
};

void mm_break_test(void)
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

#endif

