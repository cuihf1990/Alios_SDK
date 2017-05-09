/**
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 */

/**
 *                      caution
 * linuxhost hw.c won't use any lwip functionalities,
 * disable WITH_LWIP to avoid close() -> lwip_close()
 */
#undef WITH_LWIP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <hal/hal.h>
#include <csp.h>
#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/log.h>

#define TAG "hw"

extern hal_sensor_module_t linuxhost_sensor_module;

void hw_start_hal(void)
{
#ifdef CONFIG_YOC_URADAR_MESH
    extern void linuxhost_hal_urmesh_register(void);
    linuxhost_hal_urmesh_register();
#endif
    hal_sensor_register_module(&linuxhost_sensor_module);

    /* Do YOC startup */
    yoc_hal_init();
}
