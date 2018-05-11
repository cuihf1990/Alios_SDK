/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <aos/aos.h>
#include <aos/yloop.h>
#include "netmgr.h"
#include "iot_import.h"
#include "iot_export.h"
#include "iot_export_mqtt.h"
#include "linkkit_app.h"
#include "Uart_Device.h"

#ifdef CSP_LINUXHOST
#include <signal.h>
#endif

static int linkkit_started = 0;      ////是否连接服务器
static int awss_running = 0;         ////是否本地发现过设备

void reboot_system(void *parms);
static void wifi_service_event(input_event_t *event, void *priv_data) {
    if (event->type != EV_WIFI) {
        return;
    }

    if (event->code != CODE_WIFI_ON_GOT_IP) {
        return;
    }

    netmgr_ap_config_t config;
    memset(&config, 0, sizeof(netmgr_ap_config_t));
    netmgr_get_ap_config(&config);
    LOG("wifi_service_event config.ssid %s", config.ssid);
    if(strcmp(config.ssid, "adha") == 0 || strcmp(config.ssid, "aha") == 0) {
        //clear_wifi_ssid();
        return;
    }
    
    /* reduce the time of net config for 3080/3165 */
#if 0
    if(awss_running) {
        aos_post_delayed_action(200,reboot_system,NULL);
        return;
    }
#endif
    if (!linkkit_started) {
        awss_success_notify();
        linkkit_app();
        linkkit_started = 1;
    }
}

void reboot_system(void *parms)
{
   LOG("reboot system");
   aos_reboot();
}

static void cloud_service_event(input_event_t *event, void *priv_data) {
    static uint8_t awss_reported=0;
    if (event->type != EV_YUNIO) {
        return;
    }

    LOG("cloud_service_event %d", event->code);

    if (event->code == CODE_YUNIO_ON_CONNECTED) {
        LOG("user sub and pub here");
        if(!awss_reported) {
            awss_report_cloud();
            awss_reported=1;
        }
        return;
    }

    if (event->code == CODE_YUNIO_ON_DISCONNECTED) {
    }
}

static void start_netmgr(void *p)
{
    netmgr_start(true);
    //aos_task_exit(0);
}

extern int awss_report_reset();

void do_awss_active()
{
    LOG("do_awss_active %d\n", awss_running);
    awss_running = 1;
    awss_config_press();
}

void do_awss_reset()
{
    int  ret = -1;
    if(linkkit_started) {
	aos_task_new("reset", awss_report_reset, NULL, 2048);   ///// 解绑服务器信息
    }
    netmgr_clear_ap_config();   ////清除配网信息
    LOG("SSID cleared.==> Please reboot the system.\n");
    
    ret = aos_post_delayed_action(1000,reboot_system,NULL);   ////系统重启
    LOG("start action = %d",ret);
    if(ret != 0){
        aos_msleep(2000);
        aos_reboot();
    }

}

void linkkit_key_process(input_event_t *eventinfo, void *priv_data)
{
    if (eventinfo->type != EV_KEY) {
        return;
    }
    LOG("awss config press %d\n", eventinfo->value);

    if (eventinfo->code == CODE_BOOT) {
        if (eventinfo->value == VALUE_KEY_CLICK) {
            do_awss_active();      /////激活本地发现    单次有效
        } else if(eventinfo->value == VALUE_KEY_LTCLICK) {
            do_awss_reset();     /////解绑模块信息
        }
    }
}

void linkkit_post_awss_reset(void)
{
    aos_schedule_call(do_awss_reset, NULL);
}

void linkkit_post_awss_active(void)
{
    aos_schedule_call(do_awss_active, NULL);
}

#ifdef CONFIG_AOS_CLI
static void handle_reset_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    linkkit_post_awss_reset();
}

static void handle_active_cmd(char *pwbuf, int blen, int argc, char **argv)
{
    linkkit_post_awss_active();
}

static struct cli_command resetcmd = {
    .name = "reset",
    .help = "factory reset",
    .function = handle_reset_cmd
};

static struct cli_command ncmd = {
    .name = "active_awss",
    .help = "active_awss [start]",
    .function = handle_active_cmd
};
#endif

int application_start(int argc, char **argv)
{
    
#ifdef CSP_LINUXHOST
    signal(SIGPIPE, SIG_IGN);
#endif

    aos_set_log_level(AOS_LL_NONE);

    app_uart_init();

#if 0
    netmgr_ap_config_t config;
    strncpy(config.ssid, "WDNEZPL", sizeof("WDNEZPL"));
   // strncpy(config.pwd, "123456789", sizeof("123456789"));
    netmgr_set_ap_config(&config);   
#endif    
    
    netmgr_init();
    aos_register_event_filter(EV_KEY, linkkit_key_process, NULL);    ////按键事件
    aos_register_event_filter(EV_WIFI, wifi_service_event, NULL);     ///// wifi配网事件
    aos_register_event_filter(EV_YUNIO, cloud_service_event, NULL);     /////服务器事件

#ifdef CONFIG_AOS_CLI
    aos_cli_register_command(&resetcmd);
    aos_cli_register_command(&ncmd);
#endif

    aos_task_new("netmgr", start_netmgr, NULL, 4096);

    aos_loop_run();

    return 0;
}
