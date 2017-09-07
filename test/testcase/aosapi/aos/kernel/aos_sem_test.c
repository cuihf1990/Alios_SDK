/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>

#include <k_api.h>
#include <yos/kernel.h>

#include <yunit.h>


static yos_sem_t sem;

static void CASE_aosapi_kernel_sem_param()
{
	int ret;
	yos_mutex_t mutex;

#if 0
	// TODO: nullptr coredump
	ret = yos_sem_new(NULL, 0);
	YUNIT_ASSERT_MSG(ret==RHINO_NULL_PTR, "ret=%d", ret);
#endif

	// TODO: test fail
	ret = yos_sem_new(&sem, -1);
	YUNIT_ASSERT_MSG(ret==RHINO_SUCCESS, "ret=%d", ret);
	if(ret==RHINO_SUCCESS) {
		yos_sem_free(&sem);
	}

	ret = yos_sem_new(&sem, -2);
	YUNIT_ASSERT_MSG(ret==RHINO_SUCCESS, "ret=%d", ret);
	if(ret==RHINO_SUCCESS) {
		yos_sem_free(&sem);
	}

	ret = yos_sem_new(&sem, 0x7FFFFFFF);
	YUNIT_ASSERT_MSG(ret==RHINO_SUCCESS, "ret=%d", ret);
	if(ret==RHINO_SUCCESS) {
		yos_sem_free(&sem);
	}

	// TODO: test fail
	ret = yos_sem_new(&sem, 0xFFFFFFFF);
	YUNIT_ASSERT_MSG(ret==RHINO_SUCCESS, "ret=%d", ret);
	if(ret==RHINO_SUCCESS) {
		yos_sem_free(&sem);
	}

#if 0
	// TODO: nullptr param
	yos_sem_signal(NULL);
#endif

#if 0
	yos_mutex_new(&mutex);
	yos_sem_signal((yos_sem_t*)&mutex);
	yos_mutex_free(&mutex);
#endif

#if 0
	// TODO: nullptr param
	ret = yos_sem_wait(NULL, RHINO_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==RHINO_SUCCESS, "ret=%d", ret);
#endif

	yos_mutex_new(&mutex);
	ret = yos_sem_wait((yos_sem_t*)&mutex, RHINO_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==RHINO_KOBJ_TYPE_ERR, "ret=%d", ret);
	yos_mutex_free(&mutex);

	ret = yos_sem_new(&sem, 0);
	YUNIT_ASSERT_MSG(ret==RHINO_SUCCESS, "ret=%d", ret);
	ret = yos_sem_wait(&sem, RHINO_NO_WAIT);
	YUNIT_ASSERT_MSG(ret==RHINO_NO_PEND_WAIT, "ret=%d", ret);
	yos_sem_free(&sem);

	ret = yos_sem_new(&sem, 0);
	YUNIT_ASSERT_MSG(ret==RHINO_SUCCESS, "ret=%d", ret);
	ret = yos_sem_wait(&sem, 1000);
	YUNIT_ASSERT_MSG(ret==RHINO_BLK_TIMEOUT, "ret=%d", ret);
	yos_sem_free(&sem);

	ret = yos_sem_new(&sem, 0);
	YUNIT_ASSERT_MSG(ret==RHINO_SUCCESS, "ret=%d", ret);
	ret = yos_sem_wait(&sem, 0);
	YUNIT_ASSERT_MSG(ret==RHINO_NO_PEND_WAIT, "ret=%d", ret);
	yos_sem_free(&sem);
}

static void CASE_aosapi_kernel_sem_normal()
{

}


void aosapi_kernel_sem_test_entry(yunit_test_suite_t *suite)
{
	yunit_add_test_case(suite, "kernel.sem.param", CASE_aosapi_kernel_sem_param);
	yunit_add_test_case(suite, "kernel.sem.normal", CASE_aosapi_kernel_sem_normal);
}

