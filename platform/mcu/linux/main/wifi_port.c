/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hal/base.h>
#include <hal/wifi.h>

#ifndef AOS_ATCMD
static uint8_t fixmac[6] = {0xd8,0x96,0xe0,0x03,0x04,0x01};

static int wifi_init(hal_wifi_module_t *m)
{
    printf("wifi init success!!\n");
    return 0;
};

static void wifi_get_mac_addr(hal_wifi_module_t *m, uint8_t *mac)
{
    (void)m;
    printf("wifi_get_mac_addr!!\n");
    memcpy(mac, fixmac, 6);
};

static int wifi_start(hal_wifi_module_t *m, hal_wifi_init_type_t *init_para)
{
    (void)init_para;

    hal_wifi_ip_stat_t ip_stat;

    char ip[4]   = {0x7f, 0x00, 0x00, 0x01};
    char mask[4] = {0xFF, 0xFF, 0xFF, 0x00};
    char gate[4] = {0x7f, 0x00, 0x00, 0x01};

    memcpy(ip_stat.ip, ip, 4);
    memcpy(ip_stat.mask, mask, 4);
    memcpy(ip_stat.gate, gate, 4);
    memcpy(ip_stat.mac, fixmac, 6);

    if (m->ev_cb->stat_chg != NULL) {
        m->ev_cb->stat_chg(m, NOTIFY_STATION_UP, NULL);
    }

    if (m->ev_cb->ip_got != NULL) {
        m->ev_cb->ip_got(m, &ip_stat, NULL);
    }

    return 0;
}

static int wifi_start_adv(hal_wifi_module_t *m, hal_wifi_init_type_adv_t *init_para_adv)
{
    (void)init_para_adv;
 
    return 0;
}

static int get_ip_stat(hal_wifi_module_t *m, hal_wifi_ip_stat_t *out_net_para, hal_wifi_type_t wifi_type)
{
    (void)wifi_type;

    char ip[4]   = {0x7f, 0x00, 0x00, 0x01};
    char mask[4] = {0xFF, 0xFF, 0xFF, 0x00};
    char gate[4] = {0x7f, 0x00, 0x00, 0x01};

    memcpy(out_net_para->ip, ip, 4);
    memcpy(out_net_para->mask, mask, 4);
    memcpy(out_net_para->gate, gate, 4);
    memcpy(out_net_para->mac, fixmac, 6);

    return 0;
}

static int get_link_stat(hal_wifi_module_t *m, hal_wifi_link_stat_t *out_stat)
{
    out_stat->is_connected = 1;
    return 0;
}

static void start_scan(hal_wifi_module_t *m)
{
    hal_wifi_scan_result_t scan_ret;
    scan_ret.ap_num = 1;
    scan_ret.ap_list->ap_power = 80;
    memcpy(scan_ret.ap_list->ssid, "test", 5);

    if (m->ev_cb->scan_compeleted != NULL) {
        m->ev_cb->scan_compeleted(m, &scan_ret, NULL);
    }
}

static void start_scan_adv(hal_wifi_module_t *m)
{
}

static int power_off(hal_wifi_module_t *m)
{
    return 0;
}

static int power_on(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend_station(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend_soft_ap(hal_wifi_module_t *m)
{

    return 0;
}

static int set_channel(hal_wifi_module_t *m, int ch)
{
    return 0;
}

static void start_monitor(hal_wifi_module_t *m)
{

}

static void stop_monitor(hal_wifi_module_t *m)
{

}

static void register_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn)
{

}

static void register_wlan_mgnt_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn)
{

}

static int wlan_send_80211_raw_frame(hal_wifi_module_t *m, uint8_t *buf, int len)
{
    return 0;
}
#else // #ifndef AOS_ATCMD
#include "atparser.h"
#define TAG "wifi_port"

static int get_mac_helper(uint8_t *mac);
static int get_ip_stat_helper(hal_wifi_ip_stat_t *result);

static void fetch_ip_stat(void *arg)
{
    char out[128];
    hal_wifi_ip_stat_t ip_stat = {0};
    hal_wifi_module_t *m;

    if (!arg) {
        LOGE(TAG, "%s failed, invalid argument\r\n", __func__);
        return;
    }

    m = (hal_wifi_module_t *)arg;

    get_ip_stat_helper(&ip_stat);

    if (m->ev_cb->ip_got != NULL) {
        m->ev_cb->ip_got(m, &ip_stat, NULL);
    }
}

static void at_wevent_handler(void *arg)
{
    hal_wifi_module_t *m;

    if (!arg) {
        LOGE(TAG, "%s failed, invalid argument\r\n", __func__);
        return;
    }

    m = (hal_wifi_module_t *)arg;

    if (m->ev_cb->stat_chg != NULL) {
        m->ev_cb->stat_chg(m, NOTIFY_STATION_UP, NULL);
    }

    // !!!Do not call at_send_raw here since it will block at_worker
    aos_task_new("fetch_ip_stat", fetch_ip_stat, arg, 1024);
}

static int wifi_init(hal_wifi_module_t *m)
{
    LOGD(TAG, "wifi init success!!\n");
    return 0;
};

static void wifi_get_mac_addr(hal_wifi_module_t *m, uint8_t *mac)
{
    char out[64] = {0};

    (void)m;
    LOGD(TAG, "wifi_get_mac_addr!!\n");

    if (at.send_raw("AT+WMAC?", out) == 0)
        LOGD(TAG, "AT command succeed, rsp: %s", out);
    else
        LOGE(TAG, "AT command failed\r\n");

    if (strstr(out, "ERROR")) return;
    sscanf(out, "%*[^:]:%[^\r]", mac);
    printf("mac result: %s\r\n", mac);
};

static int wifi_start(hal_wifi_module_t *m, hal_wifi_init_type_t *init_para)
{
    char in[128], out[128];

    (void)init_para;

    hal_wifi_ip_stat_t ip_stat;

    snprintf(in, sizeof(in), "AT+WJAP=%s,%s", init_para->wifi_ssid, init_para->wifi_key);
    LOGD(TAG, "Will connect via at cmd: %s\r\n", in);

    at.oob("+WEVENT:STATION_UP\r\n", at_wevent_handler, (void *)m);

    if (at.send_raw(in, out) == 0)
        LOGD(TAG, "AT command succeed, rsp: %s\r\n", out);
    else
        LOGE(TAG, "AT command failed\r\n");

    if (strstr(out, "ERROR")) {
        LOGE(TAG, "Connect wifi failed\r\n");
        return -1;
    }

    return 0;
}

static int wifi_start_adv(hal_wifi_module_t *m, hal_wifi_init_type_adv_t *init_para_adv)
{
    (void)init_para_adv;
 
    return 0;
}

static int get_mac_helper(uint8_t *mac)
{
    char out[128] = {0};

    if (!mac) return -1;

    if (at.send_raw("AT+WMAC?", out) == 0)
        LOGD(TAG, "AT command succeed, rsp: %s", out);
    else
        LOGE(TAG, "AT command failed\r\n");

    if (strstr(out, "ERROR")) return -1;
    sscanf(out, "%*[^:]:%[^\r]", mac);
    LOGD(TAG, "mac result: %s\r\n", mac);

    return 0;
}

static int get_ip_stat_helper(hal_wifi_ip_stat_t *result)
{
    char out[128] = {0};

    if (!result) return -1;

    if (at.send_raw("AT+WJAPIP?", out) == 0)
        LOGD(TAG, "AT command succeed, rsp: %s", out);
    else
        LOGE(TAG, "AT command failed\r\n");

    if (strstr(out, "ERROR")) return -1;
    sscanf(out, "%*[^:]:%[^,],%[^,],%[^,],%[^\r]",
      result->ip, result->mask, result->gate, result->dns);

    LOGD(TAG, "result: %s %s %s %s\r\n",
      result->ip, result->mask, result->gate, result->dns);

    get_mac_helper(result->mac);

    return 0;
}

static int get_ip_stat(hal_wifi_module_t *m, hal_wifi_ip_stat_t *out_net_para, hal_wifi_type_t wifi_type)
{
    char out[128] = {0};

    (void)wifi_type;
    (void)m;

    get_ip_stat_helper(out_net_para);

    return 0;
}

static int get_link_stat(hal_wifi_module_t *m, hal_wifi_link_stat_t *out_stat)
{
    return 0;
}

static void start_scan(hal_wifi_module_t *m)
{

}

static void start_scan_adv(hal_wifi_module_t *m)
{
}

static int power_off(hal_wifi_module_t *m)
{
    return 0;
}

static int power_on(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend_station(hal_wifi_module_t *m)
{
    return 0;
}

static int suspend_soft_ap(hal_wifi_module_t *m)
{

    return 0;
}

static int set_channel(hal_wifi_module_t *m, int ch)
{
    return 0;
}

static void start_monitor(hal_wifi_module_t *m)
{

}

static void stop_monitor(hal_wifi_module_t *m)
{

}

static void register_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn)
{

}

static void register_wlan_mgnt_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn)
{

}

static int wlan_send_80211_raw_frame(hal_wifi_module_t *m, uint8_t *buf, int len)
{
    return 0;
}
#endif // #ifndef AOS_ATCMD

hal_wifi_module_t sim_aos_wifi_linux = {
    .base.name           = "sim_aos_wifi_linux",
    .init                =  wifi_init,
    .get_mac_addr        =  wifi_get_mac_addr,
    .start               =  wifi_start,
    .start_adv           =  wifi_start_adv,
    .get_ip_stat         =  get_ip_stat,
    .get_link_stat       =  get_link_stat,
    .start_scan          =  start_scan,
    .start_scan_adv      =  start_scan_adv,
    .power_off           =  power_off,
    .power_on            =  power_on,
    .suspend             =  suspend,
    .suspend_station     =  suspend_station,
    .suspend_soft_ap     =  suspend_soft_ap,
    .set_channel         =  set_channel,
    .start_monitor       =  start_monitor,
    .stop_monitor        =  stop_monitor,
    .register_monitor_cb =  register_monitor_cb,
    .register_wlan_mgnt_monitor_cb = register_wlan_mgnt_monitor_cb,
    .wlan_send_80211_raw_frame = wlan_send_80211_raw_frame
};

