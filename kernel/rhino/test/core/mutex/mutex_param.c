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
#include "mutex_test.h"

#define MODULE_NAME "mutex_param"

static uint8_t mutex_param_case1(void)
{
    kstat_t ret;
    CPSR_ALLOC();

    ret = yunos_mutex_create(NULL, MODULE_NAME);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mutex_create(&test_mutex, NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mutex_create(&test_mutex, MODULE_NAME);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_del(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    YUNOS_CRITICAL_ENTER();
    test_mutex.mm_alloc_flag = K_OBJ_DYN_ALLOC;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_mutex_del(&test_mutex);
    MYASSERT(ret == YUNOS_KOBJ_DEL_ERR);

    YUNOS_CRITICAL_ENTER();
    test_mutex.mm_alloc_flag = K_OBJ_STATIC_ALLOC;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_mutex_del(&test_mutex);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_dyn_create(NULL, MODULE_NAME);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mutex_dyn_create(&test_mutex_dyn, NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mutex_dyn_create(&test_mutex_dyn, MODULE_NAME);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_dyn_del(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    YUNOS_CRITICAL_ENTER();
    test_mutex_dyn->blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_mutex_dyn_del(test_mutex_dyn);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    YUNOS_CRITICAL_ENTER();
    test_mutex_dyn->blk_obj.obj_type = YUNOS_MUTEX_OBJ_TYPE;
    test_mutex_dyn->mm_alloc_flag = K_OBJ_STATIC_ALLOC;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_mutex_dyn_del(test_mutex_dyn);
    MYASSERT(ret == YUNOS_KOBJ_DEL_ERR);

    YUNOS_CRITICAL_ENTER();
    test_mutex_dyn->mm_alloc_flag = K_OBJ_DYN_ALLOC;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_mutex_dyn_del(test_mutex_dyn);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_dyn_create(NULL, MODULE_NAME);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mutex_dyn_create(&test_mutex_dyn, NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mutex_dyn_create(&test_mutex_dyn, MODULE_NAME);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_dyn_del(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    test_mutex_dyn->blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    ret = yunos_mutex_dyn_del(test_mutex_dyn);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    test_mutex_dyn->blk_obj.obj_type = YUNOS_MUTEX_OBJ_TYPE;
    test_mutex_dyn->mm_alloc_flag = K_OBJ_STATIC_ALLOC;
    ret = yunos_mutex_dyn_del(test_mutex_dyn);
    MYASSERT(ret == YUNOS_KOBJ_DEL_ERR);

    test_mutex_dyn->mm_alloc_flag = K_OBJ_DYN_ALLOC;
    ret = yunos_mutex_dyn_del(test_mutex_dyn);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static uint8_t mutex_param_case2(void)
{
    kstat_t     ret;
    CPSR_ALLOC();

    ret = yunos_mutex_create(&test_mutex, MODULE_NAME);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_lock(NULL, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mutex_lock(&test_mutex, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);

    YUNOS_CRITICAL_ENTER();
    test_mutex.owner_nested = -1;
    YUNOS_CRITICAL_EXIT();
    ret = yunos_mutex_lock(&test_mutex, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_MUTEX_NESTED_OVF);

    YUNOS_CRITICAL_ENTER();
    test_mutex.owner_nested = 1;
    YUNOS_CRITICAL_EXIT();

    ret = yunos_mutex_unlock(NULL);
    MYASSERT(ret == YUNOS_NULL_PTR);

    ret = yunos_mutex_unlock(&test_mutex);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_del(&test_mutex);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t mutex_func_runner[] = {
    mutex_param_case1,
    mutex_param_case2,
    NULL
};

void mutex_param_test(void)
{
    kstat_t ret;

    task_mutex_entry_register(MODULE_NAME, (test_func_t *)mutex_func_runner,
                              sizeof(mutex_func_runner) / sizeof(test_case_t));

    ret = yunos_task_dyn_create(&task_mutex, MODULE_NAME, 0, TASK_MUTEX_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mutex_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

