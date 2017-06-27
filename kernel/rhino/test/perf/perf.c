/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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
#include "stdio.h"


#define TASK_TEST_PRI     20
#define TASK_MAIN_PRI     15
#define TASK_TEST_STACK_SIZE 1024

#define PRECISION 1000000000

#define APB_DEFAULT_FREQ    20000000

ktask_t     *test_task;
ktask_t     *main_task;

ksem_t      *SYNhandle;
ksem_t      *SYNmainhandle;

double  Turn_to_Realtime(double counter);
void WaitForNew_tick(void);
void OS_RealTime_test(void);

extern int yunos_bsp_intc_attach_irq(int irq, int isr);
extern int yunos_bsp_intc_enable_irq(int irq);;
extern void timer0_handler(void);

extern void show_times_hdr(void);
extern void IntRealtimetest(void *arg);
extern void TaskYIELDtimeTest(void *arg);
extern void PreemptionTimetest(void *arg);
extern void MutexShufTimetest(void *arg);
extern void BinaryShufTimetest(void *arg);


double  Turn_to_Realtime(double counter)
{
    double  realtime_us;
    double US_CLK = (double)(PRECISION / APB_DEFAULT_FREQ) ;
    return realtime_us = (US_CLK * counter);
}

void WaitForNew_tick(void)
{
    sys_time_t nowtick;
    nowtick = yunos_sys_tick_get();

    while (yunos_sys_tick_get() == nowtick);
}


void OS_test(void *arg)
{
    kstat_t ret;

    WaitForNew_tick();
    yunos_sem_dyn_create(&SYNhandle, "syn_sem", 0);
    yunos_sem_dyn_create(&SYNmainhandle, "synmain_sem", 0);
    show_times_hdr();

#if 1
    ret = yunos_task_dyn_create(&test_task, "test_task", 0, TASK_TEST_PRI,
                                0, TASK_TEST_STACK_SIZE, IntRealtimetest, 1);
    yunos_sem_take(SYNhandle, YUNOS_WAIT_FOREVER);
    yunos_task_dyn_del(test_task);

    yunos_task_sleep(50);
#endif

#if 1
    yunos_task_dyn_create(&test_task, "test_task", 0, TASK_TEST_PRI,
                          0, TASK_TEST_STACK_SIZE, TaskYIELDtimeTest, 1);
    yunos_sem_take(SYNhandle, YUNOS_WAIT_FOREVER);
    yunos_task_dyn_del(test_task);
    yunos_task_sleep(50);
#endif

#if 1
    yunos_task_dyn_create(&test_task, "test_task", 0, TASK_TEST_PRI,
                          0, TASK_TEST_STACK_SIZE, PreemptionTimetest, 1);
    yunos_sem_take(SYNhandle, YUNOS_WAIT_FOREVER);
    yunos_task_dyn_del(test_task);

    yunos_task_sleep(50);
#endif

#if 1
    yunos_task_dyn_create(&test_task, "test_task", 0, TASK_TEST_PRI,
                          0, TASK_TEST_STACK_SIZE, MutexShufTimetest, 1);

    yunos_sem_take(SYNhandle, YUNOS_WAIT_FOREVER);
    yunos_task_dyn_del(test_task);

    yunos_task_sleep(50);
#endif

#if 1
    yunos_task_dyn_create(&test_task, "test_task", 0, TASK_TEST_PRI,
                          0, TASK_TEST_STACK_SIZE, BinaryShufTimetest, 1);

    yunos_sem_take(SYNhandle, YUNOS_WAIT_FOREVER);
    yunos_task_dyn_del(test_task);

    yunos_task_sleep(50);
#endif

    yunos_task_dyn_del(g_active_task);

}

void OS_RealTime_test(void)
{
    yunos_bsp_intc_enable_irq(2);
#ifndef CONFIG_TEST_PERFORMANCE
    yunos_bsp_intc_attach_irq(2, (int)timer0_handler);
#endif
    yunos_task_dyn_create(&main_task, "main_task", 0, TASK_MAIN_PRI,
                          0, TASK_TEST_STACK_SIZE, OS_test, 1);
}

