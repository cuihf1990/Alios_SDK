/**
 ****************************************************************************************
 *
 * @file arch_config.c
 *
 *
 * Copyright (C) Beken Corp 2011-2016
 *
 ****************************************************************************************
 */
#include "include.h"
#include "mem_pub.h"
#include "drv_model_pub.h"
#include "flash_pub.h"
#include "mac.h"
#include "param_config.h"
#include "uart_pub.h"
#include "mico_wlan.h"

general_param_t *g_wlan_general_param = NULL;
ap_param_t *g_ap_param_ptr = NULL;
sta_param_t *g_sta_param_ptr = NULL;
static void load_mac(void);

uint32_t cfg_param_init(void)
{
	if(NULL == g_wlan_general_param)
	{
		g_wlan_general_param = (general_param_t *)os_zalloc(sizeof(general_param_t));
		ASSERT(g_wlan_general_param);
	}
	if(NULL == g_ap_param_ptr)
	{
		g_ap_param_ptr = (ap_param_t *)os_zalloc(sizeof(ap_param_t));
		ASSERT(g_ap_param_ptr);
	}
	if(NULL == g_sta_param_ptr)
	{
		g_sta_param_ptr = (sta_param_t *)os_zalloc(sizeof(sta_param_t));
		ASSERT(g_sta_param_ptr);
	}
}

#if 1
#define SYSTEM_DATA_ADDR 0xFE000
#define DEFAULT_MAC_ADDR "\xC8\x93\x48\x22\x22\x01"
static uint8_t system_mac[6] = DEFAULT_MAC_ADDR;

static void load_mac(void)
{
	UINT32 status;
	DD_HANDLE flash_handle;
	
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char *)system_mac, sizeof(system_mac), SYSTEM_DATA_ADDR);
	
	if (system_mac[0] == 0xFF) 
	{
		os_memcpy(system_mac, DEFAULT_MAC_ADDR, 6);
	}
}

/* yhb added, save MAC to address 0xFE000 */
void wifi_get_mac_address(char *mac)
{
	static int mac_inited = 0;
	
	if (mac_inited == 0) 
	{
		load_mac();
		mac_inited = 1;
	}
	memcpy(mac, system_mac, 6);
}

int wifi_set_mac_address(char *mac)
{
    DD_HANDLE flash_handle;
	UINT32 status;

	memcpy(system_mac, mac, 6);
    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_write(flash_handle, (char *)system_mac, sizeof(system_mac), SYSTEM_DATA_ADDR);
}
#endif

UINT32 cfg_search_by_type(UINT32 type, UINT32 start_addr)
{
    UINT32 status, addr, end_addr;
    DD_HANDLE flash_handle;
    head_param_t head;

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), start_addr);
    addr = start_addr + sizeof(head_param_t);
    end_addr = addr + head.len;
    while(addr < end_addr)
    {
        ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
        if(type == head.type)
        {
            break;
        }
        else
        {
            addr += sizeof(head_param_t);
            addr += head.len;
        }
    }

    if(addr >= end_addr)
    {
        addr = 0;
    }
    ddev_close(flash_handle);

    return addr;
}

int cfg_get_general_params(void)
{
    if(!g_wlan_general_param)
    {
        g_wlan_general_param = (general_param_t *)os_malloc(sizeof(general_param_t));
    }
    return 0;
}

int cfg_get_ap_params(void)
{
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    head_param_t head;

    if(!g_ap_param_ptr)
    {
        g_ap_param_ptr = (ap_param_t *)os_malloc(sizeof(ap_param_t));
    }

    addr_start = cfg_search_by_type(PARAM_CFG_AP, CONFIG_ADDR_START);
    if(!addr_start)
    {
        PARAM_CFG_PRT("[AP]SEARCH AP CLASS FAIL\r\n");
        return -1;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);

    addr = cfg_search_by_type(AP_TYPE_BSSID, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[AP]SEARCH BSSID FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&g_ap_param_ptr->bssid, head.len, addr + sizeof(head_param_t));

    addr = cfg_search_by_type(AP_TYPE_SSID, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[AP]SEARCH SSID FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    g_ap_param_ptr->ssid.length = head.len;
    ddev_read(flash_handle, (char *)g_ap_param_ptr->ssid.array, head.len, addr + sizeof(head_param_t));

    addr = cfg_search_by_type(AP_TYPE_CHANNEL, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[AP]SEARCH CHANNEL FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&g_ap_param_ptr->chann, head.len, addr + sizeof(head_param_t));

    addr = cfg_search_by_type(AP_TYPE_MODE, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[AP]SEARCH MODE FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&g_ap_param_ptr->cipher_suite, head.len, addr + sizeof(head_param_t));

    addr = cfg_search_by_type(AP_TYPE_PASSWD, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[AP]SEARCH PASSWORD FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    g_ap_param_ptr->key_len = head.len;
    ddev_read(flash_handle, (char *)g_ap_param_ptr->key, head.len, addr + sizeof(head_param_t));

    return 0;
}

int cfg_get_sta_params(void)
{
    UINT32 status, addr, addr_start;
    DD_HANDLE flash_handle;
    head_param_t head;

    if(!g_sta_param_ptr)
    {
        g_sta_param_ptr = (sta_param_t *)os_malloc(sizeof(sta_param_t));
    }

    addr_start = cfg_search_by_type(PARAM_CFG_STA, CONFIG_ADDR_START);
    if(!addr_start)
    {
        PARAM_CFG_PRT("[STA]SEARCH STATION CLASS FAIL\r\n");
        return -1;
    }

    flash_handle = ddev_open(FLASH_DEV_NAME, &status, 0);

    addr = cfg_search_by_type(STA_TYPE_MAC, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[STA]SEARCH MAC FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&g_sta_param_ptr->own_mac, head.len, addr + sizeof(head_param_t));

    addr = cfg_search_by_type(STA_TYPE_SSID, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[STA]SEARCH SSID FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    g_sta_param_ptr->ssid.length = head.len;
    ddev_read(flash_handle, (char *)g_sta_param_ptr->ssid.array, head.len, addr + sizeof(head_param_t));

    addr = cfg_search_by_type(STA_TYPE_MODE, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[STA]SEARCH MODE FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    ddev_read(flash_handle, (char *)&g_sta_param_ptr->cipher_suite, head.len, addr + sizeof(head_param_t));

    addr = cfg_search_by_type(STA_TYPE_PASSWD, addr_start);
    if(!addr)
    {
        PARAM_CFG_PRT("[STA]SEARCH PASSWORD FAIL\r\n");
        return -1;
    }
    ddev_read(flash_handle, (char *)&head, sizeof(head_param_t), addr);
    g_sta_param_ptr->key_len = head.len;
    ddev_read(flash_handle, (char *)g_sta_param_ptr->key, head.len, addr + sizeof(head_param_t));

    ddev_close(flash_handle);

    return 0;
}

int wpa_get_ap_security(apinfo_adv_t *ap, uint8_t **key, int *key_len)
{
	if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_WEP){
		ap->security = SECURITY_TYPE_WEP;			
	}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_TKIP){
		ap->security = SECURITY_TYPE_WPA2_TKIP;	
	}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_CCMP){
		ap->security = SECURITY_TYPE_WPA2_AES;	
	}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_MIXED){
		ap->security = SECURITY_TYPE_WPA2_MIXED;	
	} else {
		ap->security = SECURITY_TYPE_NONE;
	}
	memcpy(ap->ssid, g_sta_param_ptr->ssid.array, g_sta_param_ptr->ssid.length);
	memcpy(ap->bssid, g_sta_param_ptr->fast_connect.bssid, 6);
	ap->channel = g_sta_param_ptr->fast_connect.chann;
	*key = g_sta_param_ptr->key;
	*key_len = g_sta_param_ptr->key_len;

	return 0;
}

