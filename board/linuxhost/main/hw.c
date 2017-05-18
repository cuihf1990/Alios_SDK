/**
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 */

/**
 *                      caution
 * linuxhost hw.c won't use any lwip functionalities,
 * disable WITH_LWIP to avoid close() -> lwip_close()
 */
#undef WITH_LWIP

#include <hal/hal.h>
#include <yos/kernel.h>
#include <yos/framework.h>

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
