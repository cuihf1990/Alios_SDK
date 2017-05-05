/****************************************************************************
 * Copyright (C) 2015 The YunOS Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "ble_hci_adaptor.h"
#include "ble_gap.h"
#include "ble_hcicmds.h"

/*****************************************************
* Private Data Define
*****************************************************/
#define mxd_printf printf

/*****************************************************
* Private Data Define
*****************************************************/
static ble_gap_info_t gap_info;

//mesh
//int hci_le_adv_report_bearer_cb(uint8_t * data, uint8_t len)
//{
//    return 0;
//}

//gap.c
int gap_init(void)
{
    memset(&gap_info, 0, sizeof(gap_info));

    //call hci,will return in hci_adaptor and call gap_set_bd_addr set gap_info
    btsnd_hcic_read_bd_addr();
}

void gap_set_bd_addr(BD_ADDR bda)
{
    memcpy(gap_info.bda_public, bda, sizeof(BD_ADDR));
}

void gap_set_random_bd_addr(BD_ADDR bda)
{
    memcpy(gap_info.bda_random, bda, sizeof(BD_ADDR));
    btsnd_hcic_ble_set_random_addr(bda);
}

void gap_scan_adv_report_CB(u8* data, u8 len)
{
    ;
}

void gap_conn_cmplt_CB(u8* data, u8 len)
{
    u8* p = data;
    u8 status;
    STREAM_TO_UINT8(status, p);
    if (status)
    {
        mxd_printf("[GAP]le_conn_cmplt fail status =0x%x \n", status);
        //if (gap_info.conn_CB)
        //    gap_info.conn_CB(FALSE,gap_info.Peer_Address);
        return;
    }

    //set gap state
    gap_set_state_conn();

    //get connection info,save in gap_info
    STREAM_TO_UINT16(gap_info.Connection_Handle, p);
    STREAM_TO_UINT8(gap_info.Role, p);
    STREAM_TO_UINT8(gap_info.Peer_Address_Type, p);
    STREAM_TO_BDADDR(gap_info.Peer_Address, p);
    STREAM_TO_UINT16(gap_info.Conn_Interval, p);
    STREAM_TO_UINT16(gap_info.Conn_Latency, p);
    STREAM_TO_UINT16(gap_info.Supervision_Timeout, p);


    mxd_printf("[GAP]conn state role=0x%x handle=0x%x, interval=0x%x latency=0x%x\n",
               gap_info.Role, gap_info.Connection_Handle,gap_info.Conn_Interval,gap_info.Conn_Latency);

    //if (gap_info.conn_CB)
    //    gap_info.conn_CB(TRUE,gap_info.Peer_Address);

    //gatt_LL_conn_cb(gap_info.Peer_Address);
    //l2cap_LL_conn_cb(gap_info.Connection_Handle);
}
void gap_disconn_cmplt_CB(u8* data, u8 len)
{
    gap_set_state_idle();
}
void gap_set_buffer_size(u16 HC_ACL_Data_Packet_Length,u16 HC_Total_Num_Data_Packets)
{
    ;
}

void gap_set_state_adv(void)
{
    gap_info.state = GAP_STATE_ADV;
}
void gap_set_state_scan(void)
{
    gap_info.state = GAP_STATE_SCAN;
}
void gap_set_state_init(void)
{
    gap_info.state = GAP_STATE_INIT;
}
void gap_set_state_conn(void)
{
    gap_info.state = GAP_STATE_CONN;
}
void gap_set_state_idle(void)
{
    gap_info.state = GAP_STATE_IDLE;
}
int gap_get_state(void)
{
    return gap_info.state;
}

void gap_set_while_list_size(u8 size)
{
    ;
}
void gap_set_rssi(u16 hdl,u8 rssi)
{
    ;
}


//l2cap.c
void l2cap_data_input(u8* data, u16 len)
{
    ;
}
void l2cap_data_output(u16 cid, u8* data, u16 len)
{
    u8* p=data;
    //u16 connhdl;
    //u16 l2c_len;
    //u16 acl_len;


    UINT8_TO_STREAM(p,0x02);
    UINT16_TO_STREAM(p, gap_info.Connection_Handle);
    UINT16_TO_STREAM(p,len+4);
    UINT16_TO_STREAM(p,len);
    UINT16_TO_STREAM(p,cid);

    hci_api_acl_send(data,len+9);
}

//system
void bt_sleep(int ms)
{
    usleep(ms*1000);
}

//
void mxd_set_cmpt_pkt(void)
{
    ;
}

