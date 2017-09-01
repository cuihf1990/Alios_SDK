/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <test_fw.h>
#include "comb_test.h"

#if (YUNOS_CONFIG_KOBJ_SET > 0)
static ktask_t *task_mutex;
static ktask_t *task_queue;
static ktask_t *task_queue_trigger;

static kmutex_t *mutex_comb;
static kqueue_t queue;

#define TEST_MSG_SIZE   8
#define TEST_MSG_NUM    8

void *queue_msg_buff[TEST_MSG_SIZE];
void *queue_trigger_msg;

#define MSG_SIGNATURE   0x5A
static void *msg_word;

static uint8_t     notify_flag;
static kobj_set_t *handle;
static blk_obj_t  *select_obj;

#define MODULE_NAME    "mutex_queue_opr"

static void task_mutex_opr_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_mutex_lock(mutex_comb, YUNOS_WAIT_FOREVER);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    while (notify_flag != MSG_SIGNATURE) {
        yunos_task_sleep(5);
    }

    ret = yunos_mutex_unlock(mutex_comb);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    if (test_case_check_err == 0) {
        PRINT_RESULT(MODULE_NAME, PASS);
    } else {
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    next_test_case_notify();
    ret = yunos_mutex_dyn_del(mutex_comb);
    if (ret != YUNOS_SUCCESS) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_queue_opr_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_kobj_set_insert((blk_obj_t *)&queue, handle);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_kobj_select(handle, &select_obj, YUNOS_WAIT_FOREVER);

    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_queue_recv(&queue, YUNOS_WAIT_FOREVER, &queue_trigger_msg);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);
    TEST_FW_VAL_CHK(MODULE_NAME, *(uint8_t *)&queue_trigger_msg == MSG_SIGNATURE);

    if (test_case_check_err == 0) {
        notify_flag = MSG_SIGNATURE;
    }

    yunos_queue_del(&queue);
    yunos_kobj_set_dyn_del(handle);
    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_queue_trigger_entry(void *arg)
{
    *((uint8_t *)&msg_word) = MSG_SIGNATURE;

    yunos_queue_front_send(&queue, (void *)msg_word);

    yunos_task_dyn_del(yunos_cur_task_get());
}

void mutex_queue_coopr_test(void)
{
    kstat_t ret;
    test_case_check_err = 0;

    ret = yunos_kobj_set_dyn_create(&handle, "obj_set", TEST_MSG_NUM);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_mutex_dyn_create(&mutex_comb, "mutexcomb");
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_queue_create(&queue, "queue", (void **)&queue_msg_buff,
                             TEST_MSG_SIZE);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_task_dyn_create(&task_mutex, MODULE_NAME, 0, TASK_COMB_PRI,
                                0, TASK_TEST_STACK_SIZE, task_mutex_opr_entry, 1);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);
    ret = yunos_task_dyn_create(&task_queue, MODULE_NAME, 0, TASK_COMB_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_queue_opr_entry, 1);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);

    ret = yunos_task_dyn_create(&task_queue_trigger, MODULE_NAME, 0,
                                TASK_COMB_PRI + 2,
                                0, TASK_TEST_STACK_SIZE, task_queue_trigger_entry, 1);
    TEST_FW_VAL_CHK(MODULE_NAME, ret == YUNOS_SUCCESS);
}
#endif

