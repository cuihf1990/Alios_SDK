#ifndef _WLAN_UI_PUB_
#define _WLAN_UI_PUB_

#pragma once

#include "include.h"
#include "rtos_pub.h"

#if CFG_MXCHIP
#include "mico_wlan.h"

#else
#define WiFi_Interface  wlanInterfaceTypedef

#define DHCP_Disable  (0)   /**< Disable DHCP service. */
#define DHCP_Client   (1)   /**< Enable DHCP client which get IP address from DHCP server automatically,  
								reset Wi-Fi connection if failed. */
#define DHCP_Server   (2)   /**< Enable DHCP server, needs assign a static address as local address. */

/**
 *  @brief  wlan network interface enumeration definition.
 */
typedef enum
{
    Soft_AP,  /**< Act as an access point, and other station can connect, 4 stations Max*/
    Station   /**< Act as a station which can connect to an access point*/
} wlanInterfaceTypedef;

/**
 *  @brief  Wi-Fi security type enumeration definition.
 */
enum wlan_sec_type_e
{
    SECURITY_TYPE_NONE,        /**< Open system. */
    SECURITY_TYPE_WEP,         /**< Wired Equivalent Privacy. WEP security. */
    SECURITY_TYPE_WPA_TKIP,    /**< WPA /w TKIP */
    SECURITY_TYPE_WPA_AES,     /**< WPA /w AES */
    SECURITY_TYPE_WPA2_TKIP,   /**< WPA2 /w TKIP */
    SECURITY_TYPE_WPA2_AES,    /**< WPA2 /w AES */
    SECURITY_TYPE_WPA2_MIXED,  /**< WPA2 /w AES or TKIP */
    SECURITY_TYPE_AUTO,        /**< It is used when calling @ref bkWlanStartAdv, MICO read security type from scan result. */
};

enum
{
    WLAN_RX_BEACON,    /* receive beacon packet */
    WLAN_RX_PROBE_REQ, /* receive probe request packet */
    WLAN_RX_PROBE_RES, /* receive probe response packet */
    WLAN_RX_ACTION,    /* receive action packet */
    WLAN_RX_MANAGEMENT,/* receive ALL management packet */
    WLAN_RX_DATA,      /* receive ALL data packet */
    WLAN_RX_MCAST_DATA,/* receive ALL multicast and broadcast packet */

    WLAN_RX_ALL,       /* receive ALL 802.11 packet */
};

typedef uint8_t wlan_sec_type_t;

/**
 *  @brief  wlan local IP information structure definition.
 */
typedef struct
{
	uint8_t dhcp;       /**< DHCP mode: @ref DHCP_Disable, @ref DHCP_Client, @ref DHCP_Server.*/
	char    ip[16];     /**< Local IP address on the target wlan interface: @ref wlanInterfaceTypedef.*/
    char    gate[16];   /**< Router IP address on the target wlan interface: @ref wlanInterfaceTypedef.*/
    char    mask[16];   /**< Netmask on the target wlan interface: @ref wlanInterfaceTypedef.*/
    char    dns[16];    /**< DNS server IP address.*/
    char    mac[16];    /**< MAC address, example: "C89346112233".*/
    char    broadcastip[16];
} IPStatusTypedef;

/**
 *  @brief  Scan result using normal scan.
 */
typedef  struct  _ScanResult
{
    char ApNum;       /**< The number of access points found in scanning. */
    struct
    {
        char ssid[32];  /**< The SSID of an access point. */
        char ApPower;   /**< Signal strength, min:0, max:100. */
    } *ApList;
} ScanResult;

/**
 *  @brief  Input network paras, used in bk_wlan_start function.
 */
typedef struct _network_InitTypeDef_st
{
    char wifi_mode;               /**< DHCP mode: @ref wlanInterfaceTypedef.*/
    char wifi_ssid[32];           /**< SSID of the wlan needs to be connected.*/
    char wifi_key[64];            /**< Security key of the wlan needs to be connected, ignored in an open system.*/
    char local_ip_addr[16];       /**< Static IP configuration, Local IP address. */
    char net_mask[16];            /**< Static IP configuration, Netmask. */
    char gateway_ip_addr[16];     /**< Static IP configuration, Router IP address. */
    char dnsServer_ip_addr[16];   /**< Static IP configuration, DNS server IP address. */
    char dhcpMode;                /**< DHCP mode, @ref DHCP_Disable, @ref DHCP_Client and @ref DHCP_Server. */
    char reserved[32];
    int  wifi_retry_interval;     /**< Retry interval if an error is occured when connecting an access point,
                                     time unit is millisecond. */
} network_InitTypeDef_st;

/**
 *  @brief  Advanced precise wlan parameters, used in @ref network_InitTypeDef_adv_st.
 */
typedef struct
{
    char    ssid[32];    /**< SSID of the wlan that needs to be connected. Example: "SSID String". */
    char    bssid[6];    /**< BSSID of the wlan needs to be connected. Example: {0xC8 0x93 0x46 0x11 0x22 0x33}. */
    uint8_t channel;     /**< Wlan's RF frequency, channel 0-13. 1-13 means a fixed channel
                            that can speed up a connection procedure, 0 is not a fixed input
                            means all channels are possible*/
    wlan_sec_type_t security;
}   apinfo_adv_t;

/**
 *  @brief  Input network precise paras in bkWlanStartAdv function.
 */
typedef struct _network_InitTypeDef_adv_st
{
    apinfo_adv_t ap_info;         /**< @ref apinfo_adv_t. */
    char  key[64];                /**< Security key or PMK of the wlan. */
    int   key_len;                /**< The length of the key. */
    char  local_ip_addr[16];      /**< Static IP configuration, Local IP address. */
    char  net_mask[16];           /**< Static IP configuration, Netmask. */
    char  gateway_ip_addr[16];    /**< Static IP configuration, Router IP address. */
    char  dnsServer_ip_addr[16];  /**< Static IP configuration, DNS server IP address. */
    char  dhcpMode;               /**< DHCP mode, @ref DHCP_Disable, @ref DHCP_Client and @ref DHCP_Server. */
    char  reserved[32];
    int   wifi_retry_interval;    /**< Retry interval if an error is occured when connecting an access point,
                                  time unit is millisecond. */
} network_InitTypeDef_adv_st;

/**
 *  @brief  Current link status in station mode.
 */
typedef struct _linkStatus_t
{
    int is_connected;       /**< The link to wlan is established or not, 0: disconnected, 1: connected. */
    int wifi_strength;      /**< Signal strength of the current connected AP */
    uint8_t  ssid[32];      /**< SSID of the current connected wlan */
    uint8_t  bssid[6];      /**< BSSID of the current connected wlan */
    int      channel;       /**< Channel of the current connected wlan */
} LinkStatusTypeDef;

/*WiFi Monitor */
/* @brief define the monitor callback function.
  * @param data: the 802.11 packet
  * @param len: the length of this packet, include FCS
  * @param rssi: the rssi of the received packet.
  */
typedef void (*monitor_cb_t)(uint8_t*data, int len);

#endif // CFG_MXCHIP

/** @brief  Connect or establish a Wi-Fi network in normal mode (station or soft ap mode).
 * 
 *  @detail This function can establish a Wi-Fi connection as a station or create
 *          a soft AP that other staions can connect (4 stations Max). In station mode,  
 *          MICO first scan all of the supported Wi-Fi channels to find a wlan that 
 *          matchs the input SSID, and read the security mode. Then try to connect    
 *          to the target wlan. If any error occurs in the connection procedure or  
 *          disconnected after a successful connection, MICO start the reconnection 
 *          procedure in backgound after a time interval defined in inNetworkInitPara.
 *          Call this function twice when setup coexistence mode (staion + soft ap). 
 *          This function retruns immediately in station mode, and the connection will 
 *          be executed in background.
 *
 *  @param  inNetworkInitPara: Specifies wlan parameters. 
 *
 *  @return In station mode, allways retrurn kWlanNoErr.
 *          In soft ap mode, return kWlanXXXErr
 */
OSStatus bk_wlan_start(network_InitTypeDef_st* inNetworkInitPara);

/** @brief  Connect to a Wi-Fi network with advantage settings (station mode only)
 * 
 *  @detail This function can connect to an access point with precise settings,
 *          that greatly speed up the connection if the input settings are correct
 *          and fixed. If this fast connection is failed for some reason, MICO 
 *          change back to normal: scan + connect mode refer to @ref bkWlanStart.
 *          This function returns after the fast connection try.
 *
 *  @note   This function cannot establish a soft ap, use StartNetwork() for this
 *          purpose. 
 *          If input SSID length is 0, MICO use BSSID to connect the target wlan.
 *          If both SSID and BSSID are all wrong, the connection will be failed.
 *
 *  @param  inNetworkInitParaAdv: Specifies the precise wlan parameters.
 *
 *  @retrun Allways return kWlanNoErr although error occurs in first fast try 
 *          kWlanTimeoutErr: DHCP client timeout
 */
OSStatus bk_wlan_start_adv(network_InitTypeDef_adv_st* inNetworkInitParaAdv);

/** @brief  Read current IP status on a network interface.
 * 
 *  @param  outNetpara: Point to the buffer to store the IP address. 
 *  @param  inInterface: Specifies wlan interface. 
 *             @arg Soft_AP: The soft AP that established by bkWlanStart()
 *             @arg Station: The interface that connected to an access point
 *
 *  @return   kNoErr        : on success.
 *  @return   kGeneralErr   : if an error occurred
 */
OSStatus bk_wlan_get_ip_status(IPStatusTypedef *outNetpara, WiFi_Interface inInterface);

/** @brief  Read current wireless link status on station interface.
 * 
 *  @param  outStatus: Point to the buffer to store the link status. 
 *
 *  @return   kNoErr        : on success.
 *  @return   kGeneralErr   : if an error occurred
 */
OSStatus bk_wlan_get_link_status(LinkStatusTypeDef *outStatus);

/** @brief  Start a wlan scanning in 2.4GHz in MICO backfround.
 *  
 *  @detail Once the scan is completed, MICO sends a notify: 
 *          bk_notify_WIFI_SCAN_COMPLETED, with callback function:
 *          void (*function)(ScanResult *pApList, bk_Context_t * const inContext)
 *          Register callback function using @ref bk_add_notification() before scan.
 */
void bk_wlan_start_scan(void);

/** @brief  Add the packet type which monitor should receive
 * 
 *  @detail This function can be called many times to receive different wifi packets.
 */
int bk_wlan_monitor_rx_type(int type);

/** @brief  Start wifi monitor mode
 * 
 *  @detail This function disconnect wifi station and softAP. 
 *       
 */
int bk_wlan_start_monitor(void);

/** @brief  Stop wifi monitor mode
 * 
 */
int bk_wlan_stop_monitor(void);

/** @brief  Set the monitor channel
 * 
 *  @detail This function change the monitor channel (from 1~13).
 *       it can change the channel dynamically, don't need restart monitor mode.
 */
int bk_wlan_set_channel(int channel);

/** @brief  Register the monitor callback function
 *        Once received a 802.11 packet call the registered function to return the packet.
 */
void bk_wlan_register_monitor_cb(monitor_cb_t fn);
monitor_cb_t bk_wlan_get_monitor_cb(void);
int bk_wlan_is_monitor_mode(void);
uint32_t bk_wlan_is_ap(void);
uint32_t bk_wlan_is_sta(void);
uint32_t bk_sta_cipher_is_open(void);
uint32_t bk_sta_cipher_is_wep(void);
#endif// _WLAN_UI_PUB_
