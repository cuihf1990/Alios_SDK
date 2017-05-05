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

#include "queue_test.h"

void queue_test(void)
{
    task_queue_back_send_test();
    next_test_case_wait();

    task_queue_front_send_test();
    next_test_case_wait();

    task_queue_all_send_test();
    next_test_case_wait();

    task_queue_nowait_recv_test();
    next_test_case_wait();

    task_queue_notify_set_test();
    next_test_case_wait();

    task_queue_is_full_test();
    next_test_case_wait();

    task_queue_flush_test();
    next_test_case_wait();

    task_queue_del_test();
    next_test_case_wait();

    task_queue_info_get_test();
    next_test_case_wait();
}

