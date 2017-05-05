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
#include "comb_test.h"

static ktask_t *task_sem;
static ktask_t *task_mutex;
static ksem_t  *sem_comb;
static kmutex_t mutex_comb;

#define MODULE_NAME    "sem_mutex_opr"

static void task_sem_opr_entry(void *arg)
{
    kstat_t ret;

    yunos_sem_dyn_create(&sem_comb, "semtest", 0);
    yunos_mutex_create(&mutex_comb, "mutex");

    ret = yunos_sem_take(sem_comb, YUNOS_WAIT_FOREVER);
    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT(MODULE_NAME, PASS);
    } else {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    next_test_case_notify();
    yunos_sem_dyn_del(sem_comb);
    yunos_mutex_del(&mutex_comb);
    yunos_task_dyn_del(g_active_task);
}

static void task_mutex_opr_entry(void *arg)
{
    yunos_mutex_lock(&mutex_comb, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb, YUNOS_WAIT_FOREVER);

    yunos_mutex_unlock(&mutex_comb);
    yunos_mutex_unlock(&mutex_comb);
    yunos_mutex_unlock(&mutex_comb);
    yunos_mutex_unlock(&mutex_comb);
    yunos_mutex_unlock(&mutex_comb);
    yunos_mutex_unlock(&mutex_comb);

    yunos_sem_give(sem_comb);
    yunos_task_dyn_del(g_active_task);
}

void sem_mutex_coopr_test(void)
{
    kstat_t ret;

    ret = yunos_task_dyn_create(&task_sem, MODULE_NAME, 0, TASK_COMB_PRI,
                                 0, TASK_TEST_STACK_SIZE, task_sem_opr_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_mutex, MODULE_NAME, 0, TASK_COMB_PRI+1,
                                 0, TASK_TEST_STACK_SIZE, task_mutex_opr_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

