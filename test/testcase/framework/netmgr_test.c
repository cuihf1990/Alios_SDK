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

#include <yos/kernel.h>
#include <yos/framework.h>
#include <hal/base.h>
#include <hal/wifi.h>
#include <yunit.h>
#include <yts.h>

#include "netmgr.h"


enum {
    SMART_CONFIG_FLAG = 1 << 0,
    SAVED_CONFIG_FLAG = 1 << 1,
    DONE_FLAGS = SMART_CONFIG_FLAG | SAVED_CONFIG_FLAG,
};

static uint8_t done_flag;
static void smart_config_test(void *arg);
static void saved_config_test(void *arg);

static int events_executor(input_event_t *eventinfo, void *cb_para, int flag)
{
    char ips[16];

    if (eventinfo->type != EV_WIFI) {
        return -1;
    }

    switch (eventinfo->code) {
        case CODE_WIFI_ON_GOT_IP:
            done_flag |= flag;
            netmgr_deinit();
            wifi_get_ip(ips);
            return 0;
    }
    return -1;
}

static void saved_config_event_executor(input_event_t *eventinfo, void *cb_para)
{
    events_executor(eventinfo, cb_para, SAVED_CONFIG_FLAG);
}

static void smart_config_event_executor(input_event_t *eventinfo, void *cb_para)
{
    if (events_executor(eventinfo, cb_para, SMART_CONFIG_FLAG) == 0) {
        yos_schedule_call(saved_config_test, NULL);
    }
}

static void smart_config_test(void *arg)
{
    yos_unregister_event_filter(EV_WIFI, saved_config_event_executor, NULL);
    yos_register_event_filter(EV_WIFI, smart_config_event_executor, NULL);
    netmgr_init();
    netmgr_start();
}

static void saved_config_test(void *arg)
{
    yos_unregister_event_filter(EV_WIFI, smart_config_event_executor, NULL);
    yos_register_event_filter(EV_WIFI, saved_config_event_executor, NULL);
    netmgr_init();
    netmgr_start();
}

static void test_netmgr_cases(void *arg)
{
    yos_schedule_call(smart_config_test, NULL);
}

static void netmgr_test_entry(void *arg)
{
    yos_post_delayed_action(1000, test_netmgr_cases, NULL);
    yos_loop_run();
}

static void netmgr_test_exit(void *arg)
{
    yos_loop_exit();
}

static void test_netmgr_connect_case(void)
{
    yos_task_new("netmgr_test_main", netmgr_test_entry, NULL, 8192);

    check_cond_wait(done_flag == DONE_FLAGS, 10);
    yos_schedule_call(netmgr_test_exit, NULL);
    yos_unregister_event_filter(EV_WIFI, smart_config_event_executor, NULL);
    yos_unregister_event_filter(EV_WIFI, saved_config_event_executor, NULL);
}

static int init(void)
{
    done_flag = 0;
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

static yunit_test_case_t yos_netmgr_testcases[] = {
    { "netmgr_connect", test_netmgr_connect_case },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "netmgr", init, cleanup, setup, teardown, yos_netmgr_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_netmgr(void)
{
    yunit_add_test_suites(suites);
}
