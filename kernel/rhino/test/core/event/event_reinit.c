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
#include <test_fw.h>
#include "event_test.h"

#define MODULE_NAME "event_reinit"

#define TEST_FLAG   0x5a5a5a5a

static uint8_t event_reinit_case1(void)
{
    kstat_t ret;

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.blk_obj.obj_type == YUNOS_EVENT_OBJ_TYPE);
    MYASSERT(test_event.blk_obj.blk_policy == BLK_POLICY_PRI);
    MYASSERT(test_event.blk_obj.name != NULL);
    MYASSERT(test_event.flags == TEST_FLAG);

    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    ret = yunos_event_create(&test_event, MODULE_NAME, TEST_FLAG & 0x0000ffff);
    MYASSERT(ret == YUNOS_SUCCESS);
    MYASSERT(test_event.blk_obj.obj_type == YUNOS_EVENT_OBJ_TYPE);
    MYASSERT(test_event.flags == (TEST_FLAG & 0x0000ffff));

    ret = yunos_event_del(&test_event);
    MYASSERT(ret == YUNOS_SUCCESS);

    return 0;
}

static const test_func_t event_func_runner[] = {
    event_reinit_case1,
    NULL
};

void event_reinit_test(void)
{
    kstat_t ret;

    task_event_entry_register(MODULE_NAME, (test_func_t *)event_func_runner, sizeof(event_func_runner)/sizeof(test_func_t));

    ret = yunos_task_dyn_create(&task_event, MODULE_NAME, 0, TASK_EVENT_PRI,
                                 0, TASK_TEST_STACK_SIZE, task_event_entry, 1);
    if ((ret != YUNOS_SUCCESS) && (ret != YUNOS_STOPPED)) {
        test_case_fail++;
        PRINT_RESULT(MODULE_NAME, FAIL);
    }
}

