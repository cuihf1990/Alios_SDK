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

/**
 * @file hal/wifi.h
 * @brief WiFi HAL API for YOC
 * @detail WiFi HAL defines basic WiFi management functionalities including <BR>
 *         getting mac address, connect to specified AP(ssid/password), enter promiscuous mode. <BR>
 *         On the other hand, integration with TCP/IP stack is not the scope.
 *         <BR>
 *         <BR>
 *         Definition of some terms: <BR>
 *         Smart Config - a vendor providied method to config a device from other device, typically Smart Phone <BR>
 *         Promisicious Mode - also known as Monitor Mode, a special mode that WiFi Chip pass all data to upper layer <BR>
 * @version since 5.5.0
 */

#ifndef HAL_WIFI_H
#define HAL_WIFI_H

#include <stdint.h>

/** @defgroup yoc_hal_wifi WiFi HAL API
 *  @{
 */

typedef unsigned int                   hal_ip_addr;

typedef struct hal_wifi_module_s       hal_wifi_module_t;

/**
 * @brief Wifi mode
 */
typedef enum
{
    HAL_WIFI_MODE_UNKNOWN = 0,
    /** station mode */
    HAL_WIFI_MODE_STATION,
    /** softap mode */
    HAL_WIFI_MODE_SOFTAP,
    /** monitor mode, or promiscious mode */
    HAL_WIFI_MODE_MONITOR,
    HAL_WIFI_MODE_CONFIG_MAX
} hal_wifi_mode_t;

/**
 * @brief Wifi config command in get/set mode
 */
typedef enum
{
    HAL_WIFI_SMART_CONFIG_START,
    HAL_WIFI_SMART_CONFIG_STOP,
    HAL_WIFI_CHECK_SUPPORT_SMART_CONFIG,

    HAL_WIFI_PROMISCUOUS_START,
    HAL_WIFI_PROMISCUOUS_STOP,
    HAL_WIFI_GET_CHANNEL,
    HAL_WIFI_SET_CHANNEL,
    HAL_WIFI_SET_AUTO_CONNECT,

    HAL_WIFI_GET_MAC_ADDRESS,
    HAL_WIFI_GET_BROADCAST_PORT,

    HAL_WIFI_GET_AP_RSSI,
} hal_wifi_getset_cmd_t;

/**
 * @struct hal_wifi_scan_result_t
 * @brief Wifi scan results
 */
typedef struct hal_wifi_scan_result_s
{
    /** MAC address of AP */
    unsigned char  bssid[6];
    /** SSID of AP, not necessarily 0 ended */
    unsigned char  ssid[32];
    /** SSID length */
    unsigned char  ssid_len;
    /** channel of AP         */
    unsigned char  channel;
    /** signal strength of AP */
    unsigned char  rssi;
} hal_wifi_scan_result_t;

/**
 * @struct hal_wifi_event_cb_t
 * @brief The event call back function called at specific events occurred
 * @note For HAL implementors, these callbacks must be
 *       called under normal task context, not from interrupt.
 */
typedef struct
{
    /**
     * Connected to AP
     */
    void (*connected_to_ap)(hal_wifi_module_t *m, char *ssid, int ssid_len,
                            char *pwd,int pwd_len);

    /**
     * Disconnected from AP
     */
    void (*disconnected_from_ap)(hal_wifi_module_t *m, char *ssid, int ssid_len,
                                 int reason);

    /**
     * Scan finished for user cmd
     */
    void (*finished_scan_ap)(void);

    /**
     * AP auth info changed
     */
    void (*changed_auth_of_ap)(void);

    /**
     * Got IP from AP
     */
    void (*got_ip_from_ap)(hal_wifi_module_t *m, hal_ip_addr *ip,
                           hal_ip_addr *mask, hal_ip_addr *gw);

    /**
     * DHCP from AP timeout
     */
    void (*got_dhcp_timeout_from_ap)(void);

    /**
     * SmartConfig
     */
    void (*sc_link_over)(hal_wifi_module_t *m, unsigned int ip);

    /**
     * Scan completed event triggered by hal_wifi_scan()
     */
    void (*scan_compeleted)(hal_wifi_module_t *m, int is_ok, int num, hal_wifi_scan_result_t *result);

    /**
     * Promiscuous data callback
     */
    void (*promiscuous_data)(hal_wifi_module_t *m, void *buf, uint16_t len);
} hal_wifi_event_cb_t;

/**
 * @struct hal_wifi_status_t
 * @brief Wifi status info
 */
typedef struct
{
    /** station/softap/monitor */
    hal_wifi_mode_t mode;
    /** if already connected to AP */
	int connected;
    /** ap number found by hal_wifi_scan() */
    int ap_num;
    /** ap result array found by hal_wifi_scan() */
    hal_wifi_scan_result_t *ap_records;
} hal_wifi_status_t;

/**
 * @struct hal_wifi_module_t
 * @brief The Wifi module for manufacture to implement
 */
struct hal_wifi_module_s
{
    /** hal common data structure, maintained by HAL framework */
    hal_module_base_t          base;
    /** WiFi status, maintained by vendor HAL */
    hal_wifi_status_t          status;
    const hal_wifi_event_cb_t *ev_cb;

    /**
     * The init function of manufacture
     */
    int (*init)(hal_wifi_module_t *m, void *something);

    /**
     * Set wifi mode of manufacture, mode is in hal_wifi_mode_t
     */
    int (*set_mode)(hal_wifi_module_t *m, hal_wifi_mode_t mode);

    /**
     * Tell wifi we trigger a connect action and the manufacture realize it
     */
    int (*connect_prepare)(hal_wifi_module_t *m, char *ssid, char *password);

    /**
     * Tell wifi to disconnect
     */
    int (*disconnect)(hal_wifi_module_t *m);

    /**
     * Tell wifi to scan AP and store the result temporarily, if finished scan, give user a callback
     */
    int (*scan)(hal_wifi_module_t *m, void *_not_use_now);
    int (*getset_ops)(hal_wifi_module_t *m, hal_wifi_getset_cmd_t cmd, ...);
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
void hal_wifi_register_module(hal_wifi_module_t *module);

/**
 * @breif Initialize WiFi instances
 * @param None
 * @retval == 0 success
 * @retval !=0 failure
 * @note This is supposed to be called during system boot,
 *       not supposed to be called by user module directly
 */
int hal_wifi_init(void);

/**
 * @brief Set the working mode
 * @param m The WiFi instance, NULL for default
 * @param mode The WiFi mode type
 * @retval == 0 success
 * @retval !=0 failure
 * @see hal_wifi_mode_t
 */
int hal_wifi_set_mode(hal_wifi_module_t *m, hal_wifi_mode_t mode);

/**
 * @brief Get the working mode
 * @param m The WiFi instance, NULL for default
 * @return the mode type
 * @see hal_wifi_mode_t
 */
hal_wifi_mode_t hal_wifi_get_mode(hal_wifi_module_t *m);

/**
 * @brief Set the event callback function array for the wifi
 * @param m The wifi instance, NULL for default
 * @param cb The event callback function info
 * @return None
 * @note Please don't do time consuming work in these callbacks
 */
void hal_wifi_install_event(hal_wifi_module_t *m,
                                      const hal_wifi_event_cb_t *cb);

/**
 * @brief Connect to an AP
 * @param m The wifi instance, NULL for default
 * @param ssid Target AP's SSID
 * @param password Target AP's PASSWORD
 * @retval ==0 success
 * @retval !=0 failure
 * @note WiFi must be at STATION mode
 */
int hal_wifi_connect(hal_wifi_module_t *m, char *ssid, char *password);

/**
 * @brief Disconnect from AP
 * @param m The wifi instance, NULL for default
 * @retval ==0 success
 * @retval !=0 failure
 */
int hal_wifi_disconnect(hal_wifi_module_t *m);

/**
 * @brief Scan AP
 * @param m The wifi instance, NULL for default
 * @retval ==0 success
 * @retval !=0 failure
 * @note Scan results are saved in m->status.ap_records
 */
int hal_wifi_scan(hal_wifi_module_t *m, void *_not_use_now);

/**
 * @brief Call vendor specific smartconfig commands
 * @param m The wifi instance, NULL for default
 * @param cmd HAL_WIFI_SMART_CONFIG_START or HAL_WIFI_SMART_CONFIG_STOP
 * @retval == 0 success
 * @retval !=0 failure
 * @see hal_wifi_get_set_cmd_t
 */
int hal_wifi_smartconfig(hal_wifi_module_t *m,
                                  hal_wifi_getset_cmd_t cmd);

/**
 * @brief Check if Smart Config supported
 * @param m The wifi instance, NULL for default
 * @retval == 0 success
 * @retval !=0 failure
 */
int hal_wifi_has_smartconfig(hal_wifi_module_t *m);

/**
 * @brief Enter promiscuous mode
 * @param m The wifi instance, NULL for default
 * @retval == 0 success
 * @retval !=0 failure
 */
int hal_wifi_enter_promiscuous(hal_wifi_module_t *m);

/**
 * @brief leave promiscuous mode
 * @param m The wifi instance, NULL for default
 * @retval == 0 success
 * @retval !=0 failure
 */
int hal_wifi_leave_promiscuous(hal_wifi_module_t *m);

/**
 * @brief Get current working channel
 * @param m The wifi instance, NULL for default
 * @retval >= 0 channel number
 * @retval <0   failure
 */
int hal_wifi_get_channel(hal_wifi_module_t *m);

/**
 * @brief Switch working channel
 * @param m The wifi instance, NULL for default
 * @param channel The channel to switch to 
 * @retval == 0 success
 * @retval !=0 failure
 * @note This API are supposed to be called only under promisicous mode
 */
int hal_wifi_set_channel(hal_wifi_module_t *m, int channel);

/**
 * @brief Get broadcast port
 * @param m The wifi instance, NULL for default
 * @retval >0 the port number
 * @retval <=0 error
 * @note This API will be called network management module after Smart Config is done,
 *       to exchange information with typically Smart Phone.
 */
int hal_wifi_get_broadcast_port(hal_wifi_module_t *m);

/**
 * @brief Get the mac address of a wifi module
 * @param m The wifi instance, NULL for default
 * @param mac The mac address of 6 bytes
 * @retval == 0 success
 * @retval !=0 failure
 * @note  Parameter mac must has more than 6 bytes
 */
int hal_wifi_get_mac(hal_wifi_module_t *m, uint8_t mac[6]);

/**
 * @brief Set if wifi is auto connected after power on/disconnected
 * @param m The wifi instance, NULL for default
 * @param auto_connect 0:disable auto connect, 1:enable auto connect
 * @retval == 0 success
 * @retval !=0 failure
 * @note This instruct whether to reconnect after disconnected from AP
 */
int hal_wifi_set_auto_connect(hal_wifi_module_t *m, int auto_connect);

/**
 * @brief Get the connected status of the wifi
 * @param m The wifi instance, NULL for default
 * @retval ==0 not connected
 * @retval !=0 connected
 */
int hal_wifi_is_connected(hal_wifi_module_t *m);

/**
 * @brief Get the rssi of the AP connected
 * @param m The wifi instance, NULL for default
 * @return RSSI number
 */
int hal_wifi_get_ap_rssi(hal_wifi_module_t *m);
#endif /* HAL_WIFI_H */

