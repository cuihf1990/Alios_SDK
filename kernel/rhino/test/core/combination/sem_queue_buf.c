/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <test_fw.h>
#include "comb_test.h"

static ktask_t *task_sem;
static ktask_t *task_buf_queue;
static ktask_t *task_buf_queue_trigger;

static ksem_t      *test_sem;
static kbuf_queue_t test_buf_queue;
static uint8_t buf_queue_test_buf[1];
static uint8_t buf_queue_recv[1];
static uint8_t buf_queue_send[1];

#define MODULE_NAME    "sem_queue_buf_opr"

static void task_sem_opr_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_sem_take(test_sem, YUNOS_WAIT_FOREVER);
    if (ret == YUNOS_SUCCESS) {
        test_case_success++;
        PRINT_RESULT(MODULE_NAME, PASS);
    } else {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    next_test_case_notify();
    yunos_sem_dyn_del(test_sem);
    yunos_buf_queue_del(&test_buf_queue);
    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_buf_queue_entry(void *arg)
{
    kstat_t ret;
    size_t  size;

    ret = yunos_buf_queue_recv(&test_buf_queue, YUNOS_WAIT_FOREVER,
                               (void *)buf_queue_recv, &size);
    if ((ret == YUNOS_SUCCESS) && (*(uint8_t *)buf_queue_recv == 0x5a)) {
        yunos_sem_give(test_sem);
        yunos_task_dyn_del(yunos_cur_task_get());
    }
}

static void task_buf_queue_trigger_entry(void *arg)
{
    *(uint8_t *)buf_queue_send = 0x5a;

    yunos_buf_queue_send(&test_buf_queue, (void *)buf_queue_send, 1);
    yunos_task_dyn_del(yunos_cur_task_get());
}

void sem_buf_queue_coopr_test(void)
{
    kstat_t ret;

    yunos_sem_dyn_create(&test_sem, "semtest", 0);
    yunos_buf_queue_create(&test_buf_queue, "bugqueue", (void *)buf_queue_test_buf,
                           8, 1);

    ret = yunos_task_dyn_create(&task_sem, MODULE_NAME, 0, TASK_COMB_PRI,
                                0, TASK_TEST_STACK_SIZE, task_sem_opr_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_buf_queue, MODULE_NAME, 0, TASK_COMB_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_buf_queue_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_buf_queue_trigger, MODULE_NAME, 0,
                                TASK_COMB_PRI + 2,
                                0, TASK_TEST_STACK_SIZE, task_buf_queue_trigger_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

