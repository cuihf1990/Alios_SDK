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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <k_api.h>
#include <yos/kernel.h>
#include <poll.h>
#include <hal/soc/timer.h>


extern void hw_start_hal(void);

void csp_os_init(void)
{
    hw_start_hal();

    yunos_init();
}

void csp_os_start(void)
{
    yunos_start();
}

void csp_printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}

uint32_t yoc_get_free_heap_size(void)
{
    return 0u;
}

#define us2tick(us) \
    ((us * YUNOS_CONFIG_TICKS_PER_SECOND + 999999) / 1000000)

static void _timer_cb(void *timer, void *arg)
{
    hal_timer_t *tmr = arg;
    tmr->cb(tmr->arg);
}

void hal_timer_init(hal_timer_t *tmr, unsigned int period, unsigned char auto_reload, unsigned char ch, hal_timer_cb_t cb, void *arg)
{
    (void)ch;
    bzero(tmr, sizeof(*tmr));
    tmr->cb = cb;
    tmr->arg = arg;
    if (auto_reload > 0u) {
        yunos_timer_dyn_create((ktimer_t **)&tmr->priv, "hwtmr", _timer_cb,
                                us2tick(period), us2tick(period), tmr, 0);
    }
    else {
        yunos_timer_dyn_create((ktimer_t **)&tmr->priv, "hwtmr", _timer_cb,
                                us2tick(period), 0, tmr, 0);
    }
}

int hal_timer_start(hal_timer_t *tmr)
{
    return yunos_timer_start(tmr->priv);
}

void hal_timer_stop(hal_timer_t *tmr)
{
    yunos_timer_stop(tmr->priv);
    yunos_timer_dyn_del(tmr->priv);
    tmr->priv = NULL;
}
