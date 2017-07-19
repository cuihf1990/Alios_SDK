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


#include "umesh_types.h"
#include "utilities/task.h"

typedef void (*yos_call_t)(void *);
extern int yos_schedule_call(yos_call_t f, void *arg);
ur_error_t umesh_task_schedule_call(umesh_task_t task, void *arg)
{
    int ret;
    ur_error_t error = UR_ERROR_NONE;

    ret = yos_schedule_call(task, arg);
    if (ret < 0) {
        error = UR_ERROR_FAIL;
    }

    return error;
}
