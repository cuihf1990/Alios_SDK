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
#include <stdio.h>
#include <stdlib.h>

#include <k_api.h>
#include <yos/kernel.h>

#include <yunit.h>


static yos_mutex_t g_mutex1;
static yos_mutex_t g_mutex2;
static yos_sem_t sync_sem;
static const uint32_t LOOP_COUNT = 1000000;

#define TEST_TASK_STACK_SIZE (8192)

///////////////////////////////////////////////////////////////////////////////////////////////
static void CASE_aosapi_kernel_mutex_param()
{
#if 0
	int ret;
	yos_mutex_t mutex;
	// FIXME: null pointer:coredump
	ret = yos_mutex_new(NULL);
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);
#endif

#if 0
	// FIXME: null pointer:coredump
	ret = yos_mutex_lock(NULL, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);
#endif

#if 0
	// FIXME: null pointer:coredump
	ret = yos_mutex_unlock(NULL);
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);
#endif

#if 0
	ret = yos_mutex_free(NULL);
	YUNIT_ASSERT_MSG(ret==YUNOS_NULL_PTR, "ret=%d", ret);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////
void TASK_aosapi_kernel_mutex_lock1(void *arg)
{
	int ret;
	int *pflag = (int*)arg;
	for(int i=0; i<LOOP_COUNT; i++) {
		ret = yos_mutex_lock(&g_mutex1, YUNOS_WAIT_FOREVER);
		YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

		(*pflag)++;

		ret = yos_mutex_unlock(&g_mutex1);
		YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	}
    yos_sem_signal(&sync_sem);
    yos_task_exit(0);
}
void TASK_aosapi_kernel_mutex_lock2(void *arg)
{
	int ret;
	int *pflag = (int*)arg;
	for(int i=0; i<LOOP_COUNT; i++) {
		ret = yos_mutex_lock(&g_mutex1, YUNOS_WAIT_FOREVER);
		YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

		(*pflag)--;

		ret= yos_mutex_unlock(&g_mutex1);
		YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	}
    yos_sem_signal(&sync_sem);
    yos_task_exit(0);
}
static void CASE_aosapi_kernel_mutex_lock()
{
	int ret = YUNOS_SUCCESS;
	ret = yos_mutex_new(&g_mutex1);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

    ret = yos_sem_new(&sync_sem, 0);
    YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

    int flag = 0;
	ret = yos_task_new("TASK_aosapi_kernel_mutex_lock_wait1",
			           TASK_aosapi_kernel_mutex_lock1, &flag, TEST_TASK_STACK_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_task_new("TASK_aosapi_kernel_mutex_lock_wait2",
			           TASK_aosapi_kernel_mutex_lock2, &flag, TEST_TASK_STACK_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

    ret = yos_sem_wait(&sync_sem, YUNOS_WAIT_FOREVER);
    YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

    ret = yos_sem_wait(&sync_sem, YUNOS_WAIT_FOREVER);
    YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

    yos_mutex_free(&g_mutex1);
    yos_sem_free(&sync_sem);

    YUNIT_ASSERT_MSG(flag==0, "flag=%d", flag);
}


///////////////////////////////////////////////////////////////////////////////////////////////
void TASK_aosapi_kernel_mutex_deadlock1(void *arg)
{
	int ret;
	ret = yos_mutex_lock(&g_mutex1, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_mutex_lock(&g_mutex2, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	yos_mutex_unlock(&g_mutex1);
	yos_sem_signal(&sync_sem);
}
void TASK_aosapi_kernel_mutex_deadlock2(void *arg)
{
	int ret;
	ret = yos_mutex_lock(&g_mutex2, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_mutex_lock(&g_mutex1, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	yos_mutex_unlock(&g_mutex2);
	yos_sem_signal(&sync_sem);
}
static void CASE_aosapi_kernel_mutex_deadlock()
{
	int ret = YUNOS_SUCCESS;
	ret = yos_mutex_new(&g_mutex1);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_mutex_new(&g_mutex2);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);
	ret = yos_sem_new(&sync_sem, 0);

	ret = yos_task_new("TASK_aosapi_kernel_mutex_deadlock1",
			           TASK_aosapi_kernel_mutex_deadlock1, NULL, TEST_TASK_STACK_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_task_new("TASK_aosapi_kernel_mutex_deadlock2",
			           TASK_aosapi_kernel_mutex_deadlock2, NULL, TEST_TASK_STACK_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

    ret = yos_sem_wait(&sync_sem, YUNOS_WAIT_FOREVER);
    YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

    ret = yos_sem_wait(&sync_sem, YUNOS_WAIT_FOREVER);
    YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

    yos_mutex_free(&g_mutex1);
    yos_sem_free(&sync_sem);
}

///////////////////////////////////////////////////////////////////////////////////////////////
static void CASE_aosapi_kernel_mutex_repeatlock()
{
	int ret;
	ret = yos_mutex_new(&g_mutex1);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_mutex_lock(&g_mutex1,YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	// TODO: test fail
	ret = yos_mutex_lock(&g_mutex1,YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	yos_mutex_free(&g_mutex1);
}


///////////////////////////////////////////////////////////////////////////////////////////////
void TASK_aosapi_kernel_mutex_lock_timeout(void *arg)
{
	int ret;
	ret = yos_mutex_lock(&g_mutex1, 2000);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_mutex_lock(&g_mutex1, YUNOS_NO_WAIT);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	yos_sem_signal(&sync_sem);
	yos_task_exit(0);
}
static void CASE_aosapi_kernel_mutex_lock_timeout()
{
	int ret;
	ret = yos_sem_new(&sync_sem, 0);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_mutex_new(&g_mutex1);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_mutex_lock(&g_mutex1, YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_task_new("TASK_aosapi_kernel_mutex_lock_timeout",
			           TASK_aosapi_kernel_mutex_lock_timeout, NULL, TEST_TASK_STACK_SIZE);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	ret = yos_sem_wait(&sync_sem,YUNOS_WAIT_FOREVER);
	YUNIT_ASSERT_MSG(ret==YUNOS_SUCCESS, "ret=%d", ret);

	yos_sem_free(&sync_sem);
	yos_mutex_free(&g_mutex1);
}


void aosapi_kernel_mutex_test_entry(yunit_test_suite_t* suite)
{
	yunit_add_test_case(suite, "kernel.mutex.param", CASE_aosapi_kernel_mutex_param);
//	yunit_add_test_case(suite, "kernel.mutex.lockwait", CASE_aosapi_kernel_mutex_lock);
//	yunit_add_test_case(suite, "kernel.mutex.locktimeout", CASE_aosapi_kernel_mutex_lock_timeout);
	yunit_add_test_case(suite, "kernel.mutex.repeat", CASE_aosapi_kernel_mutex_repeatlock);
//	yunit_add_test_case(suite, "kernel.mutex.deadlock", CASE_aosapi_kernel_mutex_deadlock);

	(void)CASE_aosapi_kernel_mutex_deadlock;
	(void)CASE_aosapi_kernel_mutex_lock;
	(void)CASE_aosapi_kernel_mutex_lock_timeout;
	(void)CASE_aosapi_kernel_mutex_deadlock;

}

