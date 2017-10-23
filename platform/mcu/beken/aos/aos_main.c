/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos_main.h>
#include <k_api.h>
#include <aos/kernel.h>

#define AOS_START_STACK 2048

ktask_t *g_aos_init;

extern void board_init(void);
extern int aos_kernel_init(void);


static void sys_init(void)
{
    int i = 0;

    soc_system_init();

#ifdef BOOTLOADER
    main();
#else

    board_init();

    aos_kernel_init();

#endif
}

void sys_start(void)
{
    aos_init();

    soc_driver_init();

    krhino_task_dyn_create(&g_aos_init, "aos-init", 0, AOS_DEFAULT_APP_PRI, 0, AOS_START_STACK, sys_init, 1);

    aos_start();
}

