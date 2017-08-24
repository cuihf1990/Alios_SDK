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

#include <yos.h>
#include <k_api.h>
#include <yos/kernel.h>

#define YOS_START_STACK 2048

ktask_t *g_yos_init;

extern void board_init(void);

extern int application_start(int argc, char **argv);

#ifdef YOS_BINS
extern void *g_syscall_tbl[];
extern char  app_info_addr;

extern k_mm_head  *g_kmm_head;

struct app_info_t *app_info = (struct app_info_t *)&app_info_addr;

static void application_init(void)
{
    memcpy((void *)(app_info->data_ram_start), (void *)(app_info->data_flash_begin),
           app_info->data_ram_end - app_info->data_ram_start);
    memset((void *)(app_info->bss_start), 0, app_info->bss_end - app_info->bss_start);

    yunos_add_mm_region(g_kmm_head, (void *)(app_info->heap_start),
                        app_info->heap_end - app_info->heap_start);

    yunos_mm_leak_region_init((void *)(app_info->data_ram_start), (void *)(app_info->data_ram_end));
    yunos_mm_leak_region_init((void *)(app_info->bss_start), (void *)(app_info->bss_end));
}
#endif

static void yos_init(void)
{
    int i = 0;

    soc_system_init();

#ifdef BOOTLOADER
    main();
#else

    board_init();

    vfs_init();
    vfs_device_init();

    for (i = 0; i < 10; i++) {
        vflash_register_partition(i);
    }

    yos_cli_init();
    yos_kv_init();
    yos_loop_init();

#ifdef YOS_BINS
    application_init();

    if (app_info->app_entry) {
        app_info->app_entry((int)g_syscall_tbl, NULL);
    }
#else
    application_start(0, NULL);
#endif

#endif
}

void yos_start(void)
{
    yunos_init();

    soc_driver_init();

    yunos_task_dyn_create(&g_yos_init, "yos-init", 0, YOS_DEFAULT_APP_PRI, 0, YOS_START_STACK, yos_init, 1);

    yunos_start();
}

