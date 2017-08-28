/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <test_fw.h>
#include "comb_test.h"

static ktask_t *task_sem;
static ktask_t *task_mutex;
static ktask_t *task_event;
static ktask_t *task_queue;

static ksem_t  *sem_comb_all;
static kmutex_t mutex_comb_all;
static kevent_t event_comb_all;
static kqueue_t queue_comb_all;

#define TEST_MSG_SIZE   8
static void *queue1_msg_buff[TEST_MSG_SIZE];
void *queue1_trigger_msg;

#define MSG_SIGNATURE   0x5A
static void *msg_word;

static uint8_t mutex_count;

#define MODULE_NAME    "comb_all_opr"

static void task_sem_opr_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_sem_take(sem_comb_all, YUNOS_WAIT_FOREVER);
    if ((ret == YUNOS_SUCCESS) && (mutex_count == 66)) {
        test_case_success++;
        PRINT_RESULT(MODULE_NAME, PASS);
    } else {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    yunos_sem_dyn_del(sem_comb_all);
    yunos_mutex_del(&mutex_comb_all);
    yunos_event_del(&event_comb_all);
    yunos_queue_del(&queue_comb_all);

    next_test_case_notify();
    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_mutex_opr_entry(void *arg)
{
    uint32_t flag;

    yunos_event_get(&event_comb_all, 0x1, YUNOS_AND_CLEAR, &flag,
                    YUNOS_WAIT_FOREVER);

    yunos_mutex_lock(&mutex_comb_all, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb_all, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb_all, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb_all, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb_all, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb_all, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb_all, YUNOS_WAIT_FOREVER);
    yunos_mutex_lock(&mutex_comb_all, YUNOS_WAIT_FOREVER);

    mutex_count = 55;

    yunos_mutex_unlock(&mutex_comb_all);
    yunos_mutex_unlock(&mutex_comb_all);
    yunos_mutex_unlock(&mutex_comb_all);
    yunos_mutex_unlock(&mutex_comb_all);
    yunos_mutex_unlock(&mutex_comb_all);
    yunos_mutex_unlock(&mutex_comb_all);
    yunos_mutex_unlock(&mutex_comb_all);
    yunos_mutex_unlock(&mutex_comb_all);

    mutex_count = 66;

    yunos_sem_give(sem_comb_all);
    yunos_task_dyn_del(yunos_cur_task_get());
}

static void task_event_opr_entry(void *arg)
{
    kstat_t ret;

    ret = yunos_queue_recv(&queue_comb_all, YUNOS_WAIT_FOREVER,
                           (void *)&queue1_trigger_msg);
    if ((*(uint8_t *)&queue1_trigger_msg == MSG_SIGNATURE) &&
        (ret == YUNOS_SUCCESS)) {
        yunos_event_set(&event_comb_all, 0x1, YUNOS_OR);
        yunos_task_dyn_del(yunos_cur_task_get());
    }
}

static void task_queue_opr_entry(void *arg)
{
    *((char *)&msg_word) = MSG_SIGNATURE;
    yunos_queue_back_send(&queue_comb_all, (void *)msg_word);
    yunos_task_dyn_del(yunos_cur_task_get());
}

void comb_all_coopr_test(void)
{
    kstat_t ret;

    yunos_sem_dyn_create(&sem_comb_all, "semcomball", 0);
    yunos_mutex_create(&mutex_comb_all, "mutexcomball");
    yunos_event_create(&event_comb_all, "eventcomball", 0x2);
    yunos_queue_create(&queue_comb_all, "queue1", (void **)&queue1_msg_buff,
                       TEST_MSG_SIZE);

    ret = yunos_task_dyn_create(&task_sem, MODULE_NAME, 0, TASK_COMB_PRI,
                                0, TASK_TEST_STACK_SIZE, task_sem_opr_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_mutex, MODULE_NAME, 0, TASK_COMB_PRI + 1,
                                0, TASK_TEST_STACK_SIZE, task_mutex_opr_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_event, MODULE_NAME, 0, TASK_COMB_PRI + 2,
                                0, TASK_TEST_STACK_SIZE, task_event_opr_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }

    ret = yunos_task_dyn_create(&task_queue, MODULE_NAME, 0, TASK_COMB_PRI + 3,
                                0, TASK_TEST_STACK_SIZE, task_queue_opr_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

