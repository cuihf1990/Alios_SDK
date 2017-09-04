/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>

#include <k_api.h>
#include <yos/kernel.h>
#include <assert.h>

#include <yunit.h>

#define TEST_TASK_STACK_SIZE (8192)

typedef struct{
	int id;
	int len;
	char msg[32];
}Message;
#define TEST_QUEUE_MAX_MSG_SIZE      (sizeof(Message))
#define TEST_QUEUE_MAX_MSG_COUNT	 (8)
#define TEST_QUEUE_SIZE              (TEST_QUEUE_MAX_MSG_SIZE * TEST_QUEUE_MAX_MSG_COUNT)

static char queue_buf[TEST_QUEUE_SIZE] = {0};
static yos_queue_t g_queue;
static yos_sem_t sync_sem;
static Message send_msg;
static Message recv_msg;
static unsigned int recv_size = 0;


static void CASE_aosapi_kernel_queue_param()
{
	int ret;
	yos_sem_t tmp_sem;
	yos_queue_t queue;

	/* yos_queue_new invalid param test */
#if 0
	// TODO: coredump
	ret = yos_queue_new(NULL, queue_buf, TEST_QUEUE_SIZE, TEST_QUEUE_MAX_MSG_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);
#endif
	ret = yos_queue_new(&queue, NULL, TEST_QUEUE_SIZE, TEST_QUEUE_MAX_MSG_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);

	ret = yos_queue_new(&queue, queue_buf, 0, TEST_QUEUE_MAX_MSG_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_BUF_QUEUE_SIZE_ZERO, "ret=%d", ret);

	ret = yos_queue_new(&queue, queue_buf, TEST_QUEUE_SIZE, 0);
	YUNIT_ASSERT_MSG(ret==YUNOS_INV_PARAM, "ret=%d", ret);

	/* yos_queue_send invalid param test */
#if 0
	// TODO: coredump
	ret = yos_queue_send(NULL, &send_msg, sizeof(send_msg));
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);
#endif

	// create fail
	ret = yos_queue_new(&queue, queue_buf, TEST_QUEUE_SIZE, TEST_QUEUE_MAX_MSG_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_queue_send(&queue, NULL, sizeof(send_msg));
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);

	ret = yos_queue_send(&queue, &send_msg, 0);
	YUNIT_ASSERT_MSG(ret==YUNOS_INV_PARAM, "ret=%d", ret);

	ret = yos_queue_send(&queue, &send_msg, TEST_QUEUE_MAX_MSG_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_sem_new(&tmp_sem, 0);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	ret = yos_queue_send((yos_queue_t*)&tmp_sem, &send_msg, sizeof(send_msg));
	YUNIT_ASSERT_MSG(ret==YUNOS_KOBJ_TYPE_ERR, "ret=%d", ret);
	yos_sem_free(&tmp_sem);

	/* yos_queue_recv invalid param test */
#if 0
	// TODO: coredump
	ret = yos_queue_recv(NULL, 10, &recv_msg, &recv_size);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
#endif

	ret = yos_queue_recv(&queue, 0, &recv_msg, &recv_size);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret= yos_queue_recv(&queue, YUNOS_WAIT_FOREVER, NULL, &recv_size);
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);

	ret= yos_queue_recv(&queue, YUNOS_WAIT_FOREVER, &recv_msg, NULL);
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);

	yos_sem_new(&tmp_sem, 0);
	ret = yos_queue_recv((yos_queue_t*)&tmp_sem, YUNOS_WAIT_FOREVER, &recv_msg, &recv_size);
	YUNIT_ASSERT_MSG(ret==YUNOS_KOBJ_TYPE_ERR, "ret=%d", ret);
	yos_sem_free(&tmp_sem);
        yos_queue_free(&queue);

	/* yos_queue_free invalid param test */
#if 0
	yos_queue_free(NULL);
#endif
#if 0
	yos_sem_new(&tmp_sem, 0);
	yos_queue_free((yos_queue_t*)&tmp_sem);
	YUNIT_ASSERT_MSG(ret==YUNOS_KOBJ_TYPE_ERR, "ret=%d", ret);
//	yos_sem_free(&tmp_sem); // already free
#endif
}

static void TASK_aosapi_kernel_queue_recv(void *arg)
{
	int ret;
	int i = 1;
	for(; i<10; i++) {
		memset(&recv_msg, 0, sizeof(recv_msg));
		ret = yos_queue_recv(&g_queue, 500, &recv_msg, &recv_size);
		YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
		YUNIT_ASSERT_MSG(recv_size==TEST_QUEUE_MAX_MSG_SIZE, "recv_size=%d", recv_size);
		YUNIT_ASSERT_MSG(recv_msg.id==i, "recv_msg.id=%d", i);
		YUNIT_ASSERT_MSG(recv_msg.len==5, "recv_msg.leb=5");
		YUNIT_ASSERT_MSG(strcmp(recv_msg.msg, "hello")==0, "recv_msg.msg=%s", "hello");

	}
	yos_sem_signal(&sync_sem);
}
static void TASK_aosapi_kernel_queue_send(void *arg)
{
	int ret;
	int i = 1;
	for(; i<10; i++) {
		memset(&send_msg, 0, sizeof(send_msg));
		send_msg.id = i;
		send_msg.len= 5;
		strcpy(send_msg.msg,"hello");
		ret = yos_queue_send(&g_queue, &send_msg, sizeof(send_msg));
		YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
		yos_msleep(50);
	}
	yos_sem_signal(&sync_sem);
}
static void CASE_aosapi_kernel_queue_send_recv()
{
	int ret;

	ret = yos_sem_new(&sync_sem, 0);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	// TODO: nullptr coredump
	ret = yos_queue_new(&g_queue, queue_buf, TEST_QUEUE_SIZE, TEST_QUEUE_MAX_MSG_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	assert(ret == YUNOS_SUCCESS);

	ret = yos_task_new("TASK_aosapi_kernel_queue_send_testcase",
			           TASK_aosapi_kernel_queue_send, NULL, TEST_TASK_STACK_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	ret = yos_task_new("TASK_aosapi_kernel_queue_recv_testcase",
			           TASK_aosapi_kernel_queue_recv, NULL, TEST_TASK_STACK_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	assert(ret == YUNOS_SUCCESS);

	ret = yos_sem_wait(&sync_sem, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_sem_wait(&sync_sem, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	yos_sem_free(&sync_sem);
	yos_queue_free(&g_queue);
}

static void CASE_aosapi_kernel_queue_full()
{
	int ret;
	// create fail
	ret = yos_queue_new(&g_queue, queue_buf, TEST_QUEUE_SIZE, TEST_QUEUE_MAX_MSG_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	for(int i=0; i<TEST_QUEUE_MAX_MSG_COUNT-1; i++) {
		ret = yos_queue_send(&g_queue, &send_msg, sizeof(send_msg));
		YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	}
	ret = yos_queue_send(&g_queue, &send_msg, sizeof(send_msg));
	YUNIT_ASSERT_MSG(ret==YUNOS_BUF_QUEUE_FULL, "ret=%d", ret);

	yos_queue_free(&g_queue);
}


void aosapi_kernel_queue_test_entry(yunit_test_suite_t *suite)
{
	yunit_add_test_case(suite, "kernel.queue.param", CASE_aosapi_kernel_queue_param);
//	yunit_add_test_case(suite, "kernel.queue.sendrecv", CASE_aosapi_kernel_queue_send_recv);
	yunit_add_test_case(suite, "kernel.queue.full", CASE_aosapi_kernel_queue_full);

	(void)CASE_aosapi_kernel_queue_send_recv;
}

