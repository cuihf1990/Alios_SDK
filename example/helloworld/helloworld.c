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

#include <yos/framework.h>

#include "hal/soc/soc.h"
#include "helloworld.h"

static void app_delayed_action(void *arg)
{
    printf("%s:%d %s\r\n", __func__, __LINE__, yos_task_name());
    yos_post_delayed_action(5000, app_delayed_action, NULL);
}

#ifdef YOS_BINS
extern unsigned int _app_data_ram_begin;
extern unsigned int _app_data_ram_end;
extern unsigned int _app_data_flash_begin;
extern unsigned int _app_bss_start;
extern unsigned int _app_bss_end;
extern unsigned int _app_heap_start;
extern unsigned int _app_heap_end;
extern int application_start(int argc, char **argv);

struct app_info_t {
     int (*application_start)(int argc, char *argv[]);
     unsigned int data_ram_start;
     unsigned int data_ram_end;
     unsigned int data_flash_begin;
     unsigned int bss_start;
     unsigned int bss_end;
     unsigned int heap_start;
     unsigned int heap_end;
};

__attribute__ ((section(".app_info"))) struct app_info_t app_info = {
    application_start,
    &_app_data_ram_begin,
    &_app_data_ram_end,
    &_app_data_flash_begin,
    &_app_bss_start,
    &_app_bss_end,
    &_app_heap_start,
    &_app_heap_end
};
#endif

int application_start(int argc, char *argv[])
{
    yos_framework_init();
    yos_post_delayed_action(1000, app_delayed_action, NULL);
    yos_loop_run();
}

