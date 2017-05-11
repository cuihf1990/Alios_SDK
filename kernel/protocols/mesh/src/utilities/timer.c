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

#include "yos/framework.h"
#include "yos/kernel.h"

#include "utilities/timer.h"

ur_timer_t ur_start_timer(uint32_t dt, timer_handler_t handler, void *args)
{
    yos_post_delayed_action(dt, handler, args);
    return handler;
}

void ur_stop_timer(ur_timer_t *timer, void *args)
{
    timer_handler_t handler;

    if (*timer != NULL) {
        handler = (timer_handler_t)(*timer);
        yos_cancel_delayed_action(-1, handler, args);
        *timer = NULL;
    }
}

uint32_t ur_get_now(void)
{
    uint64_t now;

    now = yos_now();
    now = now / 1000000;
    return (uint32_t)now;
}
