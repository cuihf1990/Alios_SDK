/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <test_fw.h>
#include "tasksem_test.h"

#define MODULE_NAME     "tasksem_opr"
#define MODULE_NAME_CO1 "tasksem_coopr1"
#define MODULE_NAME_CO2 "tasksem_coopr2"

#define LOOP_CNT    8

static uint8_t tasksem_opr_case1(void)
{
    kstat_t ret;

    ret = yunos_task_sem_create(task_tasksem, &test_tasksem, MODULE_NAME, 8);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_task_sem_take(YUNOS_NO_WAIT);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_task_sem_give(task_tasksem);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_task_sem_del(task_tasksem);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t tasksem_func_runner[] = {
    tasksem_opr_case1,
    NULL
};

void tasksem_opr_test(void)
{
    kstat_t ret;

    task_tasksem_entry_register(MODULE_NAME, (test_func_t *)tasksem_func_runner,
                                sizeof(tasksem_func_runner) / sizeof(test_case_t));

    ret = yunos_task_dyn_create(&task_tasksem, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_tasksem_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

static void task_tasksem_co1_entry(void *arg)
{
    kstat_t  ret;
    uint8_t  cnt = 0;

    while (1) {
        ret = yunos_task_sem_take(YUNOS_WAIT_FOREVER);
        MYASSERT_INFO(ret == YUNOS_SUCCESS);

        ret = yunos_task_sem_give(task_tasksem_co2);
        MYASSERT_INFO(ret == YUNOS_SUCCESS);

        cnt++;
        if (cnt >= LOOP_CNT) {
            break;
        }
    }

    yunos_task_sem_del(yunos_cur_task_get());

    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_tasksem_co2_entry(void *arg)
{
    kstat_t ret;
    uint8_t cnt = 0;

    while (1) {
        yunos_sched_disable();

        ret = yunos_task_sem_give(task_tasksem_co1);
        MYASSERT_INFO(ret == YUNOS_SUCCESS);

        yunos_sched_enable();

        ret = yunos_task_sem_take(YUNOS_WAIT_FOREVER);
        if (ret == YUNOS_BLK_TIMEOUT) {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO1, FAIL);
            break;
        }

        cnt++;
        if (cnt >= LOOP_CNT) {
            break;
        }
    }

    test_case_success++;
    PRINT_RESULT(MODULE_NAME_CO1, PASS);

    yunos_task_sem_del(yunos_cur_task_get());

    next_test_case_notify();
    yunos_task_dyn_del(yunos_cur_task_get());
}

void tasksem_coopr1_test(void)
{
    kstat_t ret;

    ret = yunos_task_sem_create(yunos_cur_task_get(), &test_tasksem_co1, MODULE_NAME, 0);
    MYASSERT_INFO(ret == YUNOS_SUCCESS);

    ret = yunos_task_sem_create(yunos_cur_task_get(), &test_tasksem_co2, MODULE_NAME, 0);
    MYASSERT_INFO(ret == YUNOS_SUCCESS);

    ret = yunos_task_dyn_create(&task_tasksem_co1, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_tasksem_co1_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO1, FAIL);
    }

    ret = yunos_task_dyn_create(&task_tasksem_co2, MODULE_NAME, 0, TASK_SEM_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_tasksem_co2_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO1, FAIL);
    }
}

static void task_tasksem_co3_entry(void *arg)
{
    kstat_t ret;
    uint8_t cnt = 0;

    while (1) {
        yunos_task_sleep(5);

        ret = yunos_task_sem_give(task_tasksem_co2);
        MYASSERT_INFO(ret == YUNOS_SUCCESS);

        cnt++;
        if (cnt >= LOOP_CNT) {
            break;
        }
    }

    yunos_task_sem_del(yunos_cur_task_get());

    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_tasksem_co4_entry(void *arg)
{
    kstat_t ret;
    uint8_t cnt = 0;

    while (1) {
        ret = yunos_task_sem_take(YUNOS_WAIT_FOREVER);
        if (ret == YUNOS_BLK_TIMEOUT) {
            test_case_fail++;
            PRINT_RESULT(MODULE_NAME_CO2, FAIL);
            break;
        }

        cnt++;
        if (cnt >= LOOP_CNT) {
            break;
        }
    }

    test_case_success++;
    PRINT_RESULT(MODULE_NAME_CO2, PASS);

    yunos_task_sem_del(yunos_cur_task_get());

    next_test_case_notify();
    yunos_task_dyn_del(yunos_cur_task_get());
}

void tasksem_coopr2_test(void)
{
    kstat_t ret;

    ret = yunos_task_sem_create(yunos_cur_task_get(), &test_tasksem_co1, MODULE_NAME, 0);
    MYASSERT_INFO(ret == YUNOS_SUCCESS);

    ret = yunos_task_sem_create(yunos_cur_task_get(), &test_tasksem_co2, MODULE_NAME, 0);
    MYASSERT_INFO(ret == YUNOS_SUCCESS);

    ret = yunos_task_dyn_create(&task_tasksem_co1, MODULE_NAME, 0, TASK_SEM_PRI,
                                0, TASK_TEST_STACK_SIZE, task_tasksem_co3_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO2, FAIL);
    }

    ret = yunos_task_dyn_create(&task_tasksem_co2, MODULE_NAME, 0, TASK_SEM_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_tasksem_co4_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME_CO2, FAIL);
    }
}

