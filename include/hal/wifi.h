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

#ifndef HAL_WIFI_H
#define HAL_WIFI_H

#include <stdint.h>
#include <hal/base.h>

/** @defgroup yos_hal_wifi WiFi HAL API
 *  @{
 */

typedef struct hal_wifi_module_s       hal_wifi_module_t;

enum wlan_sec_type_e {
    SECURITY_TYPE_NONE,        /**< Open system. */
    SECURITY_TYPE_WEP,         /**< Wired Equivalent Privacy. WEP security. */
    SECURITY_TYPE_WPA_TKIP,    /**< WPA /w TKIP */
    SECURITY_TYPE_WPA_AES,     /**< WPA /w AES */
    SECURITY_TYPE_WPA2_TKIP,   /**< WPA2 /w TKIP */
    SECURITY_TYPE_WPA2_AES,    /**< WPA2 /w AES */
    SECURITY_TYPE_WPA2_MIXED,  /**< WPA2 /w AES or TKIP */
    SECURITY_TYPE_AUTO,        /**< It is used when calling @ref micoWlanStartAdv, MICO read security type from scan result. */
};

/**
 *  @brief  Scan result using normal scan.
 */
typedef  struct {
    char ap_num;       /**< The number of access points found in scanning. */
    struct {
        char ssid[32];  /**< The SSID of an access point. */
        char ap_power;   /**< Signal strength, min:0, max:100. */
    } *ap_list;
} hal_wifi_scan_result_t;

typedef  struct  {
    char ap_num;       /**< The number of access points found in scanning.*/
    struct {
        char ssid[32];  /**< The SSID of an access point.*/
        char ap_power;   /**< Signal strength, min:0, max:100*/
        char bssid[6];  /**< The BSSID of an access point.*/
        char channel;   /**< The RF frequency, 1-13*/
        uint8_t security;   /**< Security type, @ref wlan_sec_type_t*/
    } *ap_list;
} hal_wifi_scan_result_adv_t;

typedef enum {
    NOTIFY_STATION_UP = 1,
    NOTIFY_STATION_DOWN,

    NOTIFY_AP_UP,
    NOTIFY_AP_DOWN,
} hal_wifi_event_t;


typedef struct {
    char    ssid[32];    /**< SSID of the wlan that needs to be connected. Example: "SSID String". */
    char    bssid[6];    /**< BSSID of the wlan needs to be connected. Example: {0xC8 0x93 0x46 0x11 0x22 0x33}. */
    uint8_t channel;     /**< Wlan's RF frequency, channel 0-13. 1-13 means a fixed channelthat can speed up a connection procedure, 0 is not a fixed input
                            means all channels are possible*/
    uint8_t security;
} hal_wifi_ap_info_adv_t;




typedef struct {
    char wifi_mode;               /**< DHCP mode: @ref wlanInterfaceTypedef.*/
    char wifi_ssid[32];           /**< SSID of the wlan needs to be connected.*/
    char wifi_key[64];            /**< Security key of the wlan needs to be connected, ignored in an open system.*/
    char local_ip_addr[16];       /**< Static IP configuration, Local IP address. */
    char net_mask[16];            /**< Static IP configuration, Netmask. */
    char gateway_ip_addr[16];     /**< Static IP configuration, Router IP address. */
    char dns_server_ip_addr[16];   /**< Static IP configuration, DNS server IP address. */
    char dhcp_mode;                /**< DHCP mode, @ref DHCP_Disable, @ref DHCP_Client and @ref DHCP_Server. */
    char reserved[32];
    int  wifi_retry_interval;     /**< Retry interval if an error is occured when connecting an access point,
                                     time unit is millisecond. */
} hal_wifi_init_type_t;


typedef struct {
    hal_wifi_ap_info_adv_t ap_info;         /**< @ref apinfo_adv_t. */
    char  key[64];                /**< Security key or PMK of the wlan. */
    int   key_len;                /**< The length of the key. */
    char  local_ip_addr[16];      /**< Static IP configuration, Local IP address. */
    char  net_mask[16];           /**< Static IP configuration, Netmask. */
    char  gateway_ip_addr[16];    /**< Static IP configuration, Router IP address. */
    char  dns_server_ip_addr[16];  /**< Static IP configuration, DNS server IP address. */
    char  dhcp_mode;               /**< DHCP mode, @ref DHCP_Disable, @ref DHCP_Client and @ref DHCP_Server. */
    char  reserved[32];
    int   wifi_retry_interval;    /**< Retry interval if an error is occured when connecting an access point, time unit is millisecond. */
} hal_wifi_init_type_adv_t;


typedef struct {
    uint8_t dhcp;       /**< DHCP mode: @ref DHCP_Disable, @ref DHCP_Client, @ref DHCP_Server.*/
    char    ip[16];     /**< Local IP address on the target wlan interface: @ref wlanInterfaceTypedef.*/
    char    gate[16];   /**< Router IP address on the target wlan interface: @ref wlanInterfaceTypedef.*/
    char    mask[16];   /**< Netmask on the target wlan interface: @ref wlanInterfaceTypedef.*/
    char    dns[16];    /**< DNS server IP address.*/
    char    mac[16];    /**< MAC address, example: "C89346112233".*/
    char    broadcastip[16];
} hal_wifi_ip_stat_t;

typedef enum {
    SOFT_AP,  /**< Act as an access point, and other station can connect, 4 stations Max*/
    STATION   /**< Act as a station which can connect to an access point*/
} hal_wifi_type_t;

enum {
    DHCP_DISABLE = 0,
    DHCP_CLIENT,
    DHCP_SERVER,
};

typedef struct {
    int     is_connected;  /**< The link to wlan is established or not, 0: disconnected, 1: connected. */
    int     wifi_strength; /**< Signal strength of the current connected AP */
    uint8_t ssid[32];      /**< SSID of the current connected wlan */
    uint8_t bssid[6];      /**< BSSID of the current connected wlan */
    int     channel;       /**< Channel of the current connected wlan */
} hal_wifi_link_stat_t;


/**
 * @struct hal_wifi_event_cb_t
 * @brief The event call back function called at specific events occurred
 * @note For HAL implementors, these callbacks must be
 *       called under normal task context, not from interrupt.
 */
typedef struct {
    void (*connect_fail)(hal_wifi_module_t *m, int err, void *arg);
    void (*ip_got)(hal_wifi_module_t *m, hal_wifi_ip_stat_t *pnet, void *arg);
    void (*stat_chg)(hal_wifi_module_t *m, hal_wifi_event_t stat, void *arg);
    void (*scan_compeleted)(hal_wifi_module_t *m, hal_wifi_scan_result_t *result,
                            void *arg);
    void (*scan_adv_compeleted)(hal_wifi_module_t *m,
                                hal_wifi_scan_result_adv_t *result, void *arg);
    void (*para_chg)(hal_wifi_module_t *m, hal_wifi_ap_info_adv_t *ap_info,
                     char *key, int key_len, void *arg);
    void (*fatal_err)(hal_wifi_module_t *m, void *arg);
} hal_wifi_event_cb_t;

typedef void (*monitor_data_cb_t)(uint8_t *data, int len);

/**
 * @struct hal_wifi_module_t
 * @brief The Wifi module for manufacture to implement
 */
struct hal_wifi_module_s {
    hal_module_base_t    base;
    const hal_wifi_event_cb_t *ev_cb;

    int  (*init)(hal_wifi_module_t *m);
    void (*get_mac_addr)(hal_wifi_module_t *m, uint8_t *mac);
    int  (*start)(hal_wifi_module_t *m, hal_wifi_init_type_t *init_para);
    int  (*start_adv)(hal_wifi_module_t *m,
                      hal_wifi_init_type_adv_t *init_para_adv);
    int  (*get_ip_stat)(hal_wifi_module_t *m, hal_wifi_ip_stat_t *out_net_para,
                        hal_wifi_type_t wifi_type);
    int  (*get_link_stat)(hal_wifi_module_t *m, hal_wifi_link_stat_t *out_stat);
    void (*start_scan)(hal_wifi_module_t *m);
    void (*start_scan_adv)(hal_wifi_module_t *m);
    int  (*power_off)(hal_wifi_module_t *m);
    int  (*power_on)(hal_wifi_module_t *m);
    int  (*suspend)(hal_wifi_module_t *m);
    int  (*suspend_station)(hal_wifi_module_t *m);
    int  (*suspend_soft_ap)(hal_wifi_module_t *m);
    int  (*set_channel)(hal_wifi_module_t *m, int ch);
    void (*start_monitor)(hal_wifi_module_t *m);
    void (*stop_monitor)(hal_wifi_module_t *m);
    void (*register_monitor_cb)(hal_wifi_module_t *m, monitor_data_cb_t fn);
    void (*register_wlan_mgnt_monitor_cb)(hal_wifi_module_t *m, monitor_data_cb_t fn);
    int (*wlan_send_80211_raw_frame)(hal_wifi_module_t *m, uint8_t *buf, int len);
};

/**
 * @brief Get the default wifi instance.
 * The system may have more than 1 wifi instance,
 * this API return the default one.
 * @param None
 * @retval ==NULL no instances registered
 * @retval !=NULL instance pointer
 */
hal_wifi_module_t *hal_wifi_get_default_module(void);


/**
 * @brief Regster a wifi instance to the HAL Framework
 * @param module the wifi instance
 * @return None
*/
void hal_wifi_register_module(hal_wifi_module_t *m);

/**
 * @breif Initialize WiFi instances
 * @param None
 * @retval == 0 success
 * @retval !=0 failure
 * @note This is supposed to be called during system boot,
 *       not supposed to be called by user module directly
 */
int  hal_wifi_init(void);

void hal_wifi_get_mac_addr(hal_wifi_module_t *m, uint8_t *mac);

int  hal_wifi_start(hal_wifi_module_t *m, hal_wifi_init_type_t *init_para);

int  hal_wifi_start_adv(hal_wifi_module_t *m,
                        hal_wifi_init_type_adv_t *init_para_adv);

int  hal_wifi_get_ip_stat(hal_wifi_module_t *m,
                          hal_wifi_ip_stat_t *out_net_para, hal_wifi_type_t wifi_type);

int  hal_wifi_get_link_stat(hal_wifi_module_t *m,
                            hal_wifi_link_stat_t *out_stat);

void hal_wifi_start_scan(hal_wifi_module_t *m);

void hal_wifi_start_scan_adv(hal_wifi_module_t *m);

int  hal_wifi_power_off(hal_wifi_module_t *m);

int  hal_wifi_power_on(hal_wifi_module_t *m);

int  hal_wifi_suspend(hal_wifi_module_t *m);

int  hal_wifi_suspend_station(hal_wifi_module_t *m);

int  hal_wifi_suspend_soft_ap(hal_wifi_module_t *m);
int  hal_wifi_set_channel(hal_wifi_module_t *m, int ch);

void hal_wifi_start_wifi_monitor(hal_wifi_module_t *m);
void hal_wifi_stop_wifi_monitor(hal_wifi_module_t *m);
void hal_wifi_register_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn);

/**
 * @brief Set the event callback function array for the wifi
 * @param m The wifi instance, NULL for default
 * @param cb The event callback function info
 * @return None
 * @note Please don't do time consuming work in these callbacks
 */
void hal_wifi_install_event(hal_wifi_module_t *m,
                            const hal_wifi_event_cb_t *cb);

#endif /* HAL_WIFI_H */

