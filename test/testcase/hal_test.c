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

#include <yos/list.h>
#include <yos/kernel.h>

#include <hal/soc/timer.h>

#include <yunit.h>
#include <yts.h>

static void test_timer_cb(void *arg)
{
    int *pc = arg;
    (*pc) ++;
}

static void test_timer_case(void)
{
    int counter = 0, old_counter;
    hal_timer_t t;
    hal_timer_init(&t, 50000, 1, 0, test_timer_cb, &counter);
    yos_msleep(1000);
    YUNIT_ASSERT(counter == 0);

    hal_timer_start(&t);
    check_cond_wait(counter > 3, 2);

    hal_timer_stop(&t);
    yos_msleep(1000);

    old_counter = counter;
    yos_msleep(1000);
    YUNIT_ASSERT(counter == old_counter);
    if (counter != old_counter)
        printf("%s %d %d\n", __func__, counter, old_counter);
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
    { "timer", test_timer_case },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "hal", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_hal(void)
{
    yunit_add_test_suites(suites);
}

