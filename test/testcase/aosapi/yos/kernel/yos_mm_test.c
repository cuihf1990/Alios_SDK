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

#include <yos/kernel.h>

#include <yunit.h>


static void CASE_aosapi_kernel_mm_param()
{
	char *p = NULL;
	/* dumpsys_mm_info_func here */
	yos_malloc(0);

	/* coredump here */
#if 0
	yos_free(NULL);
#endif
}

static void CASE_aosapi_kernel_mm_allocfree()
{
	const int COUNT = 1024;
	int *ptr = yos_malloc(sizeof(int)*COUNT);

	memset(ptr, 0, COUNT);
	int i = 0;
	for(; i<COUNT; i++) {
		*(ptr+i) = i;
	}
	i = 0;
	for(; i<COUNT; i++) {
		YUNIT_ASSERT_MSG((int)*(ptr+i)==i, "*(ptr+i)=%d", i);
	}
	yos_free(ptr);
	ptr = NULL;
}

static void CASE_aosapi_kernel_mm_alloclarge()
{
	char *p = NULL;
	p = yos_malloc(102400000L);
	if(p) {
		yos_free(p);
	}
}


void aosapi_kernel_mm_test_entry(yunit_test_suite_t *suite)
{
	yunit_add_test_case(suite, "kernel.mm.param", CASE_aosapi_kernel_mm_param);
	yunit_add_test_case(suite, "kernel.mm.allocfree", CASE_aosapi_kernel_mm_allocfree);
	yunit_add_test_case(suite, "kernel.mm.alloclarge", CASE_aosapi_kernel_mm_alloclarge);
}

