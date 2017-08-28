/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <k_api.h>
#include <test_fw.h>

#include "buf_queue_test.h"

#define TEST_BUFQUEUE_MSG0_SIZE 8
#define TEST_BUFQUEUE_BUF0_SIZE 16
#define TEST_BUFQUEUE_MSG_MAX   8

static ktask_t     *task_0_test;
static char         g_test_recv_msg0[TEST_BUFQUEUE_MSG0_SIZE];
static char         g_test_bufqueue_buf0[TEST_BUFQUEUE_MSG0_SIZE];
static kbuf_queue_t g_test_bufqueue0;

static void buf_queue_create_param_test(void)
{
    kstat_t ret;

    ret = yunos_buf_queue_create(NULL, "test_bufqueue0", g_test_bufqueue_buf0,
                                 TEST_BUFQUEUE_BUF0_SIZE, TEST_BUFQUEUE_MSG_MAX);
    BUFQUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);

    ret = yunos_buf_queue_create(&g_test_bufqueue0, NULL, g_test_bufqueue_buf0,
                                 TEST_BUFQUEUE_BUF0_SIZE, TEST_BUFQUEUE_MSG_MAX);
    BUFQUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);

    ret = yunos_buf_queue_create(&g_test_bufqueue0, "test_bufqueue0", NULL,
                                 TEST_BUFQUEUE_BUF0_SIZE, TEST_BUFQUEUE_MSG_MAX);
    BUFQUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);

    ret = yunos_buf_queue_create(&g_test_bufqueue0, "test_bufqueue0",
                                 g_test_bufqueue_buf0,
                                 0, TEST_BUFQUEUE_MSG_MAX);
    BUFQUEUE_VAL_CHK(ret == YUNOS_BUF_QUEUE_SIZE_ZERO);
}

static void buf_queue_del_param_test(void)
{
    kstat_t ret;
    ksem_t  sem;

    ret = yunos_buf_queue_del(NULL);
    BUFQUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);

    yunos_intrpt_enter();
    ret = yunos_buf_queue_del(&g_test_bufqueue0);
    BUFQUEUE_VAL_CHK(ret == YUNOS_NOT_CALLED_BY_INTRPT);
    yunos_intrpt_exit();

    yunos_sem_create(& sem, "test_sem ", 0);
    ret = yunos_buf_queue_del((kbuf_queue_t *)&sem);
    BUFQUEUE_VAL_CHK(ret == YUNOS_KOBJ_TYPE_ERR);
    yunos_sem_del(&sem);

    ret = yunos_buf_queue_dyn_del(&g_test_bufqueue0);
    BUFQUEUE_VAL_CHK(ret == YUNOS_KOBJ_DEL_ERR);
}

static void task_queue0_entry(void *arg)
{
    kstat_t ret;
    size_t  size;

    while (1) {
        ret = yunos_buf_queue_create(&g_test_bufqueue0, "test_bufqueue0",
                                     (void **)&g_test_bufqueue_buf0,
                                     TEST_BUFQUEUE_BUF0_SIZE, TEST_BUFQUEUE_MSG_MAX);
        BUFQUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        /* check yunos_buf_queue_create param */
        buf_queue_create_param_test();

        /* check yunos_buf_queue_del param */
        buf_queue_del_param_test();

        ret = yunos_buf_queue_del(&g_test_bufqueue0);
        BUFQUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        next_test_case_notify();

        yunos_task_dyn_del(task_0_test);
    }
}

kstat_t task_buf_queue_del_test(void)
{
    kstat_t ret;
    test_case_check_err = 0;

    ret = yunos_task_dyn_create(&task_0_test, "task_bufqueue0_test", 0, 10,
                                0, TASK_TEST_STACK_SIZE, task_queue0_entry, 1);
    BUFQUEUE_VAL_CHK((ret == YUNOS_SUCCESS) || (ret == YUNOS_STOPPED));

    return 0;
}

