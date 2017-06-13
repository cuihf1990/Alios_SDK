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
#include <stdlib.h>
#include <string.h>
#include <hal/base.h>
#include <hal/wifi.h>
#include "common.h"
#include "mico_wlan.h"

hal_wifi_module_t sim_yos_wifi_beken;


static int wifi_init(hal_wifi_module_t *m)
{
    printf("wifi init success!!\n");
    return 0;
};

static void wifi_get_mac_addr(hal_wifi_module_t *m, uint8_t *mac)
{
	mico_wlan_get_mac_address(mac);
};


static int wifi_start(hal_wifi_module_t *m, hal_wifi_init_type_t *init_para)
{
    int ret;
	network_InitTypeDef_st cfg;
	
	memcpy(&cfg, init_para, sizeof(cfg));
	ret = micoWlanStart(&cfg);

    return ret;
}

static int wifi_start_adv(hal_wifi_module_t *m, hal_wifi_init_type_adv_t *init_para_adv)
{
    int ret;
	network_InitTypeDef_adv_st cfg;

	memcpy(&cfg, init_para_adv, sizeof(cfg));
 	ret = micoWlanStartAdv(&cfg);
	
    return ret;
}

static int get_ip_stat(hal_wifi_module_t *m, hal_wifi_ip_stat_t *out_net_para, hal_wifi_type_t wifi_type)
{
    int ret;
	IPStatusTypedef state;
	WiFi_Interface iface = (WiFi_Interface)wifi_type;
	
	ret = micoWlanGetIPStatus(&state, iface);

	memcpy(out_net_para, &state, sizeof(state));
    return ret;
}

static int get_link_stat(hal_wifi_module_t *m, hal_wifi_link_stat_t *out_stat)
{
	LinkStatusTypeDef linkstate;
	int ret;
	
    ret = micoWlanGetLinkStatus(&linkstate);
	memcpy(out_stat, &linkstate, sizeof(linkstate));

	return ret;
}

static void start_scan(hal_wifi_module_t *m)
{
    micoWlanStartScan();
}

static void start_scan_adv(hal_wifi_module_t *m)
{
	micoWlanStartScanAdv();
}

static int power_off(hal_wifi_module_t *m)
{
    return micoWlanPowerOff();
}

static int power_on(hal_wifi_module_t *m)
{
    return micoWlanPowerOn();
}

static int suspend(hal_wifi_module_t *m)
{
    return micoWlanSuspend();
}

static int suspend_station(hal_wifi_module_t *m)
{
    return micoWlanSuspendStation();
}

static int suspend_soft_ap(hal_wifi_module_t *m)
{

    return micoWlanSuspendSoftAP();
}

static int set_channel(hal_wifi_module_t *m, int ch)
{
    return mico_wlan_set_channel(ch);
}

static void start_monitor(hal_wifi_module_t *m)
{
	mico_wlan_start_monitor();
}

static void stop_monitor(hal_wifi_module_t *m)
{
	mico_wlan_stop_monitor();
}

static void register_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn)
{
	mico_wlan_register_monitor_cb((monitor_cb_t)fn);
}

void NetCallback(net_para_st *pnet)
{
	if (sim_yos_wifi_beken.ev_cb == NULL)
		return;
	if (sim_yos_wifi_beken.ev_cb->ip_got == NULL)
		return;

	sim_yos_wifi_beken.ev_cb->ip_got(&sim_yos_wifi_beken, pnet, NULL);
}

void connected_ap_info(apinfo_adv_t *ap_info, char *key, int key_len)
{
	if (sim_yos_wifi_beken.ev_cb == NULL)
		return;
	if (sim_yos_wifi_beken.ev_cb->para_chg == NULL)
		return;

	sim_yos_wifi_beken.ev_cb->para_chg(&sim_yos_wifi_beken, ap_info, key, key_len, NULL);
}

void WifiStatusHandler(int status)
{
	if (sim_yos_wifi_beken.ev_cb == NULL)
		return;
	if (sim_yos_wifi_beken.ev_cb->stat_chg == NULL)
		return;

	sim_yos_wifi_beken.ev_cb->stat_chg(&sim_yos_wifi_beken, status, NULL);
}


hal_wifi_module_t sim_yos_wifi_beken = {
    .base.name           = "sim_yos_wifi_beken",
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
};

