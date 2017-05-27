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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <yos/framework.h>
#include <yos/log.h>

#include "netmgr.h"

#define TAG "netmgr"

#define DEMO_AP_SSID "yos"
#define DEMO_AP_PASSWORD "yos__yos"

#define MAX_RETRY_CONNECT 120
#define RETRY_INTERVAL_MS 500

typedef struct {
    char ssid[33];
    char pwd[65];
} yos_persistent_conf_wifi_t;

typedef struct {
    netmgr_ap_config_t         ap_config;
    hal_wifi_module_t          *wifi_hal_mod;
    autoconfig_plugin_t        *autoconfig_chain;
    monitor_data_cb_t          monitor_dt_cb;
    int32_t                    ipv4_owned;
    int8_t                     disconnected_times;
    yos_persistent_conf_wifi_t persistent_conf_wifi;
    bool                       auto_start_smartconfig;
} netmgr_cxt_t;

static netmgr_cxt_t        g_netmgr_cxt;
static autoconfig_plugin_t g_def_smartconfig;

static void netmgr_wifi_config_start(void);
static void add_autoconfig_plugin(autoconfig_plugin_t *plugin);
static void del_first_autoconfig_plugin(void);
static int32_t has_valid_ap(void);
static autoconfig_plugin_t *get_autoconfig_plugin(void);

static void format_ip(uint32_t ip, char *buf)
{
    int i = 0;

    unsigned char octet[4]  = { 0, 0, 0, 0 };

    for (i = 0; i < 4; i++) {
        octet[i] = ( ip >> ((3 - i) * 8) ) & 0xFF;
    }

    sprintf(buf, "%d.%d.%d.%d", octet[3], octet[2], octet[1], octet[0]);
}

static void netmgr_connect_fail_event(hal_wifi_module_t *m, int err, void* arg)
{

}

static void netmgr_ip_got_event(hal_wifi_module_t *m,
                                hal_wifi_ip_stat_t *pnet, void *arg)
{
    char got_ip[16] = { 0 };
    char gw_ip[16] = { 0 };
    char mask_ip[16] = { 0 };
    int32_t ip = 0;
    uint8_t index;

    sprintf(got_ip, "%d.%d.%d.%d", pnet->ip[3], pnet->ip[2], pnet->ip[1], pnet->ip[0]);
    sprintf(gw_ip, "%d.%d.%d.%d", pnet->gate[3], pnet->gate[2], pnet->gate[1], pnet->gate[0]);
    sprintf(mask_ip, "%d.%d.%d.%d", pnet->mask[3], pnet->mask[2], pnet->mask[1], pnet->mask[0]);

    LOGI(TAG, "Got ip : %s, gw : %s, mask : %s", got_ip, gw_ip, mask_ip);

    for (index = 0; index < 4; index++) {
        ip |= (pnet->ip[index] << (index * 8));
    }
    g_netmgr_cxt.ipv4_owned = ip;
    yos_post_event(EV_WIFI, CODE_WIFI_ON_PRE_GOT_IP, 0u);
}

static void netmgr_stat_chg_event(hal_wifi_module_t *m, hal_wifi_event_t stat,
                                  void *arg)
{
    switch (stat) {
        case NOTIFY_STATION_UP:
            yos_post_event(EV_WIFI, CODE_WIFI_ON_CONNECTED,
                           (unsigned long)g_netmgr_cxt.ap_config.ssid);
            break;
        case NOTIFY_STATION_DOWN:
            yos_post_event(EV_WIFI, CODE_WIFI_ON_DISCONNECT, 0u);
            break;
        case NOTIFY_AP_UP:
            break;
        case NOTIFY_AP_DOWN:
            break;
        default:
            break;
    }
}

static void netmgr_scan_completed_event(hal_wifi_module_t *m,
                                         hal_wifi_scan_result_t *result,
                                         void *arg)
{
}

static void netmgr_scan_adv_completed_event(hal_wifi_module_t *m,
                                             hal_wifi_scan_result_t *result,
                                             void *arg)
{
}

static void netmgr_para_chg_event(hal_wifi_module_t *m,
                                  hal_wifi_ap_info_adv_t *ap_info,
                                  char *key, int key_len, void *arg)
{
}

static void netmgr_fatal_err_event(hal_wifi_module_t *m, void* arg)
{
}

static void netmgr_monitor_data(uint8_t *buf, int len)
{
    if (g_netmgr_cxt.monitor_dt_cb!= NULL) {
        g_netmgr_cxt.monitor_dt_cb(buf, len);
    }
}

static const hal_wifi_event_cb_t g_wifi_hal_event = {
    .connect_fail        = netmgr_connect_fail_event,
    .ip_got              = netmgr_ip_got_event,
    .stat_chg            = netmgr_stat_chg_event,
    .scan_compeleted     = netmgr_scan_completed_event,
    .scan_adv_compeleted = netmgr_scan_adv_completed_event,
    .para_chg            = netmgr_para_chg_event,
    .fatal_err           = netmgr_fatal_err_event,
};

static void reconnect_wifi(void *arg)
{
    hal_wifi_module_t    *module;
    hal_wifi_init_type_t type;
    netmgr_ap_config_t   *ap_config = &(g_netmgr_cxt.ap_config);

    module = hal_wifi_get_default_module();

    type.wifi_mode = Station;
    memcpy(type.wifi_ssid, ap_config->ssid, sizeof(type.wifi_ssid));
    memcpy(type.wifi_key, ap_config->pwd, sizeof(type.wifi_key));
    hal_wifi_start(module, &type);
}

static void get_wifi_ssid(void)
{
    memset(g_netmgr_cxt.ap_config.ssid, 0, sizeof(g_netmgr_cxt.ap_config.ssid));
    strncpy(g_netmgr_cxt.ap_config.ssid,
            g_netmgr_cxt.persistent_conf_wifi.ssid, MAX_SSID_SIZE);

    memset(g_netmgr_cxt.ap_config.pwd, 0, sizeof(g_netmgr_cxt.ap_config.pwd));
    strncpy(g_netmgr_cxt.ap_config.pwd,
            g_netmgr_cxt.persistent_conf_wifi.pwd, MAX_PWD_SIZE);
}

static int clear_wifi_ssid(void)
{
    int ret = 0;

    memset(g_netmgr_cxt.ap_config.ssid, 0, sizeof(g_netmgr_cxt.ap_config.ssid));
    memset(g_netmgr_cxt.ap_config.pwd, 0, sizeof(g_netmgr_cxt.ap_config.pwd));

    memset(&g_netmgr_cxt.persistent_conf_wifi, 0, sizeof(yos_persistent_conf_wifi_t));
    ret = yos_kv_set("wifi", (unsigned char *)(&g_netmgr_cxt.persistent_conf_wifi),
                     sizeof(yos_persistent_conf_wifi_t), 1);

    return ret;
}

static int set_wifi_ssid(void)
{
    int ret = 0;

    memset(&g_netmgr_cxt.persistent_conf_wifi, 0,
           sizeof(yos_persistent_conf_wifi_t));
    strncpy(g_netmgr_cxt.persistent_conf_wifi.ssid,
            g_netmgr_cxt.ap_config.ssid, MAX_SSID_SIZE);
    strncpy(g_netmgr_cxt.persistent_conf_wifi.pwd,
            g_netmgr_cxt.ap_config.pwd, MAX_PWD_SIZE);

    ret = yos_kv_set("wifi", (unsigned char *)&g_netmgr_cxt.persistent_conf_wifi,
                     sizeof(yos_persistent_conf_wifi_t), 1);

    return ret;
}

static void handle_wifi_disconnect(void)
{
    if (g_netmgr_cxt.autoconfig_chain != NULL) {
        g_netmgr_cxt.autoconfig_chain->autoconfig_stop();
        del_first_autoconfig_plugin();
    }

    g_netmgr_cxt.disconnected_times++;

    if (has_valid_ap() == 1 && g_netmgr_cxt.disconnected_times < MAX_RETRY_CONNECT) {
        yos_post_delayed_action(RETRY_INTERVAL_MS, reconnect_wifi, NULL);
    } else {
        clear_wifi_ssid();
        netmgr_wifi_config_start();
    }
}


static void netmgr_events_executor(input_event_t *eventinfo, void *priv_data)
{
    if (eventinfo->type != EV_WIFI) {
        return;
    }

    switch(eventinfo->code) {
        case CODE_WIFI_ON_CONNECTED:
            g_netmgr_cxt.disconnected_times = 0;
            break;
        case CODE_WIFI_ON_DISCONNECT:
            handle_wifi_disconnect();
            break;
        case CODE_WIFI_ON_PRE_GOT_IP:
            if (g_netmgr_cxt.autoconfig_chain != NULL) {
                g_netmgr_cxt.autoconfig_chain->config_result_cb(
                    0, g_netmgr_cxt.ipv4_owned);
                g_netmgr_cxt.autoconfig_chain->autoconfig_stop();
            } else {
                yos_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP,
                    (unsigned long)(&g_netmgr_cxt.ipv4_owned));
            }
            break;
        case CODE_WIFI_ON_GOT_IP:
            if (g_netmgr_cxt.autoconfig_chain != NULL) {
                set_wifi_ssid();
                del_first_autoconfig_plugin();
            }
            break;
        case CODE_WIFI_CMD_RECONNECT:
            g_netmgr_cxt.disconnected_times = 0;
            reconnect_wifi(NULL);
            break;
        default :
            break;
    }
}

void wifi_get_ip(char ips[16])
{
    format_ip(g_netmgr_cxt.ipv4_owned, ips);
}

void wifi_set_monitor_data_handler(monitor_data_cb_t cb)
{
    g_netmgr_cxt.monitor_dt_cb = cb;
}

static void netmgr_wifi_config_start(void)
{
    autoconfig_plugin_t * valid_plugin = get_autoconfig_plugin();
    hal_wifi_module_t *module;

    if (valid_plugin != NULL) {
        valid_plugin->autoconfig_start();
        g_netmgr_cxt.autoconfig_chain = valid_plugin;
        module = hal_wifi_get_default_module();
        hal_wifi_register_monitor_cb(module, netmgr_monitor_data);
    } else {
        LOGW(TAG, "net mgr none config policy");
    }
}

static int32_t has_valid_ap(void)
{
    int32_t len = strlen(g_netmgr_cxt.ap_config.ssid);

    if(len <= 0) {
        return 0;
    }

    return 1;
}

static void add_autoconfig_plugin(autoconfig_plugin_t *plugin)
{
    plugin->next = g_netmgr_cxt.autoconfig_chain;
    g_netmgr_cxt.autoconfig_chain = plugin;
}

static void del_first_autoconfig_plugin(void)
{
    g_netmgr_cxt.autoconfig_chain = g_netmgr_cxt.autoconfig_chain->next;
}

static autoconfig_plugin_t *get_autoconfig_plugin(void)
{
    return g_netmgr_cxt.autoconfig_chain;
}

int netmgr_set_ap_config(netmgr_ap_config_t *config)
{
    int ret = 0;

    strncpy(g_netmgr_cxt.ap_config.ssid, config->ssid, MAX_SSID_SIZE);
    strncpy(g_netmgr_cxt.ap_config.pwd, config->pwd, MAX_PWD_SIZE);

    strncpy(g_netmgr_cxt.persistent_conf_wifi.ssid,
            config->ssid, sizeof(g_netmgr_cxt.persistent_conf_wifi.ssid) - 1);
    strncpy(g_netmgr_cxt.persistent_conf_wifi.pwd,
            config->pwd, sizeof(g_netmgr_cxt.persistent_conf_wifi.pwd) - 1);

    ret = yos_kv_set("wifi", (unsigned char *)&g_netmgr_cxt.persistent_conf_wifi,
                     sizeof(yos_persistent_conf_wifi_t), 0);
    return ret;
}

void netmgr_set_smart_config(autoconfig_plugin_t *plugin)
{
    add_autoconfig_plugin(plugin);
    netmgr_wifi_config_start();
}

static void read_persistent_conf(void)
{
    int ret;
    int len;

    len = sizeof(yos_persistent_conf_wifi_t);
    ret = yos_kv_get("wifi", (unsigned char *)&g_netmgr_cxt.persistent_conf_wifi, &len);
    if (ret < 0) {
        return;
    }
    get_wifi_ssid();
}

int netmgr_init(void)
{
    hal_wifi_module_t *module;

    module = hal_wifi_get_default_module();
    memset(&g_netmgr_cxt, 0, sizeof(g_netmgr_cxt));
    g_netmgr_cxt.wifi_hal_mod = module;
    add_autoconfig_plugin(&g_def_smartconfig);
    hal_wifi_install_event(g_netmgr_cxt.wifi_hal_mod, &g_wifi_hal_event);
    g_netmgr_cxt.auto_start_smartconfig = true;
    read_persistent_conf();
    return 0;
}

void netmgr_deinit(void)
{
}

int netmgr_start(void)
{
    yos_register_event_filter(EV_WIFI, netmgr_events_executor, NULL);

    if (has_valid_ap() == 1) {
        yos_post_event(EV_WIFI, CODE_WIFI_CMD_RECONNECT, 0);
    } else {
        netmgr_wifi_config_start();
    }
    return 0;
}

static int def_smart_config_start(void)
{
    netmgr_ap_config_t config;

    memcpy(config.ssid, DEMO_AP_SSID, sizeof(config.ssid));
    memcpy(config.pwd, DEMO_AP_PASSWORD, sizeof(config.pwd));
    netmgr_set_ap_config(&config);
    yos_post_event(EV_WIFI, CODE_WIFI_CMD_RECONNECT, 0);
    return 0;
}

static void def_smart_config_stop(void)
{
    yos_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP,
        (unsigned long)(&g_netmgr_cxt.ipv4_owned));
}

static void def_smart_config_result_cb(int result, uint32_t ip) {
}

static autoconfig_plugin_t g_def_smartconfig = {
    .description = "def_smartconfig",
    .autoconfig_start = def_smart_config_start,
    .autoconfig_stop = def_smart_config_stop,
    .config_result_cb = def_smart_config_result_cb
};
