/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <stdio.h>
#include <stdlib.h>

#define TASK1_STACK_SIZE 512

extern void stm32_soc_init(void);
static ktask_t task1_obj;
cpu_stack_t task1_buf[TASK1_STACK_SIZE];

void task1(void *arg)
{
    stm32_soc_init();
    krhino_task_sleep(3);
    
    while (1)
    {
        printf("hello world!\n");
        krhino_task_sleep(10);
        
    };
}

int main(void)
{
    krhino_init();
    krhino_task_create(&task1_obj, "task1", 0,20, 50, task1_buf, 512, task1, 1);
    krhino_start();
    return 0;
}

