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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fifo.h>
#include <trace_hal.h>

#if (YUNOS_CONFIG_TRACE > 0)

#define TASK_TEST_STACK_SIZE 512
#define LOOP_CNT 1

static ktask_t *task_trace_test_0;
extern struct k_fifo trace_fifo;

static uint32_t trace_buf[1025];
static uint32_t time_cnt;


static void trace_entry(void *arg)
{
    uint32_t len;
    int fh1;

    printf("trace test entry\n");

    fh1 = (int)trace_hal_init();

    while (1) {

        len = fifo_out_all(&trace_fifo, &trace_buf[1]);

        if (len > 0) {
            trace_buf[0] = len;
            trace_hal_send((void *)fh1, trace_buf, len + 4);
        }

        yunos_task_sleep(1);

        time_cnt++;
        if (time_cnt >= 180 * YUNOS_CONFIG_TICKS_PER_SECOND) {
            printf("trace test success\n");
            trace_hal_deinit((void *)fh1);
            yunos_task_dyn_del(NULL);
        }
    }
}

void trace_test()
{
    if (yunos_task_dyn_create(&task_trace_test_0, "task_trace_test0", NULL, 7,
                              0, TASK_TEST_STACK_SIZE, trace_entry, 1) != YUNOS_SUCCESS) {
        printf("task_del_test 0 creat fail \n");
    }
}

#endif

