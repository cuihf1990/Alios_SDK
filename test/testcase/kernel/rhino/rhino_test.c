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

#include <unistd.h>
#include <yos/kernel.h>

#include "yunit.h"
#include "test_fw.h"

static int init(void)
{
    test_case_init();
    return 0;
}

static int cleanup(void)
{
    test_case_cleanup();
    return 0;
}

static void setup(void)
{
    return;
}

static void teardown(void)
{
    return;
}

static int item = 0;

void rhino_ytest_fn(void)
{
    YUNIT_ASSERT(test_case_fail ==  0);
}

void test_rhino(void)
{

    void* suite = yunit_add_test_suite("rhino", init, cleanup, setup, teardown);

    for (item = 0;; item++) {
        if ((test_fw_map[item].name == NULL) || (test_fw_map[item].fn == NULL)) {
            break;
        }

        yunit_add_test_case(suite, test_fw_map[item].name, test_fw_map[item].fn);
    }

    yunit_add_test_case(suite, "rhino test stats", rhino_ytest_fn);
}

