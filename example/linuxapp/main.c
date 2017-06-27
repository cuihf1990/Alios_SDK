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

#include <stdio.h>
#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/network.h>
#include <yos/log.h>
#include <strings.h>
#include <netmgr.h>

struct cookie {
    int flag;
};

static void app_delayed_action(void *arg)
{
    struct cookie *cookie = arg;
    LOG("%s - %s", __func__, yos_task_name());
    if (cookie->flag != 0) {
        yos_post_delayed_action(10000, app_delayed_action, arg);
    }
    else {
        yos_schedule_call(app_delayed_action, arg);
    }
    cookie->flag ++;
}

static void handle_event(input_event_t *event, void *arg)
{
    if (event->type != EV_WIFI) {
        return;
    }

    if (event->code != CODE_WIFI_ON_GOT_IP) {
        return;
    }

    struct hostent *hent = gethostbyname("www.taobao.com");
    LOG("%s - %s", __func__, yos_task_name());
    if(hent) {
        LOG("%s - %s", __func__, hent->h_name);
    }

    yos_post_delayed_action(1000, app_delayed_action, arg);
}

int application_start(void)
{
    struct cookie *cookie = yos_malloc(sizeof(*cookie));
    bzero(cookie, sizeof(*cookie));

    yos_register_event_filter(EV_WIFI, handle_event, cookie);

    netmgr_init();
    netmgr_start(true);
	
#ifdef CONFIG_CMD_BENCHMARKS
    extern void benchmark_cli_init();
    benchmark_cli_init();
#endif

    yos_loop_run();
    /* never return */

    return 0;
}

