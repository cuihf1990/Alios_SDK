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
#include "workqueue_test.h"

#define MODULE_NAME "workqueue_interface"

#if (YUNOS_CONFIG_WORKQUEUE > 0)
extern void workqueue_init(void);

static ksem_t g_wq_test_sem;

static void work0_func(void *arg)
{
    yunos_task_sleep(45);
    printf("--%s--\n", (char *)arg);
}

static void work1_func(void *arg)
{
    yunos_task_sleep(15);
    printf("--%s--\n", (char *)arg);
}

static void work2_func(void *arg)
{
    yunos_task_sleep(10);
    printf("--%s--\n", (char *)arg);
}

static void work3_func(void *arg)
{
    printf("--%s--\n", (char *)arg);
}

static void work4_func(void *arg)
{
    yunos_task_sleep(15);
    printf("--%s--\n", (char *)arg);
    yunos_sem_give(&g_wq_test_sem);
}

static void work5_func(void *arg)
{
    printf("--%s--\n", (char *)arg);
    yunos_sem_give(&g_wq_test_sem);
}

static void work6_func(void *arg)
{
    printf("--%s--\n", (char *)arg);
}

static uint8_t workqueue_interface_case1(void)
{
    kstat_t     ret;
    work_t      work0;
    work_t      work1;
    work_t      work2;
    work_t      work3;
    work_t      work4;
    work_t      work5;
    work_t      work6;
    workqueue_t wq0;
    workqueue_t wq1;
    workqueue_t wq2;
    cpu_stack_t stack0_buf[512];
    size_t      stack0_size = 512;
    cpu_stack_t stack1_buf[512];
    size_t      stack1_size = 512;
    cpu_stack_t stack2_buf[512];
    size_t      stack2_size = 512;

    printf("==========WORKQUEUE TEST START!==========\n");

    workqueue_init();

    yunos_sem_create(&g_wq_test_sem, "WORKQUEUE-SEM", 0);

    /* creat workqueues */
    ret = yunos_workqueue_create(&wq0, "WORKQUEUE1-TEST", TASK_WORKQUEUE_PRI,
                                 stack0_buf, stack0_size);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_workqueue_create(&wq1, "WORKQUEUE1-TEST", TASK_WORKQUEUE_PRI,
                                 stack1_buf, stack1_size);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_workqueue_del(&wq2);
    if (ret != YUNOS_WORKQUEUE_NOT_EXIST) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_workqueue_create(&wq2, "WORKQUEUE2-TEST", TASK_WORKQUEUE_PRI,
                                 stack2_buf, stack2_size);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_workqueue_create(&wq1, "WORKQUEUE", TASK_WORKQUEUE_PRI,
                                 stack1_buf, stack1_size);
    if (ret != YUNOS_WORKQUEUE_EXIST) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_workqueue_del(&wq0);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    /* init works */
    ret = yunos_work_init(&work0, work0_func, "WORK 0", 0);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_init(&work1, work1_func, "WORK 1", 0);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_init(&work2, work2_func, "WORK 2", 1);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_init(&work3, work3_func, "WORK 3", 20);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_init(&work4, work4_func, "WORK 4", 18);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_init(&work5, work5_func, "WORK 5", 40);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_init(&work6, work6_func, "WORK 6", 50);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    /* work run */
    ret = yunos_work_run(&wq1, &work0);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_run(&wq1, &work1);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_run(&wq1, &work1);
    if (ret != YUNOS_WORKQUEUE_WORK_EXIST) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_workqueue_del(&wq1);
    if (ret != YUNOS_WORKQUEUE_BUSY) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_run(&wq1, &work2);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    yunos_task_sleep(3);

    ret = yunos_work_run(&wq1, &work2);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_run(&wq1, &work3);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_run(&wq2, &work4);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_run(&wq2, &work5);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    ret = yunos_work_run(&wq2, &work6);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    /* wait for task4 */
    yunos_sem_take(&g_wq_test_sem, YUNOS_WAIT_FOREVER);

    ret = yunos_work_run(&wq2, &work5);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        yunos_sem_give(&g_wq_test_sem);
        return 1;
    }

    /* wait for task6 */
    yunos_sem_take(&g_wq_test_sem, YUNOS_WAIT_FOREVER);

    ret = yunos_workqueue_del(&wq2);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_workqueue_del(&wq1);
    if (ret != YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    printf("=====FUNCTION TEST DONE!=====\n");

    ret = yunos_workqueue_create(NULL, "WORKQUEUE1-TEST", TASK_WORKQUEUE_PRI,
                                 stack1_buf, stack1_size);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_workqueue_create(&wq1, NULL, TASK_WORKQUEUE_PRI,
                                 stack1_buf, stack1_size);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_workqueue_create(&wq1, "WORKQUEUE1-TEST", YUNOS_CONFIG_PRI_MAX,
                                 stack1_buf, stack1_size);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_workqueue_create(&wq1, "WORKQUEUE1-TEST", TASK_WORKQUEUE_PRI,
                                 NULL, stack1_size);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_workqueue_create(&wq1, "WORKQUEUE1-TEST", TASK_WORKQUEUE_PRI,
                                 stack1_buf, 0);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_workqueue_del(NULL);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_work_init(NULL, work1_func, "WORK 1", 200);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_work_init(&work1, NULL, "WORK 1", 200);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_work_run(NULL, &work3);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    ret = yunos_work_run(&wq1, NULL);
    if (ret == YUNOS_SUCCESS) {
        MYASSERT(ret);
        return 1;
    }

    printf("=====PARAMTER TEST DONE!=====\n");

    yunos_sem_del(&g_wq_test_sem);

    printf("==========WORKQUEUE TEST DONE!==========\n");

    return 0;
}

static const test_func_t workqueue_func_runner[] = {
    workqueue_interface_case1,
    NULL
};

void workqueue_interface_test(void)
{
    kstat_t ret;

    task_workqueue_entry_register(MODULE_NAME,
                                  (test_func_t *)workqueue_func_runner,
                                  sizeof(workqueue_func_runner) / sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_workqueue, MODULE_NAME, 0,
                                TASK_WORKQUEUE_PRI, 0, TASK_TEST_STACK_SIZE,
                                task_workqueue_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}
#endif
