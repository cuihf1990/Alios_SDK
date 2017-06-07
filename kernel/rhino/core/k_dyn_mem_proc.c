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

#if (YUNOS_CONFIG_KOBJ_DYN_ALLOC > 0)
void dyn_mem_proc_task(void *arg)
{
    void   *recv_msg;
    kstat_t ret;

    (void)arg;

    while (1) {
        ret = yunos_queue_recv(&g_dyn_queue, YUNOS_WAIT_FOREVER, &recv_msg);
        if (ret != YUNOS_SUCCESS) {
            k_err_proc(YUNOS_DYN_MEM_PROC_ERR);
        }

        yunos_mm_free(recv_msg);
    }
}

void dyn_mem_proc_task_start(void)
{
    ktask_t *dyn_mem_task;

    yunos_task_dyn_create(&dyn_mem_task, "dyn_mem_proc_task", 0,
                          YUNOS_CONFIG_K_DYN_MEM_TASK_PRI,
                          0, YUNOS_CONFIG_K_DYN_TASK_STACK, dyn_mem_proc_task, 1);

}
#endif

