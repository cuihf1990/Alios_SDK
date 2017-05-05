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

#define MODULE_NAME     "mutex_opr"
#define MODULE_NAME_CO1 "mutex_coopr1"
#define MODULE_NAME_CO2 "mutex_coopr2"

static uint8_t mutex_opr_case1(void)
{
    kstat_t ret;
    uint8_t old_pri;

    ret = yunos_mutex_create(&test_mutex, MODULE_NAME);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_unlock(&test_mutex);
    MYASSERT(ret == YUNOS_MUTEX_NOT_RELEASED_BY_OWNER);

    ret = yunos_mutex_lock(&test_mutex, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);

    yunos_task_pri_change(g_active_task, TASK_MUTEX_PRI, &old_pri);

    ret = yunos_mutex_lock(&test_mutex, YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_MUTEX_OWNER_NESTED);

    ret = yunos_mutex_unlock(&test_mutex);
    MYASSERT(ret == YUNOS_MUTEX_OWNER_NESTED);

    ret = yunos_mutex_unlock(&test_mutex);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_mutex_del(&test_mutex);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t mutex_func_runner[] = {
    mutex_opr_case1,
    NULL
};

void mutex_opr_test(void)
{
    kstat_t ret;

    task_mutex_entry_register(MODULE_NAME, (test_func_t *)mutex_func_runner, sizeof(mutex_func_runner)/sizeof(test_case_t));

    ret = yunos_task_dyn_create(&task_mutex, MODULE_NAME, 0, TASK_MUTEX_PRI,
                                 0, TASK_TEST_STACK_SIZE, task_mutex_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

static uint8_t task_pri_get(ktask_t *task)
{
    CPSR_ALLOC();

    uint8_t pri;

    YUNOS_CRITICAL_ENTER();

    pri = task->prio;

    YUNOS_CRITICAL_EXIT();

    return pri;
}

void task_mutex_coopr1_co1_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        yunos_task_sleep(5);

        yunos_sched_disable();
        ret = yunos_mutex_lock(&test_mutex, YUNOS_WAIT_FOREVER);
        if(ret != YUNOS_SCHED_DISABLE) {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO1, FAIL);
        }
        yunos_sched_enable();

        ret = yunos_mutex_lock(&test_mutex, YUNOS_NO_WAIT);
        if(ret != YUNOS_NO_PEND_WAIT) {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO1, FAIL);
        }

        ret = yunos_mutex_lock(&test_mutex, YUNOS_WAIT_FOREVER);
        if(ret != YUNOS_SUCCESS) {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO1, FAIL);
        }

        yunos_mutex_unlock(&test_mutex);

        break;
    }

    yunos_task_dyn_del(g_active_task);
}

void task_mutex_coopr1_co2_entry(void *arg)
{
    uint8_t pri;

    yunos_mutex_create(&test_mutex, MODULE_NAME_CO1);

    while (1) {
        yunos_mutex_lock(&test_mutex, YUNOS_WAIT_FOREVER);
        yunos_task_sleep(10);

        /* now, the task's priority is revert */
        pri = task_pri_get(g_active_task);
        if (pri != TASK_MUTEX_PRI) {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO1, FAIL);
            yunos_mutex_del(&test_mutex);

            next_test_case_notify();
            yunos_task_dyn_del(g_active_task);

            return;
        } else {
            yunos_task_dyn_del(task_mutex_co1);
            yunos_mutex_unlock(&test_mutex);
            break;
        }
    }

    test_case_success++;
    PRINT_RESULT(MODULE_NAME_CO1, PASS);

    yunos_mutex_del(&test_mutex);

    next_test_case_notify();
    yunos_task_dyn_del(g_active_task);
}

/* the case is to test a mutex task owner 's priority revert in case of another higher
 * priority task try to get mutex */
void mutex_coopr1_test(void)
{
    kstat_t ret;

    ret = yunos_task_dyn_create(&task_mutex_co1, MODULE_NAME_CO1, 0, TASK_MUTEX_PRI,
                                 0, TASK_TEST_STACK_SIZE, task_mutex_coopr1_co1_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_mutex_co2, MODULE_NAME_CO1, 0, TASK_MUTEX_PRI+1,
                                 0, TASK_TEST_STACK_SIZE, task_mutex_coopr1_co2_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

static void task_mutex_coopr2_co1_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        yunos_task_sleep(10);

        ret = yunos_mutex_lock(&test_mutex_co1, YUNOS_WAIT_FOREVER);
        if(ret != YUNOS_SUCCESS) {
            break;
        }

        yunos_mutex_unlock(&test_mutex_co1);

        break;
    }

    yunos_task_dyn_del(g_active_task);
}

static void task_mutex_coopr2_co2_entry(void *arg)
{
    while (1) {
        yunos_task_sleep(10);

        yunos_mutex_lock(&test_mutex_co2, YUNOS_WAIT_FOREVER);
        yunos_mutex_lock(&test_mutex_co2, YUNOS_WAIT_FOREVER);

        break;
    }

    yunos_task_dyn_del(g_active_task);
}

static void task_mutex_coopr2_co3_entry(void *arg)
{
    uint8_t pri;

    yunos_mutex_create(&test_mutex_co1, MODULE_NAME_CO2);
    yunos_mutex_create(&test_mutex_co2, MODULE_NAME_CO2);

    while (1) {
        yunos_mutex_lock(&test_mutex_co1, YUNOS_WAIT_FOREVER);
        yunos_mutex_lock(&test_mutex_co2, YUNOS_WAIT_FOREVER);

        yunos_task_sleep(20);
        pri = task_pri_get(g_active_task);
        if (pri == TASK_MUTEX_PRI) {
            yunos_mutex_unlock(&test_mutex_co1);

            pri = task_pri_get(g_active_task);
            if (pri == TASK_MUTEX_PRI+1) {
                yunos_mutex_unlock(&test_mutex_co2);

                pri = task_pri_get(g_active_task);
                if (pri == TASK_MUTEX_PRI+2) {
                    break;
                } else {
                    test_case_fail++;
                    PRINT_RESULT(MODULE_NAME_CO2, FAIL);

                    yunos_mutex_del(&test_mutex);

                    next_test_case_notify();
                    yunos_task_dyn_del(g_active_task);

                    return;
                }
            } else {
                test_case_fail++;
                PRINT_RESULT(MODULE_NAME_CO2, FAIL);

                yunos_mutex_del(&test_mutex);

                next_test_case_notify();
                yunos_task_dyn_del(g_active_task);

                return;
            }
        } else {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO2, FAIL);

            yunos_mutex_del(&test_mutex);

            next_test_case_notify();
            yunos_task_dyn_del(g_active_task);

            return;
        }

    }

    test_case_success++;
    PRINT_RESULT(MODULE_NAME_CO2, PASS);

    yunos_mutex_del(&test_mutex);

    next_test_case_notify();
    yunos_task_dyn_del(g_active_task);
}


void mutex_coopr2_test(void)
{
    kstat_t ret;

    ret = yunos_task_dyn_create(&task_mutex_co1, MODULE_NAME_CO2, 0, TASK_MUTEX_PRI,
                                 0, TASK_TEST_STACK_SIZE, task_mutex_coopr2_co1_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_mutex_co2, MODULE_NAME_CO2, 0, TASK_MUTEX_PRI+1,
                                 0, TASK_TEST_STACK_SIZE, task_mutex_coopr2_co2_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_mutex_co3, MODULE_NAME_CO2, 0, TASK_MUTEX_PRI+2,
                                 0, TASK_TEST_STACK_SIZE, task_mutex_coopr2_co3_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

