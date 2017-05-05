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

#include <k_api.h>
#include <k_mm_region.h>
#include <test_fw.h>
#include "mm_region_test.h"

ktask_t   *task_mm_region;
ktask_t   *task_mm_region_co;

static uint8_t data[MM_REGION_TOTAL_SIZE] = {0};

k_mm_region_t regions[]={
    {data,      MM_REGION_0_SIZE},
    {data+16,   MM_REGION_1_SIZE},
    {data+500,  MM_REGION_2_SIZE},
};

k_mm_region_t regions2[]={
    {data,      MM_REGION_0_SIZE},
    {data+32,   MM_REGION_1_SIZE},
    {data+512,  MM_REGION_2_SIZE},
};

static test_func_t *module_runner;
static const char  *module_name;
static uint8_t      module_casenum;

static const test_case_t mm_region_case_runner[] = {
    mm_region_break_test,
    NULL
};

void mm_region_test(void)
{
    if (test_case_register((test_case_t *)mm_region_case_runner) == 0) {
        test_case_run();
        test_case_unregister();
    }
}

void task_mm_region_entry_register(const char *name, test_func_t *runner, uint8_t casenum)
{
    module_runner  = runner;
    module_name    = name;
    module_casenum = casenum;
}

void task_mm_region_entry(void *arg)
{
    test_func_t *runner;
    uint8_t      caseidx;
    char         name[64];
    uint8_t      casenum;

    runner  = (test_func_t *)module_runner;
    casenum = module_casenum;
    caseidx = 0;

    while (1) {
        if (*runner == NULL) {
            break;
        }

        if (casenum > 2) {
            caseidx++;
            sprintf(name, "%s_%d", module_name, caseidx);
        } else {
            sprintf(name, "%s", module_name);
        }

        if ((*runner)() == 0) {
            test_case_success++;
            PRINT_RESULT(name, PASS);
        } else {
            test_case_fail++;
            PRINT_RESULT(name, FAIL);
        }

        runner++;
    }

    next_test_case_notify();
    yunos_task_dyn_del(g_active_task);
}

