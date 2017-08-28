#include <stdio.h>
#include <stdlib.h>

#include <k_api.h>
#include <yos/kernel.h>

#include <yunit.h>

static yos_workqueue_t workqueue;
static yos_work_t work;
static yos_sem_t sync_sem;
static void CASE_aosapi_kernel_workqueue_param()
{
	int ret;
#if 0
	// TODO: test fail(nullptr coredump)
	ret = yos_workqueue_create(NULL, 10, 1024);
	YUNIT_ASSERT(ret == YUNOS_SUCCESS);
#endif

	ret = yos_workqueue_create(&workqueue, YUNOS_CONFIG_PRI_MAX, 1024);
	YUNIT_ASSERT_MSG(ret==YUNOS_BEYOND_MAX_PRI, "ret=%d", ret);

	ret = yos_workqueue_create(&workqueue, YUNOS_CONFIG_PRI_MAX+1, 1024);
	YUNIT_ASSERT_MSG(ret==YUNOS_BEYOND_MAX_PRI, "ret=%d", ret);

	// TODO: test fail(YUNOS_TASK_INV_STACK_SIZE)
	ret = yos_workqueue_create(&workqueue, 10, 0);
	YUNIT_ASSERT_MSG(ret==YUNOS_TASK_INV_STACK_SIZE, "ret=%d", ret);

#if 0
	// TODO: test fail(nullptr coredump)
	yos_workqueue_del(NULL);
#endif
}

static void CASE_aosapi_kernel_workqueue_default()
{
	// TODO: not implement
	// yos_work_cancle() and yos_work_sched()
}

static void WORK_aosapi_kernel_workqueue_custom(void *arg)
{
	int i = 4;
	while(i--) {
		yos_msleep(1000);
		printf("workqueue:%d\n", i);
	}
	yos_sem_signal(&sync_sem);
}
static void CASE_aosapi_kernel_workqueue_custom()
{
	int ret = 0;

	ret = yos_sem_new(&sync_sem, 0);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_workqueue_create(&workqueue, 10, 1024);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_work_init(&work, WORK_aosapi_kernel_workqueue_custom, NULL, 100);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_work_run(&workqueue, &work);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_sem_wait(&sync_sem, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	yos_workqueue_del(&workqueue);
}

static void WORK_aosapi_kernel_work_param(void* arg)
{
}
static void CASE_aosapi_kernel_work_param()
{
	int ret = 0;
#if 0
	// TODO: nullptr coredump
	ret = yos_work_init(NULL,WORK_aosapi_kernel_work_param,  NULL, 1000);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
#endif

	ret = yos_work_init(&work, NULL, NULL, 1024);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);

#if 0
	// TODO: nullptr coredump
	ret = yos_work_run(NULL, &work);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
#endif

#if 0
	// TODO: nullptr coredump
	ret = yos_workqueue_create(&workqueue, 10, 1024);
	YUNIT_ASSERT(ret == YUNOS_SUCCESS);
	yos_work_run(&workqueue, NULL);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
	yos_workqueue_del(&workqueue);
#endif

#if 0
	// TODO: not implement
	ret = yos_work_cancel(NULL);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
#endif

#if 0
	// TODO: not implement
	ret = yos_work_sched(NULL);
	YUNIT_ASSERT(ret == YUNOS_NULL_PTR);
#endif

	(void)WORK_aosapi_kernel_work_param;
}



void aosapi_kernel_workqueue_test_entry(yunit_test_suite_t *suite)
{
	yunit_add_test_case(suite, "kernel.workqueue.param", CASE_aosapi_kernel_workqueue_param);
	yunit_add_test_case(suite, "kernel.workqueue.default", CASE_aosapi_kernel_workqueue_default);
	yunit_add_test_case(suite, "kernel.workqueue.custom", CASE_aosapi_kernel_workqueue_custom);
	yunit_add_test_case(suite, "kernel.work.param", CASE_aosapi_kernel_work_param);
}
