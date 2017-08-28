#include <stdio.h>
#include <k_api.h>
#include <test_fw.h>

#include "queue_test.h"

#define TEST_QUEUE_MSG0_SIZE 30

static ktask_t  *task_0_test;
static ktask_t  *task_1_test;
static ktask_t  *task_2_test;
static void    *queue_send_msg = (void *)1;
static void    *queue_recv_msg = (void *)1;
static void    *g_test_queue_msg0[TEST_QUEUE_MSG0_SIZE];
static kqueue_t g_test_queue0;
static int      g_allsend_count = 0;

static void queue_all_send_param_test(void)
{
    kstat_t ret;

    ret = yunos_queue_all_send(NULL, queue_send_msg, QMSG_SEND_TO_END);
    QUEUE_VAL_CHK(ret == YUNOS_NULL_PTR);
}

static void task_queue0_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        g_allsend_count = 0;
        queue_send_msg  = (void *)1;
        queue_recv_msg  = (void *)1;

        /* check yunos_queue_all_send param */
        queue_all_send_param_test();

        /* check yunos_queue_all_send */
        ret = yunos_queue_recv(&g_test_queue0, YUNOS_WAIT_FOREVER, &queue_recv_msg);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);
        ret = yunos_queue_recv(&g_test_queue0, YUNOS_WAIT_FOREVER, &queue_recv_msg);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        if (queue_recv_msg == queue_send_msg) {
            g_allsend_count++;
        } else {
            test_case_fail++;
            printf("queue all send is \t[FAIL %d]\n", (int)test_case_fail);
        }

        yunos_queue_del(&g_test_queue0);
        next_test_case_notify();
        yunos_task_dyn_del(task_0_test);
    }
}

static void task_queue1_entry(void *arg)
{
    kstat_t ret;

    while (1) {
        /* check yunos_queue_all_send */
        ret = yunos_queue_recv(&g_test_queue0, YUNOS_WAIT_FOREVER, &queue_recv_msg);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);
        ret = yunos_queue_recv(&g_test_queue0, YUNOS_WAIT_FOREVER, &queue_recv_msg);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        if ((queue_recv_msg == queue_send_msg) && (g_allsend_count == 1)) {
            test_case_success++;
            PRINT_RESULT("queue all send", PASS);
        } else {
            test_case_fail++;
            PRINT_RESULT("queue all send", FAIL);
        }

        yunos_task_dyn_del(task_1_test);
    }
}

static void task_queue2_entry(void *arg)
{
    kstat_t ret;

    yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND / 10);

    while (1) {
        /* check yunos_queue_back_send */
        ret = yunos_queue_all_send(&g_test_queue0, queue_send_msg, QMSG_SEND_TO_END);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);
        ret = yunos_queue_all_send(&g_test_queue0, queue_send_msg, QMSG_SEND_TO_FRONT);
        QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

        yunos_task_dyn_del(task_2_test);
    }
}

kstat_t task_queue_all_send_test(void)
{
    kstat_t ret;

    ret = yunos_queue_create(&g_test_queue0, "test_queue0",
                             (void **)&g_test_queue_msg0, TEST_QUEUE_MSG0_SIZE);
    QUEUE_VAL_CHK(ret == YUNOS_SUCCESS);

    ret = yunos_task_dyn_create(&task_0_test, "task_queue0_test", 0, 10,
                                0, TASK_TEST_STACK_SIZE, task_queue0_entry, 1);
    QUEUE_VAL_CHK((ret == YUNOS_SUCCESS) || (ret == YUNOS_STOPPED));

    ret = yunos_task_dyn_create(&task_1_test, "task_queue1_test", 0, 11,
                                0, TASK_TEST_STACK_SIZE, task_queue1_entry, 1);
    QUEUE_VAL_CHK((ret == YUNOS_SUCCESS) || (ret == YUNOS_STOPPED));

    ret = yunos_task_dyn_create(&task_2_test, "task_queue2_test", 0, 12,
                                0, TASK_TEST_STACK_SIZE, task_queue2_entry, 1);
    QUEUE_VAL_CHK((ret == YUNOS_SUCCESS) || (ret == YUNOS_STOPPED));

    return 0;
}

