/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>
#include "hal.h"
#include "hal_gpio_stm32l4.h"

#define DEMO_TASK_STACKSIZE    512 //512*cpu_stack_t = 2048byte
#define DEMO_TASK_PRIORITY     20
#define UART_DATA_BYTES 10

extern void stm32_soc_init(void);
void hal_uart_test(void);
void hal_gpio_test(void);
static ktask_t demo_task_obj;
cpu_stack_t demo_task_buf[DEMO_TASK_STACKSIZE];

extern uart_dev_t uart_dev_com1;
extern gpio_dev_t gpio_dev_GPIOB_PIN13;

void demo_task(void *arg)
{   
    while (1)
    {
			  hal_uart_test();
        hal_gpio_test();
        krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND/50);
    };
}

int main(void)
{
    krhino_init();
    krhino_task_create(&demo_task_obj, "demo_task", 0,DEMO_TASK_PRIORITY, 
        50, demo_task_buf, DEMO_TASK_STACKSIZE, demo_task, 1);

    stm32_soc_init();
	
    krhino_start();
    
    return 0;
}

void hal_uart_test(void)
{
    char readbuf[UART_DATA_BYTES] = {0};
		uint32_t recBytes = 0;
	  int ret = -1;

		/* receive a message of 10 bytes and sent out through the uart */
		ret = hal_uart_recv(&uart_dev_com1, readbuf, UART_DATA_BYTES, &recBytes, 10);

	  if((ret == 0) && (recBytes == UART_DATA_BYTES))
    {
        hal_uart_send(&uart_dev_com1, readbuf, UART_DATA_BYTES, 10);
    }
}

void hal_gpio_test(void)
{
    hal_gpio_output_toggle(&gpio_dev_GPIOB_PIN13);
}
