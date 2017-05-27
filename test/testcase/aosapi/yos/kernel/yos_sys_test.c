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

static void CASE_aosapi_kernel_sys_version()
{
	const char *version = yos_version_get();
	YUNIT_ASSERT(strcmp(version, "0.4.0")==0);
}

static void CASE_aosapi_kernel_sys_reboot()
{
	YUNIT_ASSERT(1);
}


void aosapi_kernel_sys_test_entry(yunit_test_suite_t *suite)
{
	yunit_add_test_case(suite, "kernel.sys.reboot", CASE_aosapi_kernel_sys_reboot);
	yunit_add_test_case(suite, "kernel.sys.version", CASE_aosapi_kernel_sys_version);
}

