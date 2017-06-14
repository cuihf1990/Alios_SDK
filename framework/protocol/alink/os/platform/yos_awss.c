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

#include <hal/base.h>
#include <hal/wifi.h>
#include <netmgr.h>
#include <yos/framework.h>

#include "platform.h"
#include "platform_config.h"

platform_awss_recv_80211_frame_cb_t g_ieee80211_handler;
autoconfig_plugin_t g_alink_smartconfig;

//一键配置超时时间, 建议超时时间1-3min, APP侧一键配置1min超时
int platform_awss_get_timeout_interval_ms(void)
{
    return 3 * 60 * 1000;
}

//默认热点配网超时时间
int platform_awss_get_connect_default_ssid_timeout_interval_ms( void )
{
    return 0;
}

//一键配置每个信道停留时间, 建议200ms-400ms
int platform_awss_get_channelscan_interval_ms(void)
{
    return 200;
}

//wifi信道切换，信道1-13
void platform_awss_switch_channel(char primary_channel,
                                  char secondary_channel, uint8_t bssid[ETH_ALEN])
{
    hal_wifi_module_t *module;

    module = hal_wifi_get_default_module();
    if (module == NULL) {
        return;
    }

    hal_wifi_set_channel(module, (int)primary_channel);
}

static void monitor_data_handler(uint8_t *buf, int len)
{
    int with_fcs = 1;
    int link_type = AWSS_LINK_TYPE_80211_RADIO;

    (*g_ieee80211_handler)(buf, len, link_type, with_fcs);
}

//进入monitor模式, 并做好一些准备工作，如
//设置wifi工作在默认信道6
//若是linux平台，初始化socket句柄，绑定网卡，准备收包
//若是rtos的平台，注册收包回调函数aws_80211_frame_handler()到系统接口
void platform_awss_open_monitor(platform_awss_recv_80211_frame_cb_t cb)
{
    hal_wifi_module_t *module = hal_wifi_get_default_module();

    if (module == NULL) {
        return;
    }

    g_ieee80211_handler = cb;
    hal_wifi_register_monitor_cb(module, monitor_data_handler);
    hal_wifi_start_wifi_monitor(module);
    platform_awss_switch_channel(6, 0, NULL);
}

//退出monitor模式，回到station模式, 其他资源回收
void platform_awss_close_monitor(void)
{
    hal_wifi_module_t *module;

    module = hal_wifi_get_default_module();
    if (module == NULL) {
        return;
    }

    hal_wifi_register_monitor_cb(module, NULL);
    hal_wifi_stop_wifi_monitor(module);
}

int platform_awss_connect_ap(
    _IN_ uint32_t connection_timeout_ms,
    _IN_ char ssid[PLATFORM_MAX_SSID_LEN],
    _IN_ char passwd[PLATFORM_MAX_PASSWD_LEN],
    _IN_OPT_ enum AWSS_AUTH_TYPE auth,
    _IN_OPT_ enum AWSS_ENC_TYPE encry,
    _IN_OPT_ uint8_t bssid[ETH_ALEN],
    _IN_OPT_ uint8_t channel)
{
    int ret;
    netmgr_ap_config_t config;

    strcpy(config.ssid, ssid);
    strcpy(config.pwd, passwd);
    yos_post_event(EV_WIFI, CODE_WIFI_CMD_RECONNECT, 0u);
    ret = netmgr_set_ap_config(&config);

    return ret;
}

int platform_wifi_scan(platform_wifi_scan_result_cb_t cb)
{
    return 0;
}

p_aes128_t platform_aes128_init(
    const uint8_t *key,
    const uint8_t *iv,
    AES_DIR_t dir)
{
    return 0;
}

int platform_aes128_destroy(
    p_aes128_t aes)
{
    return 0;
}

int platform_aes128_cbc_encrypt(
    p_aes128_t aes,
    const void *src,
    size_t blockNum,
    void *dst )
{
    return 0;
}

int platform_aes128_cbc_decrypt(
    p_aes128_t aes,
    const void *src,
    size_t blockNum,
    void *dst )
{
    return 0;
}

int platform_wifi_get_ap_info(
    char ssid[PLATFORM_MAX_SSID_LEN],
    char passwd[PLATFORM_MAX_PASSWD_LEN],
    uint8_t bssid[ETH_ALEN])
{
    return 0;
}


int platform_wifi_low_power(int timeout_ms)
{
    //wifi_enter_power_saving_mode();
    usleep(timeout_ms);
    //wifi_exit_power_saving_mode();

    return 0;
}

int platform_wifi_enable_mgnt_frame_filter(
    _IN_ uint32_t filter_mask,
    _IN_OPT_ uint8_t vendor_oui[3],
    _IN_ platform_wifi_mgnt_frame_cb_t callback)
{
    return -2;
}

int platform_wifi_send_80211_raw_frame(_IN_ enum platform_awss_frame_type type,
                                       _IN_ uint8_t *buffer, _IN_ int len)
{
    return -2;
}

static int smart_config_start(void)
{
    awss_start();
    return 0;
}

static void smart_config_stop(void)
{
    awss_stop();
}

static void smart_config_result_cb(int result, uint32_t ip)
{
    yos_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP, 0u);
}

autoconfig_plugin_t g_alink_smartconfig = {
    .description = "alink_smartconfig",
    .autoconfig_start = smart_config_start,
    .autoconfig_stop = smart_config_stop,
    .config_result_cb = smart_config_result_cb
};
