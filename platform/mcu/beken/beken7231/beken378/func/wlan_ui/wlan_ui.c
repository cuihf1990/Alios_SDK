#include "include.h"
#include "common.h"

#include "wlan_ui_pub.h"
#include "rw_pub.h"
#include "vif_mgmt.h"

#include "sa_station.h"
#include "param_config.h"
#include "common/ieee802_11_defs.h"
#include "driver_beken.h"
#include "mac_ie.h"
#include "sa_ap.h"
#include "main_none.h"
#include "sm.h"
#include "mac.h"
#include "scan_task.h"
#include "hal_machw.h"

monitor_cb_t g_monitor_cb = 0;

extern int connect_flag;
extern struct assoc_ap_info assoc_ap;

static void rwnx_remove_added_interface(void)
{
    struct vif_info_tag *p_vif_entry = vif_mgmt_first_used();

    while (p_vif_entry != NULL)
    {
        mt_msg_dispatch(MM_REMOVE_IF_REQ, &p_vif_entry->index);
        p_vif_entry = vif_mgmt_next(p_vif_entry);
    }
}

void bk_wlan_connection_loss(void)
{
    struct vif_info_tag *p_vif_entry = vif_mgmt_first_used();

    while (p_vif_entry != NULL)
    {
        os_printf("bk_wlan_connection_loss\r\n");
        sta_ip_down();

        mt_msg_dispatch(MM_CONNECTION_LOSS_IND, &p_vif_entry->index);
        p_vif_entry = vif_mgmt_next(p_vif_entry);
    }
}

static int ip_aton(const char *cp, UINT32 *addr)
{
    char c;
    UINT8 base;
    UINT32 val;
    UINT32 parts[4];
    UINT32 *pp = parts;

    c = *cp;
    for (;;)
    {
        if (!isdigit(c))
            return (0);
        val = 0;
        base = 10;
        if (c == '0')
        {
            c = *++cp;
            if (c == 'x' || c == 'X')
            {
                base = 16;
                c = *++cp;
            }
            else
                base = 8;
        }
        for (;;)
        {
            if (isdigit(c))
            {
                val = (val * base) + (int)(c - '0');
                c = *++cp;
            }
            else if (base == 16 && isxdigit(c))
            {
                val = (val << 4) | (int)(c + 10 - (islower(c) ? 'a' : 'A'));
                c = *++cp;
            }
            else
                break;
        }
        if (c == '.')
        {
            if (pp >= parts + 3)
            {
                return (0);
            }
            *pp++ = val;
            c = *++cp;
        }
        else
            break;
    }
    if (c != '\0' && !isspace(c))
    {
        return (0);
    }
    switch (pp - parts + 1)
    {

    case 0:
        return (0);

    case 1:
        break;

    case 2:
        if (val > 0xffffffUL)
        {
            return (0);
        }
        val |= parts[0] << 24;
        break;

    case 3:
        if (val > 0xffff)
        {
            return (0);
        }
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4:
        if (val > 0xff)
        {
            return (0);
        }
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    default:
        break;
    }

    *addr = val;
    return (1);
}

static char *ip_ntoa(UINT32 addr, char *buf, int buflen)
{
    UINT32 s_addr;
    char inv[3];
    char *rp;
    UINT8 *ap;
    UINT8 rem;
    UINT8 n;
    UINT8 i;
    int len = 0;

    s_addr = addr;

    rp = buf;
    ap = (UINT8 *)&s_addr;
    for(n = 0; n < 4; n++)
    {
        i = 0;
        do
        {
            rem = *ap % (UINT8)10;
            *ap /= (UINT8)10;
            inv[i++] = '0' + rem;
        }
        while(*ap);
        while(i--)
        {
            if (len++ >= buflen)
            {
                return NULL;
            }
            *rp++ = inv[i];
        }
        if (len++ >= buflen)
        {
            return NULL;
        }
        *rp++ = '.';
        ap++;
    }
    *--rp = 0;
    return buf;
}

uint32_t bk_wlan_is_ap(void)
{
    ASSERT(g_wlan_general_param);
    return (CONFIG_ROLE_AP == g_wlan_general_param->role);
}

uint32_t bk_wlan_is_sta(void)
{
    ASSERT(g_wlan_general_param);
    return (CONFIG_ROLE_STA == g_wlan_general_param->role);
}

uint32_t bk_sta_cipher_is_open(void)
{
    ASSERT(g_sta_param_ptr);
    return (CONFIG_CIPHER_OPEN == g_sta_param_ptr->cipher_suite);
}

uint32_t bk_sta_cipher_is_wep(void)
{
    ASSERT(g_sta_param_ptr);
    return (CONFIG_CIPHER_WEP == g_sta_param_ptr->cipher_suite);
}

void bk_wlan_ap_init(network_InitTypeDef_st *inNetworkInitPara)
{
    os_printf("Soft_AP_start\r\n");

    if(!g_ap_param_ptr)
    {
        g_ap_param_ptr = (ap_param_t *)os_zalloc(sizeof(ap_param_t));
        ASSERT(g_ap_param_ptr);
    }

    if(MAC_ADDR_NULL((u8 *)&g_ap_param_ptr->bssid))
    {
        wifi_get_mac_address((u8 *)&g_ap_param_ptr->bssid);
    }

    g_ap_param_ptr->chann = CFG_CHANNEL_AP;

    if(!g_wlan_general_param)
    {
        g_wlan_general_param = (general_param_t *)os_zalloc(sizeof(general_param_t));
        ASSERT(g_wlan_general_param);
    }
    g_wlan_general_param->role = CONFIG_ROLE_AP;

    if(inNetworkInitPara)
    {
        g_ap_param_ptr->ssid.length = os_strlen(inNetworkInitPara->wifi_ssid);
        os_memcpy(g_ap_param_ptr->ssid.array, inNetworkInitPara->wifi_ssid, g_ap_param_ptr->ssid.length);
        g_ap_param_ptr->key_len = os_strlen(inNetworkInitPara->wifi_key);
        if(g_ap_param_ptr->key_len < 8)
        {
            g_ap_param_ptr->cipher_suite = CONFIG_CIPHER_OPEN;
        }
        else
        {
            g_ap_param_ptr->cipher_suite = CONFIG_CIPHER_CCMP;
            os_memcpy(g_ap_param_ptr->key, inNetworkInitPara->wifi_key, g_ap_param_ptr->key_len);
        }

        if(inNetworkInitPara->dhcpMode == DHCP_Server)
        {
            g_wlan_general_param->dhcp_enable = 1;
        }
        else
        {
            g_wlan_general_param->dhcp_enable = 0;
        }
        ip_aton(inNetworkInitPara->local_ip_addr, &(g_wlan_general_param->ip_addr));
        ip_aton(inNetworkInitPara->net_mask, &(g_wlan_general_param->ip_mask));
        ip_aton(inNetworkInitPara->gateway_ip_addr, &(g_wlan_general_param->ip_gw));
    }

    sa_ap_init();
}

void bk_wlan_sta_init(network_InitTypeDef_st *inNetworkInitPara)
{
    if(!g_sta_param_ptr)
    {
        g_sta_param_ptr = (sta_param_t *)os_zalloc(sizeof(sta_param_t));
        ASSERT(g_sta_param_ptr);
    }

    wifi_get_mac_address((u8 *)&g_sta_param_ptr->own_mac);
    if(!g_wlan_general_param)
    {
        g_wlan_general_param = (general_param_t *)os_zalloc(sizeof(general_param_t));
        ASSERT(g_wlan_general_param);
    }
    g_wlan_general_param->role = CONFIG_ROLE_STA;

    if(inNetworkInitPara)
    {
        g_sta_param_ptr->ssid.length = os_strlen(inNetworkInitPara->wifi_ssid);
        os_memcpy(g_sta_param_ptr->ssid.array,
                  inNetworkInitPara->wifi_ssid,
                  g_sta_param_ptr->ssid.length);


        g_sta_param_ptr->key_len = os_strlen(inNetworkInitPara->wifi_key);
		os_memcpy(g_sta_param_ptr->key, inNetworkInitPara->wifi_key, g_sta_param_ptr->key_len);

        if(inNetworkInitPara->dhcpMode == DHCP_Client)
        {
            g_wlan_general_param->dhcp_enable = 1;
        }
        else
        {
            g_wlan_general_param->dhcp_enable = 0;
            ip_aton(inNetworkInitPara->local_ip_addr, &(g_wlan_general_param->ip_addr));
            ip_aton(inNetworkInitPara->net_mask, &(g_wlan_general_param->ip_mask));
            ip_aton(inNetworkInitPara->gateway_ip_addr, &(g_wlan_general_param->ip_gw));
        }
    }

    sa_station_init();
}

OSStatus bk_wlan_start(network_InitTypeDef_st *inNetworkInitPara)
{
    if(inNetworkInitPara->wifi_mode == Soft_AP)
    {
    	hostapd_thread_stop();
		supplicant_main_exit();
        bk_wlan_ap_init(inNetworkInitPara);

        os_printf("lwip_intf_initial\r\n");
        ip_address_set(Soft_AP, 0, inNetworkInitPara->local_ip_addr,
                       inNetworkInitPara->net_mask,
                       inNetworkInitPara->gateway_ip_addr,
                       inNetworkInitPara->dnsServer_ip_addr);
        

        os_printf("wpa_main_entry\r\n");
        wpa_main_entry(2, 0);

        sm_build_broadcast_deauthenticate();
		uap_ip_start();
    }
    else if(inNetworkInitPara->wifi_mode == Station)
    {
    	hostapd_thread_stop();
        supplicant_main_exit();
        sta_ip_down();
        ip_address_set(Station, inNetworkInitPara->dhcpMode, inNetworkInitPara->local_ip_addr,
                       inNetworkInitPara->net_mask, inNetworkInitPara->gateway_ip_addr,
                       inNetworkInitPara->dnsServer_ip_addr);
        bk_wlan_sta_init(inNetworkInitPara);
        supplicant_main_entry(inNetworkInitPara->wifi_ssid);
    }

    return 0;
}

extern int sa_ap_inited();
extern int sa_sta_inited();

void bk_wlan_start_scan(void)
{
    SCAN_PARAM_T scan_param = {0};

	if ((sa_ap_inited() == 0) && (sa_sta_inited() == 0))
    	bk_wlan_sta_init(0);

    os_memset(&scan_param.bssid, 0xff, ETH_ALEN);
    mt_msg_dispatch(SCANU_START_REQ, &scan_param);
}

void bk_wlan_sta_init_adv(network_InitTypeDef_adv_st *inNetworkInitParaAdv)
{
	int valid_ap = 1;
	
    if(!g_sta_param_ptr)
    {
        g_sta_param_ptr = (sta_param_t *)os_malloc(sizeof(sta_param_t));
        ASSERT(g_sta_param_ptr);
    }

    if(MAC_ADDR_NULL((u8 *)&g_sta_param_ptr->own_mac))
    {
        wifi_get_mac_address((char *)&g_sta_param_ptr->own_mac);
    }

	if (MAC_ADDR_NULL(inNetworkInitParaAdv->ap_info.bssid)) {
		valid_ap = 0;
	}

	if (((inNetworkInitParaAdv->ap_info.channel <= 0)) || 
		 (inNetworkInitParaAdv->ap_info.channel > 13)) {
		valid_ap = 0;
	}
	
    g_sta_param_ptr->ssid.length = os_strlen(inNetworkInitParaAdv->ap_info.ssid);
    os_memcpy(g_sta_param_ptr->ssid.array, inNetworkInitParaAdv->ap_info.ssid, g_sta_param_ptr->ssid.length);

    switch(inNetworkInitParaAdv->ap_info.security)
    {
    case SECURITY_TYPE_NONE:
        g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_OPEN;
        break;
    case SECURITY_TYPE_WEP:
        g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_WEP;
        break;
    case SECURITY_TYPE_WPA_TKIP:
    case SECURITY_TYPE_WPA2_TKIP:
        g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_TKIP;
        break;
    case SECURITY_TYPE_WPA_AES:
    case SECURITY_TYPE_WPA2_AES:
        g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_CCMP;
        break;
    case SECURITY_TYPE_WPA2_MIXED:
        g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_MIXED;
        break;
    default:
		valid_ap = 0;
        break;
    }

	if (valid_ap) {
    g_sta_param_ptr->fast_connect_set = 1;
    g_sta_param_ptr->fast_connect.chann = inNetworkInitParaAdv->ap_info.channel;
    os_memcpy(g_sta_param_ptr->fast_connect.bssid, inNetworkInitParaAdv->ap_info.bssid, ETH_ALEN);
	}
    g_sta_param_ptr->key_len = inNetworkInitParaAdv->key_len;
    os_memcpy((uint8_t *)g_sta_param_ptr->key, inNetworkInitParaAdv->key, inNetworkInitParaAdv->key_len);

    if(!g_wlan_general_param)
    {
        g_wlan_general_param = (general_param_t *)os_malloc(sizeof(general_param_t));
    }
    g_wlan_general_param->role = CONFIG_ROLE_STA;
    if(inNetworkInitParaAdv->dhcpMode == DHCP_Client)
    {
        g_wlan_general_param->dhcp_enable = 1;
    }
    else
    {
        g_wlan_general_param->dhcp_enable = 0;
        ip_aton(inNetworkInitParaAdv->local_ip_addr, &(g_wlan_general_param->ip_addr));
        ip_aton(inNetworkInitParaAdv->net_mask, &(g_wlan_general_param->ip_mask));
        ip_aton(inNetworkInitParaAdv->gateway_ip_addr, &(g_wlan_general_param->ip_gw));
    }
    sa_station_init();
}

OSStatus bk_wlan_start_adv(network_InitTypeDef_adv_st *inNetworkInitParaAdv)
{
    hostapd_thread_stop();
    supplicant_main_exit();
    sta_ip_down();
    ip_address_set(Station, inNetworkInitParaAdv->dhcpMode,
                   inNetworkInitParaAdv->local_ip_addr,
                   inNetworkInitParaAdv->net_mask,
                   inNetworkInitParaAdv->gateway_ip_addr,
                   inNetworkInitParaAdv->dnsServer_ip_addr);

    bk_wlan_sta_init_adv(inNetworkInitParaAdv);
    supplicant_main_entry(inNetworkInitParaAdv->ap_info.ssid);

    return 0;
}

OSStatus bk_wlan_get_ip_status(IPStatusTypedef *outNetpara, WiFi_Interface inInterface)
{
    uint8_t mac[6];
	
    if(g_wlan_general_param->dhcp_enable)
    {
        outNetpara->dhcp = DHCP_Server;
    }
    else
    {
        outNetpara->dhcp = DHCP_Disable;
    }
    ip_ntoa(g_wlan_general_param->ip_addr, outNetpara->ip, 16);
    ip_ntoa(g_wlan_general_param->ip_gw, outNetpara->gate, 16);
    ip_ntoa(g_wlan_general_param->ip_mask, outNetpara->mask, 16);

    mico_wlan_get_mac_address(mac);
	sprintf(outNetpara->mac, "%02x%02x%02x%02x%02x%02x", mac[0],
			mac[1], mac[2], mac[3], mac[4], mac[5]);

    return 0;
}

OSStatus bk_wlan_get_link_status(LinkStatusTypeDef *outStatus)
{
    if(g_wlan_general_param->role == CONFIG_ROLE_AP)
    {
        return -1;
    }
    outStatus->is_connected = connect_flag;
    os_memcpy(outStatus->bssid, assoc_ap.bssid, 6);
    os_memcpy(outStatus->ssid, assoc_ap.ssid, assoc_ap.ssid_len);
    outStatus->channel = assoc_ap.chann;
    outStatus->wifi_strength = assoc_ap.level;

    return 0;
}

/** @brief  Add the packet type which monitor should receive
 *
 *  @detail This function can be called many times to receive different wifi packets.
 */
int bk_wlan_monitor_rx_type(int type)
{
    unsigned int filter = 0;
    switch(type)
    {
    case WLAN_RX_BEACON:
        nxmac_accept_beacon_setf(1);
        break;
    case WLAN_RX_PROBE_REQ:
        nxmac_accept_probe_req_setf(1);
        break;
    case WLAN_RX_PROBE_RES:
        nxmac_accept_probe_resp_setf(1);
        break;
    case WLAN_RX_ACTION:
        break;
    case WLAN_RX_MANAGEMENT:
        nxmac_accept_other_mgmt_frames_setf(1);
        break;
    case WLAN_RX_DATA:
        nxmac_accept_other_data_frames_setf(1);
        nxmac_accept_qo_s_null_setf(1);
        nxmac_accept_qcfwo_data_setf(1);
        nxmac_accept_q_data_setf(1);
        nxmac_accept_cfwo_data_setf(1);
        nxmac_accept_data_setf(1);
        break;
    case WLAN_RX_MCAST_DATA:
        nxmac_accept_multicast_setf(1);
        nxmac_accept_broadcast_setf(1);
        break;
    case WLAN_RX_ALL:
        nxmac_rx_cntrl_set((uint32_t)0x7FFFFFFC);
        break;
    }

    mt_msg_dispatch(MM_SET_FILTER_REQ, &filter);
    return 0;
}

/** @brief  Start wifi monitor mode
 *
 *  @detail This function disconnect wifi station and softAP.
 *
 */
int bk_wlan_start_monitor(void)
{
    bk_wlan_ap_init(0);
    rwnx_remove_added_interface();
    g_wlan_general_param->role = CONFIG_ROLE_NULL;

    return 0;
}

/** @brief  Stop wifi monitor mode
 *
 */
int bk_wlan_stop_monitor(void)
{
    if(g_monitor_cb)
    {
        g_monitor_cb = 0;
        hal_machw_exit_monitor_mode();
    }
    return 0;
}

/** @brief  Set the monitor channel
 *
 *  @detail This function change the monitor channel (from 1~13).
 *       it can change the channel dynamically, don't need restart monitor mode.
 */
int bk_wlan_set_channel(int channel)
{
	rwnxl_reset_evt(0);
    mt_msg_dispatch(MM_SET_CHANNEL_REQ, &channel);
    return 0;
}

/** @brief  Register the monitor callback function
 *        Once received a 802.11 packet call the registered function to return the packet.
 */
void bk_wlan_register_monitor_cb(monitor_cb_t fn)
{
    g_monitor_cb = fn;
}

monitor_cb_t bk_wlan_get_monitor_cb(void)
{
    return g_monitor_cb;
}

int bk_wlan_is_monitor_mode(void)
{
    return (0 == g_monitor_cb) ? FALSE : TRUE;
}
// eof

