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

#include <stdlib.h>
#include <stdio.h>

#include <yos/kernel.h>
#include <yos/framework.h>

#include <yunit.h>
#include <yts.h>

extern void ota_check_update(const char *buf, int len);

extern void ota_service_init(void);

extern void do_update(const char *buf);

const char *ota_info = "{\"md5\":\"6B21342306D0F619AF97006B7025D18A\",\"resourceUrl\":\"http:\/\/otalink.alicdn.com\/ALINKTEST_LIVING_LIGHT_ALINK_TEST\/v2.0.0.1\/uthash-master.zip\",\"size\":\"265694 \",\"uuid\":\"5B7CFD5C6B1D6A231F5FB6B7DB2B71FD\",\"version\":\"v2.0.0.1\",\"zip\":\"0\"}";

static void test_fota_case(void)
{
    ota_service_init();
    yos_post_event(EV_SYS, CODE_SYS_ON_START_FOTA, 0);
    ota_check_update("",1);
    do_update(ota_info);
}

static int init(void)
{
    return 0;
}

static int cleanup(void)
{
    return 0;
}

static void setup(void)
{

}

static void teardown(void)
{

}

static yunit_test_case_t yunos_basic_testcases[] = {
    { "fota", test_fota_case },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "yloop", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_fota(void)
{
    yunit_add_test_suites(suites);
}

