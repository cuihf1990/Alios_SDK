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
#include "comb_test.h"

static const test_case_t comb_case_runner[] = {
    sem_event_coopr_test,
    sem_buf_queue_coopr_test,
    sem_mutex_coopr_test,
    comb_all_coopr_test,
    sem_queue_coopr_test,
    mutex_queue_coopr_test,
    mutex_buf_queue_coopr_test,
    NULL
};

void comb_test(void)
{
   if (test_case_register((test_case_t *)comb_case_runner) == 0) {
       test_case_run();
       test_case_unregister();
   }
}

