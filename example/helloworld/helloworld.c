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

int application_start(int argc, char *argv[])
{
    yos_framework_init();
    yos_post_delayed_action(1000, app_delayed_action, NULL);
    yos_loop_run();
}

