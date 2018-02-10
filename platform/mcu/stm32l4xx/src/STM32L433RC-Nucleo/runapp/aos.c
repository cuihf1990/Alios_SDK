/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <k_api.h>
#include <aos/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "soc_init.h"
#define AOS_START_STACK 4096

static ktask_t demo_task_obj;
cpu_stack_t demo_task_buf[AOS_START_STACK];


#define WIFI_PRODUCT_INFO_SIZE                      ES_WIFI_MAX_SSID_NAME_SIZE

ktask_t *g_aos_init;

static kinit_t kinit;

extern int application_start(int argc, char **argv);
extern int aos_framework_init(void);
extern void board_init(void);

static void var_init()
{
    kinit.argc = 0;
    kinit.argv = NULL;
    kinit.cli_enable = 1;
}

extern void hw_start_hal(void);

#include "hal/soc/uart.h"
#include "hal/hal_uart_stm32l4.h"
#include "board.h"
uart_dev_t   uart_0;

static int default_UART_Init()
{
    uart_0.port                = PORT_UART1;
    uart_0.config.baud_rate    = STDIO_UART_BUADRATE;
    uart_0.config.data_width   = DATA_WIDTH_8BIT;
    uart_0.config.parity       = NO_PARITY;
    uart_0.config.stop_bits    = STOP_BITS_1;
    uart_0.config.flow_control = FLOW_CONTROL_DISABLED;

    return hal_uart_init(&uart_0);
}

static void sys_init(void)
{

    stm32_soc_init();
    default_UART_Init();

#ifdef BOOTLOADER
    main();
#else
    hw_start_hal();
    //board_init();
    var_init();
    aos_kernel_init(&kinit);
#endif
}


static void sys_start(void)
{
    aos_init();
    //krhino_task_dyn_create(&g_aos_init, "aos-init", 0, AOS_DEFAULT_APP_PRI, 0, AOS_START_STACK, (task_entry_t)sys_init, 1);
    krhino_task_create(&demo_task_obj, "aos-init", 0,AOS_DEFAULT_APP_PRI, 
        50, demo_task_buf, AOS_START_STACK, (task_entry_t)sys_init, 1);
    
    aos_start();
}

int main(void)
{
    sys_start();
    return 0;
}

