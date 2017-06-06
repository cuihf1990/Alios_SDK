/**
 ******************************************************************************
 * @file    mico_wlan.h
 * @author  William Xu
 * @version V1.0.0
 * @date    16-Sep-2014
 * @brief   This file provides all the headers of wlan connectivity functions.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */	 
#include "include.h"
#include "rtos_pub.h"
#include "wlan_ui_pub.h"

void mico_wlan_get_mac_address( uint8_t *mac )
{
}

OSStatus micoWlanStart(network_InitTypeDef_st* inNetworkInitPara)
{
	return bk_wlan_start(inNetworkInitPara);
}

OSStatus micoWlanStartAdv(network_InitTypeDef_adv_st* inNetworkInitParaAdv)
{
	return bk_wlan_start_adv(inNetworkInitParaAdv);
}

OSStatus micoWlanGetIPStatus(IPStatusTypedef *outNetpara, WiFi_Interface inInterface)
{
	return bk_wlan_get_ip_status(outNetpara,inInterface);
}

OSStatus micoWlanGetLinkStatus(LinkStatusTypeDef *outStatus)
{
	return bk_wlan_get_link_status(outStatus);
}

void micoWlanStartScan(void)
{
	bk_wlan_start_scan();
}

void micoWlanStartScanAdv(void)
{
}

OSStatus micoWlanPowerOff(void)
{
	return 0;
}

OSStatus micoWlanPowerOn(void)
{
	return 0;
}

OSStatus micoWlanSuspend(void)
{
	return 0;
}

OSStatus micoWlanSuspendStation(void)
{
	return 0;
}

OSStatus micoWlanSuspendSoftAP(void)
{
	return 0;
}

OSStatus micoWlanStartEasyLink(int inTimeout)
{
	return 0;
}

OSStatus micoWlanStartEasyLinkPlus(int inTimeout)
{
	return 0;
}

OSStatus micoWlanStopEasyLink(void)
{
	return 0;
}
OSStatus micoWlanStopEasyLinkPlus(void)
{
	return 0;
}

OSStatus micoWlanStartWPS(int inTimeout)
{
	return 0;
}

OSStatus micoWlanStopWPS(void)
{
	return 0;
}

OSStatus micoWlanStartAirkiss(int inTimeout)
{
	return 0;
}

OSStatus micoWlanStopAirkiss(void)
{
	return 0;
}

void micoWlanEnablePowerSave(void)
{
	return;
}

void micoWlanDisablePowerSave(void)
{
	return;
} 

void wifimgr_debug_enable(bool enable)
{
	return;
}

int mico_wlan_monitor_rx_type(int type)
{
	return bk_wlan_monitor_rx_type(type);
}

int mico_wlan_start_monitor(void)
{
	return bk_wlan_start_monitor();
}

int mico_wlan_stop_monitor(void)
{
	return bk_wlan_stop_monitor();
}

int mico_wlan_set_channel(int channel)
{
	return bk_wlan_set_channel(channel);
}

void mico_wlan_register_monitor_cb(monitor_cb_t fn)
{
	bk_wlan_register_monitor_cb(fn);
}
// eof

