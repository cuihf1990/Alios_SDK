/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <yos.h>
#include <k_api.h>
#include <yos/yos.h>

#define YOS_START_STACK 2048

ktask_t *g_yos_init;

extern int application_start(int argc, char **argv);

static void yos_init(void)
{
    soc_system_init();

#ifdef BOOTLOADER
    main();
#else
    yos_framework_init();

    application_start(0, NULL);
#endif
}

void yos_start(void)
{
    yunos_init();

    soc_driver_init();

    yunos_task_dyn_create(&g_yos_init, "yos-init", 0, 10, 0, YOS_START_STACK, yos_init, 1);

    yunos_start();
}

