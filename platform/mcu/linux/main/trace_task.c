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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fifo.h>
#include <hal/trace.h>

#if (YUNOS_CONFIG_TRACE > 0)

#define TRACE_TASK_STACK_SIZE 512
extern struct k_fifo trace_fifo;
static ktask_t *trace_task;
static uint32_t trace_buf[1025];

static void trace_entry(void *arg)
{
    int fh1;
    uint32_t len;

    printf("trace main entry\n");

    fh1 = (int)trace_hal_init();

    while (1) {

        len = fifo_out_all(&trace_fifo, &trace_buf[1]);
        
        if (len > 0) {
            trace_buf[0] = len;
            trace_hal_send((void *)fh1, trace_buf, len + 4);
        }

        yunos_task_sleep(2);
    }
}

void trace_start(int flag)
{
    if (flag > 0) {
        if (yunos_task_dyn_create(&trace_task, "task_trace_test0", NULL, 3,
                                  0, TRACE_TASK_STACK_SIZE, trace_entry, 1) != YUNOS_SUCCESS) {
            printf("trace task creat fail \n");
        }
    }
}

#else

void trace_start(int flag)
{
    (void)flag;

}

#endif

