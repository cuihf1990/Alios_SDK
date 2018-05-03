/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <k_api.h>
#include <aos/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f4xx.h"
#include "hal/soc/soc.h"
#include "board.h"

#define AOS_START_STACK 2048

#define WIFI_PRODUCT_INFO_SIZE                      ES_WIFI_MAX_SSID_NAME_SIZE

ktask_t *g_aos_init;

static kinit_t kinit;

extern int application_start(int argc, char **argv);
extern int aos_framework_init(void);
extern void board_init(void);

static gpio_dev_t gpio_key_boot, gpio_key_status;
/* For QC test */
static void board_qc_check(void)
{
    uint32_t gpio_value = 1;

    gpio_key_boot.port = BOOT_SEL;
    gpio_key_boot.config = INPUT_PULL_UP;
    hal_gpio_init(&gpio_key_boot);
    hal_gpio_input_get(&gpio_key_boot, &gpio_value);

    if (gpio_value != 0)
    {
        return;
    }

    gpio_value = 1;
    gpio_key_status.port = MFG_SEL;
    gpio_key_status.config = INPUT_PULL_UP;
    hal_gpio_init(&gpio_key_status);
    hal_gpio_input_get(&gpio_key_status, &gpio_value);
    if (gpio_value != 0)
    {
        return;
    }

    // QC:
    printf("Enter QC mode\r\n");
    qc_test();
    return;
}

static void hal_init()
{
    board_init();
}

static void var_init()
{
    kinit.argc = 0;
    kinit.argv = NULL;
    kinit.cli_enable = 1;
}

extern void hw_start_hal(void);

static void sys_init(void)
{
    
    init_architecture();
    init_platform();

#ifdef BOOTLOADER
    main();
#else
    hal_init();
    hw_start_hal();
    board_qc_check();

    var_init();
    aos_kernel_init(&kinit);
#endif
}


static void sys_start(void)
{
    aos_init();
    krhino_task_dyn_create(&g_aos_init, "aos-init", 0, AOS_DEFAULT_APP_PRI, 0, AOS_START_STACK, (task_entry_t)sys_init, 1);
    aos_start();
}

#if defined (__CC_ARM) /* Keil / armcc */
int main(void)
{
    sys_start();
    return 0;
}
#else

extern uint32_t g_pfnVectors[];

void entry_main(void)
{
    SCB->VTOR = (unsigned long) &g_pfnVectors;
    /* Enable CPU Cycle counting */
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    init_clocks();
    sys_start();
}
#endif

