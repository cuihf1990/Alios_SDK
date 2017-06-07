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
#include "sem_test.h"

#define MODULE_NAME "sem_break"

static uint8_t sem_break_case1(void)
{
    kstat_t ret;

    ret = yunos_sem_dyn_create(&test_sem, MODULE_NAME, 0);
    MYASSERT(ret == YUNOS_SUCCESS);

    /* try to delete after change it */
    test_sem->blk_obj.obj_type = YUNOS_EVENT_OBJ_TYPE;
    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == YUNOS_KOBJ_TYPE_ERR);

    /* try to delete after recover it */
    test_sem->blk_obj.obj_type = YUNOS_SEM_OBJ_TYPE;
    ret = yunos_sem_dyn_del(test_sem);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t sem_func_runner[] = {
    sem_break_case1,
    NULL
};

void sem_break_test(void)
{
    kstat_t ret;

    task_sem_entry_register(MODULE_NAME, (test_func_t *)sem_func_runner,
                            sizeof(sem_func_runner) / sizeof(test_case_t));

    ret = yunos_task_dyn_create(&task_sem, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_sem_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

