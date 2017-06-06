#include "include.h"

#if CFG_WIFI_STATION_MODE
#include "schedule_pub.h"

#include "sa_station.h"
#include "drv_model_pub.h"
#include "uart_pub.h"
#include "scanu_task.h"
#include "scan_task.h"
#include "rxu_task.h"
#include "mm_task.h"
#include "me_task.h"
#include "sm_task.h"
#include "rw_msg_tx.h"
#include "mac_ie.h"
#include "vif_mgmt.h"

#include "arm_arch.h"
#include "mem_pub.h"
#include "rw_pub.h"
#include "common.h"

#if CFG_USE_LWIP_NETSTACK
#include "lwip_intf.h"
#include "app_lwip_udp.h"
#include "app_lwip_tcp.h"
#endif

#if CFG_USE_TEMPERATURE_DETECT
#include "temp_detect_pub.h"
#endif
#if CFG_MODE_SWITCH
#include "param_config.h"
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "rtos_pub.h"
#include "error.h"

extern struct mac_scan_result *scanu_search_by_ssid(struct mac_ssid const *ssid);

static uint32_t sa_sta_rsp_word = 0;
void sa_sta_clear_rsp_word(void)
{
	sa_sta_rsp_word = 0;
}

void sa_sta_set_rsp_word(uint32_t val)
{
	sa_sta_rsp_word = val;
}

/*---------------------------------------------------------------------------*/
struct scanu_rst_upload *sa_station_send_scan_cmd(SCAN_PARAM_T *scan_param)
{
    UINT8 *rsp_scan;
    UINT32 rsp_len;
    UINT32 vies_len;
    UINT8 *elmt_addr;
    struct ke_msg *msg;
    UINT8 *var_part_addr;
    SCAN_IND_PTR scanu_ret_ptr;
    IEEE802_11_PROBE_RSP_PTR probe_rsp_ieee80211_ptr;
    struct scanu_rst_upload *result_ptr;
    struct sta_scan_res *r;
    int i, ret = 0, find = 0;

    SASTA_PRT("[sa_sta]scan: SCANU_START_REQ\r\n");
    mt_msg_dispatch(SCANU_START_REQ, scan_param);

    rsp_len = sizeof(struct ke_msg) + SCANU_IND_PAYLOAD_LEN;
    rsp_scan = (UINT8 *)os_malloc(rsp_len);
    if(!rsp_scan)
    {
        SASTA_PRT("[sa_sta]scan resp no buffer\r\n");
        return NULL;
    }

    result_ptr = os_zalloc(sizeof(*result_ptr));
    if(!result_ptr)
    {
        SASTA_PRT("[sa_sta]scan result no buffer\r\n");
        os_free(rsp_scan);
        return NULL;
    }
    result_ptr->res = os_calloc(MAX_BSS_LIST, sizeof(struct sta_scan_res *));
    if(!result_ptr->res)
    {
        SASTA_PRT("[sa_sta]scan res no buffer\r\n");
        os_free(rsp_scan);
        os_free(result_ptr);
        return NULL;
    }

    while(1)
    {
        if(mr_kmsg_fetch(rsp_scan, rsp_len))
        {
            msg = (struct ke_msg *)rsp_scan;
            if(SCANU_START_CFM == msg->id)
            {
                break;
            }
            else if(SCANU_RESULT_IND == msg->id)
            {
                scanu_ret_ptr = (SCAN_IND_PTR)msg->param;
                probe_rsp_ieee80211_ptr =  (IEEE802_11_PROBE_RSP_PTR)scanu_ret_ptr->payload;
                vies_len = scanu_ret_ptr->length - MAC_BEACON_VARIABLE_PART_OFT;
                var_part_addr = probe_rsp_ieee80211_ptr->rsp.variable;

                find = 0;
                for(i = 0; i < MAX_BSS_LIST; i++)
                {
                    if(result_ptr->res[i])
                    {
                        if(!os_memcmp(probe_rsp_ieee80211_ptr->bssid, result_ptr->res[i]->bssid, ETH_ALEN))
                        {
                            find = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                if(!find)
                {
                    r = os_zalloc(sizeof(*r) + vies_len);
                    if(!r)
                    {
                        ret = -1;
                        break;
                    }
                    os_memcpy(r->bssid, probe_rsp_ieee80211_ptr->bssid, ETH_ALEN);
                    r->freq = scanu_ret_ptr->center_freq;
                    r->beacon_int = probe_rsp_ieee80211_ptr->rsp.beacon_int;
                    r->caps = probe_rsp_ieee80211_ptr->rsp.capab_info;
                    r->level = scanu_ret_ptr->rssi;
                    os_memcpy(r->tsf, probe_rsp_ieee80211_ptr->rsp.timestamp, 8);
                    r->ie_len = vies_len;
                    os_memcpy(r + 1, var_part_addr, vies_len);

                    result_ptr->res[result_ptr->scanu_num++] = r;
                }

                SASTA_PRT("____center:%d\r\n", scanu_ret_ptr->center_freq);
                SASTA_PRT("____band:%d\r\n", scanu_ret_ptr->band);
                SASTA_PRT("____rssi:%d\r\n", scanu_ret_ptr->rssi);
                SASTA_PRT("____bssid:%2x:%2x:%2x:%2x:%2x:%2x\r\n"
                          , probe_rsp_ieee80211_ptr->bssid[0]
                          , probe_rsp_ieee80211_ptr->bssid[1]
                          , probe_rsp_ieee80211_ptr->bssid[2]
                          , probe_rsp_ieee80211_ptr->bssid[3]
                          , probe_rsp_ieee80211_ptr->bssid[4]
                          , probe_rsp_ieee80211_ptr->bssid[5]);

                elmt_addr = (UINT8 *)mac_ie_find((UINT32)var_part_addr, (UINT16)vies_len, MAC_ELTID_SSID);
                if (elmt_addr)
                {
                    char ssid[MAC_SSID_LEN + 1];
                    UINT8 ssid_len = *(elmt_addr + MAC_SSID_LEN_OFT);

                    if (ssid_len > MAC_SSID_LEN)
                        ssid_len = MAC_SSID_LEN;

                    os_memcpy(&ssid[0], elmt_addr + MAC_SSID_SSID_OFT, ssid_len);
                    ssid[ssid_len] = '\0';
                    SASTA_PRT("____ssid:%s\r\n", ssid);
                }

                SASTA_PRT("\r\n");
            }
        }

        process_run();
    }

    os_free(rsp_scan);
    rsp_scan = NULL;

    if(ret)
    {
        for (i = 0; i < result_ptr->scanu_num; i++)
        {
            os_free(result_ptr->res[i]);
        }

        os_free(result_ptr->res);
        os_free(result_ptr);
        result_ptr = NULL;
    }

    return result_ptr;
}

/*---------------------------------------------------------------------------*/
int sa_station_send_associate_cmd(CONNECT_PARAM_T *connect_param)
{
    struct ke_msg *msg;
    struct sm_connect_indication *conn_ind_ptr;
    struct mac_scan_result *desired_ap_ptr;

	if(g_sta_param_ptr->fast_connect_set)
	{
		g_sta_param_ptr->fast_connect_set = 0;
		connect_param->chan.freq = rw_ieee80211_get_centre_frequency(g_sta_param_ptr->fast_connect.chann);
		connect_param->chan.band = 0;
		connect_param->chan.flags = 0;
		connect_param->chan.tx_power = 0;
	}
	else
	{
	    desired_ap_ptr = scanu_search_by_ssid((void *)&connect_param->ssid);
	    if(NULL == desired_ap_ptr)
	    {
	        return -1;
	    }
	    connect_param->chan = *(desired_ap_ptr->chan);
		if(0 == connect_param->chan.tx_power)
		{
			connect_param->chan.tx_power = 10;
		}
	}

    mt_msg_dispatch(SM_CONNECT_REQ, connect_param);

    return 0;
}
/*---------------------------------------------------------------------------*/
int sa_station_send_disassociate_cmd(DISCONNECT_PARAM_T *disconnect_param)
{
	mt_msg_dispatch(SM_DISCONNECT_REQ, disconnect_param);

	return 0;
}

/*---------------------------------------------------------------------------*/
void sa_station_send_and_wait_rsp(UINT16 tx_cmd, void *param, UINT16 rx_rsp)
{
    mt_msg_dispatch(tx_cmd, param);

    while(1)
    {
        if(rx_rsp != sa_sta_rsp_word)
        {
        	vTaskDelay(10);
        }
		else
		{
			sa_sta_clear_rsp_word();
			break;
		}
    }
}

/*---------------------------------------------------------------------------*/
static void sa_station_cfg80211_init(void)
{
    UINT32 intf_type = VIF_STA;

    SASTA_PRT("[sa_sta]MM_RESET_REQ\r\n");
    sa_station_send_and_wait_rsp(MM_RESET_REQ, 0, MM_RESET_CFM);

    SASTA_PRT("[sa_sta]ME_CONFIG_REQ\r\n");
    sa_station_send_and_wait_rsp(ME_CONFIG_REQ, 0, ME_CONFIG_CFM);

    SASTA_PRT("[sa_sta]ME_CHAN_CONFIG_REQ\r\n");
    sa_station_send_and_wait_rsp(ME_CHAN_CONFIG_REQ, 0, ME_CHAN_CONFIG_CFM);

    SASTA_PRT("[sa_sta]MM_START_REQ\r\n");
    sa_station_send_and_wait_rsp(MM_START_REQ, 0, MM_START_CFM);

    SASTA_PRT("[sa_sta]MM_ADD_IF_REQ\r\n");
    sa_station_send_and_wait_rsp(MM_ADD_IF_REQ, &intf_type, MM_ADD_IF_CFM);
}

#ifndef DISABLE_RECONNECT

xTaskHandle  reconnect_thread_handle = NULL;
uint32_t  reconnect_stack_size = 2000;

void sa_reconnect_main(void *arg)
{  
	sa_station_init();     
	os_printf("sa_reconnect_main\r\n");

	rtos_delete_thread( NULL );
	reconnect_thread_handle = NULL;
}

void sa_reconnect_init(void)
{
    OSStatus ret; 

	if(NULL == reconnect_thread_handle)
	{
	    ret = rtos_create_thread(&reconnect_thread_handle, 
	            THD_RECONNECT_PRIORITY,
	            "reconnect_thread", 
	            (beken_thread_function_t)sa_reconnect_main, 
	            (unsigned short)reconnect_stack_size, 
	            (beken_thread_arg_t)0);
	    ASSERT(kNoErr == ret);
	}
	else
	{   
		os_printf("sa_reconnect_init_strange\r\n");
	}
}
#endif

void sa_station_init(void)
{
    sa_station_cfg80211_init();
}

void sa_station_uninit(void)
{
}
/*---------------------------------------------------------------------------*/
#endif // CFG_WIFI_STATION_MODE

// eof

