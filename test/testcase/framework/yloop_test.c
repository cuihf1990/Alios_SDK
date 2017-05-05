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

#include <yos/kernel.h>
#include <yos/framework.h>

#include <yunit.h>
#include <yts.h>

#define TASK1 "task1"
#define TASK2 "task2"

struct task_cookie {
    yos_loop_t loop;
    int flag;
    const char *name;
};

#define T_ACTION2_FLAG      0x01
#define T_WORK_DONE_FLAG    0x02
#define T_WORK_FLAG         0x04
#define T_ACTION_TERM_FLAG  0x08
#define T_ALL 0x0f

static uint32_t done_flag;

#define yos_task_name() g_active_task->task_name

static void app_delayed_action(void *arg)
{
    struct task_cookie *cookie = arg;

    YUNIT_ASSERT(strcmp(yos_task_name(), cookie->name) == 0);
    if (cookie->flag == 0) {
        yos_post_delayed_action(1000, app_delayed_action, arg);
    }
    else if (cookie->flag == 1) {
        yos_schedule_call(app_delayed_action, arg);
    }
    else {
        yos_loop_exit();
    }
    cookie->flag ++;
}

static void app_delayed_action2(void *arg)
{
    struct task_cookie *cookie = arg;

    YUNIT_ASSERT(strcmp(yos_task_name(), cookie->name) == 0);

    done_flag |= T_ACTION2_FLAG;
}

static struct task_cookie cookie1 = {
    .name = TASK1,
};

static struct task_cookie cookie2 = {
    .name = TASK2,
};

static void work_done(void *arg)
{
    struct task_cookie *cookie = arg;
    YUNIT_ASSERT(strcmp(yos_task_name(), cookie->name) == 0);
    yos_loop_exit();

    done_flag |= T_WORK_DONE_FLAG;
}

static void mywork(void *arg)
{
    struct task_cookie *cookie = arg;
    YUNIT_ASSERT(strcmp(yos_task_name(), cookie->name) != 0);

    done_flag |= T_WORK_FLAG;
}

static void app_main_entry(void *arg)
{
    struct task_cookie *cookie = arg;
    YUNIT_ASSERT(strcmp(yos_task_name(), cookie->name) == 0);

    cookie->loop = yos_current_loop();

    yos_post_delayed_action(1000, app_delayed_action, cookie);

    yos_loop_run();
    yos_loop_run();

    yos_schedule_work(mywork, arg, work_done, arg);
    yos_loop_run();
}

static void action_after_terminated(void *arg)
{
    printf("%s:%d - %s\n", __func__, __LINE__, yos_task_name());
    yos_loop_exit();

    done_flag |= T_ACTION_TERM_FLAG;
}

static void app_second_entry(void *arg)
{
    struct task_cookie *cookie = arg;
    YUNIT_ASSERT(strcmp(yos_task_name(), cookie->name) == 0);

    yos_loop_init();

    cookie->loop = yos_current_loop();
    yos_post_delayed_action(1000, app_delayed_action, cookie);

    yos_loop_run();

    YUNIT_ASSERT(cookie1.loop != NULL);
    yos_loop_schedule_call(cookie1.loop, app_delayed_action2, &cookie1);
    yos_loop_destroy();

    yos_schedule_call(action_after_terminated, NULL);
}

static void test_simple_case(void)
{
    yos_task_new(TASK1, app_main_entry, &cookie1, 8192);
    yos_task_new(TASK2, app_second_entry, &cookie2, 8192);
    check_cond_wait(done_flag == T_ALL, 10);
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
    { "simple", test_simple_case },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "yloop", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_yloop(void)
{
    yunit_add_test_suites(suites);
}

