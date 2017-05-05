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

#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo_144.h"
#include "stdio.h"
#include "main.h"

#include "k_api.h"

#define INIT_TASK_STACK_SIZE 1024

ktask_t     init_task1;
cpu_stack_t init_task_stack1[INIT_TASK_STACK_SIZE];

ktask_t     init_task2;
cpu_stack_t init_task_stack2[INIT_TASK_STACK_SIZE];

extern UART_HandleTypeDef UartHandle;

extern void apps_start(void);

#ifdef CONFIG_YOC_UT_RHINO
extern void test_case_task_start(void);
#endif

#ifdef CONFIG_YOC_YSH
extern void ysh_task_start(void);
#endif

static void init_task_entry1(void *arg)
{
    HAL_Init();

    SystemClock_Config();

    BSP_LED_Init(LED2);

    __disable_irq();

    HAL_SYSTICK_Config(1800000);

    UartHandle.Instance          = USARTx;
    UartHandle.Init.BaudRate     = 19200;
    UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits     = UART_STOPBITS_1;
    UartHandle.Init.Parity       = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode         = UART_MODE_TX_RX;
    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&UartHandle) != HAL_OK) {
        Error_Handler();
    }

    HAL_SYSTICK_Config(1800000); /* HZ == 100 */

    __enable_irq();

#ifdef CONFIG_YOC_UT_RHINO
    test_case_task_start();
#endif

#ifdef CONFIG_YOC_YSH
    ysh_init();
    ysh_task_start();
#endif

    printf("TASK1: YoC Running!\n");
    yunos_task_sleep(300);

    yunos_task_del(&init_task1);
}

static void init_task_entry2(void *arg)
{
    printf("TASK2: YoC Running!\n");
    yunos_task_sleep(300);

    yunos_task_del(&init_task2);
}

void soc_apps_entry(void)
{
    yunos_task_create(&init_task1, "init_task1", NULL, 9,
                      0, init_task_stack1, INIT_TASK_STACK_SIZE,
                      init_task_entry1, 1);

    yunos_task_create(&init_task2, "init_task2", NULL, 9,
                      0, init_task_stack2, INIT_TASK_STACK_SIZE,
                      init_task_entry2, 1);

    apps_start();
}

