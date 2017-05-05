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
#ifndef _BLE_GAP_H_
#define _BLE_GAP_H_

#include "mxd_type.h"

typedef struct
{
    uint8_t state;

    uint16_t Connection_Handle;
    uint8_t  Role;
    uint8_t  Peer_Address_Type;
    BD_ADDR  Peer_Address;
    uint16_t Conn_Interval;
    uint16_t Conn_Latency;
    uint16_t Supervision_Timeout;

    //int HC_ACL_Data_Packet_Length;
    //int HC_Total_Num_Data_Packets;
    //uint8_t while_list_size;
    //gap_scan_reportCB  scan_CB;
    //gap_conn_cmpt_reportCB  conn_CB;
    //gap_disconn_cmpt_reportCB  disconn_CB;
    int  rssi;
    BD_ADDR bda_public;
    BD_ADDR bda_random;
} ble_gap_info_t;


enum
{
    GAP_STATE_IDLE=0,
    GAP_STATE_ADV=1,
    GAP_STATE_SCAN=2,
    GAP_STATE_INIT=3,
    GAP_STATE_CONN=4,
} gap_state_t;

//mesh
int hci_le_adv_report_bearer_cb(uint8_t * data, uint8_t len);

//gap.c
int gap_init(void);
void gap_scan_adv_report_CB(u8* data, u8 len);
void gap_conn_cmplt_CB(u8* data, u8 len);
void gap_disconn_cmplt_CB(u8* data, u8 len);
void gap_set_buffer_size(u16 HC_ACL_Data_Packet_Length,u16 HC_Total_Num_Data_Packets);
void gap_set_bd_addr(BD_ADDR bda);
void gap_set_random_bd_addr(BD_ADDR bda);

void gap_set_state_adv(void);
void gap_set_state_scan(void);
void gap_set_state_init(void);
void gap_set_state_conn(void);
void gap_set_state_idle(void);
int gap_get_state(void);

void gap_set_while_list_size(u8 size);
void gap_set_rssi(u16 hdl,u8 rssi);


//l2cap.c
void l2cap_data_input(u8* data, u16 len);
void l2cap_data_output(u16 cid, u8* data, u16 len);


//system
void bt_sleep(int ms);

//
void mxd_set_cmpt_pkt(void);

#endif
