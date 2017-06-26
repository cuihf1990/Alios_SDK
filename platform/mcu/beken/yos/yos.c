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

#include <yos.h>
#include <k_api.h>
#include <yos/kernel.h>

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

    yunos_task_dyn_create(&g_yos_init, "yos-init", 0, YOS_DEFAULT_APP_PRI, 0, YOS_START_STACK, yos_init, 1);

    yunos_start();
}

