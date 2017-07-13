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

static void el_int_hdl(void *arg)
{
    printf("easylink interrupt handler\r\n");
}

void application_start(void)
{
    hal_gpio_enable_irq(2, IRQ_TRIGGER_FALLING_EDGE, el_int_hdl, NULL);

    hal_gpio_init(12, OUTPUT_PUSH_PULL);
    for(;;)
    {
        hal_gpio_output_trigger(12);
        yos_msleep(1000);
    }
}

