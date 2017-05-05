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
#include <stdlib.h>
#include <string.h>

#include "mxd_type.h"
#include "ble_trans_adaptor.h"
#include "ble_hcidefs.h"
#include "ble_gap.h"
#include "ble_hci_adaptor.h"

/*****************************************************
* Extern Function Declare
*****************************************************/


/*****************************************************
* Pre-processor Define
*****************************************************/
#define HCI_TRACE_DEBUG0 printf
#define HCI_TRACE_DEBUG1 printf
#define HCI_TRACE_DEBUG2 printf
#define HCI_TRACE_DEBUG3 printf
#define HCI_TRACE_DEBUG4 printf
#define HCI_TRACE_DEBUG5 printf
#define mxd_printf printf


#define HCI_RESET_CMD               0x0c03
#define HCI_LE_SET_ADV_PAR_CMD      0x2006
#define HCI_LE_SET_ADV_DATA_CMD     0x2008
#define HCI_LE_SET_ADV_EN_CMD       0x200A

#define HCI_DISCONN_CMPLT_EVT       0x05
#define HCI_ENCRYPTION_CHG_EVT       0x08
#define HCI_ENCRYPTION_KEY_REFRESH_EVT       0x30
#define HCI_CMD_CMPLT_EVT           0xE
#define HCI_CMD_STATUS_EVT          0xF
#define HCI_NUM_OF_CMPLT_PKTS_EVT   0x13
#define HCI_READ_RMT_VERSION_EVT              0x0c
#define HCI_HW_ERR_EVT              0x10
#define HCI_LE_META_EVT             0x3E

#define HCI_LE_CONN_CMPLT_EVT                   0x01
#define HCI_LE_ADV_REPORT_EVT                   0x02
#define HCI_LE_CONN_UPDATE_EVT                  0x03
#define HCI_LE_READ_REMOTE_FEATURES_CMPLT_EVT   0x04
#define HCI_LE_LONG_TERM_KEY_REQ_EVT            0x05
#define HCI_LE_CON_PARA_REQ_EVT                 0x06


/*****************************************************
* Type Define
*****************************************************/
static struct
{
    const bt_transport_t *transport;
    const u8* curr_reset_cmd;
    u8* acl_in_data;
    u16 acl_in_len;
    u16 hconn; // connection handle
    u8 ncmds; // number of commands that can be sent
    u16 wait_cmd_option;
    u8 wait_for_cmd_complt_evt;

} hci;

/*****************************************************
* Private Function Declare
*****************************************************/
static void hci_transport_event_handler(u8 channel, u8* buffer, u16 size);

static void hci_event_received(u8* data, u16 len);
static void hci_disconn_complt(u8* data, u8 len);
static void hci_cmd_cmplt(u8* data, u8 len);
static void hci_cmd_status(u8* data, u8 len);
static void hci_num_of_cmplt_pkts(u8* data, u8 len);
static void hci_read_rmt_version_evt(u8* data, u8 len);

//static void hci_hw_err(u8* data, u8 len);
static void hci_le_conn_cmplt(u8* data, u8 len);
static void hci_le_adv_report(u8* data, u8 len);
static void hci_le_conn_update(u8* data, u8 len);
static void hci_le_read_rmt_feature(u8* data, u8 len);
static void hci_le_long_term_key_req(u8* data, u8 len);
static void hci_le_con_para_req(u8* data, u8 len);

static void hci_encryption_chg_evt(u8* data, u8 len);
static void hci_encryption_key_refresh_evt(u8* data, u8 len);
static void hci_acl_received(u8* data, u16 len);
#if 0
//
// HCI Reset Sequence
static const u8 hci_reset_seq[] =
{
    0x03, 0x0C, 0x00, // reset
    0x03, 0x10, 0x00, // read local supported features
    0x01, 0x0C, 0x08, 0xFF, 0xFF, 0xFB, 0xFF, 0x07, 0xF8, 0xBF, 0x3D, // set event mask
    0x01, 0x20, 0x08, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // le set event mask

    0x02, 0x20, 0x00, // le read buffer size
    0x05, 0x10, 0x00, // read buffer size
    0x03, 0x20, 0x00, // le read local supported features

    0x09, 0x10, 0x00, // read BD ADDR
    0,0,0

    0x14, 0x0C, 0x00, // read local name

    0x07, 0x20, 0x00, // le read advertising channel tx power
    0x0F, 0x20, 0x00, // le read white list size
    0x1C, 0x20, 0x00, // le read supported states

    0x56, 0x0C, 0x01, 0x01, // write simple pairing mode (1)
    0x45, 0x0C, 0x01, 0x02, // write inquiry mode (2)
    0x58, 0x0C, 0x00, // read inquiry response tx power
    0x04, 0x10, 0x01, 0x01, // read local extended features (page 1)
    0x12, 0x0C, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // delete stored key
    0x0F, 0x08, 0x02, 0x0F, 0x00, // write default link policy settings
    0x6D, 0x0C, 0x02, 0x01, 0x01, // write le host supported (1)
    0,0,0 // end of sequence

};
#endif

static void bt_dumphex(char* msg, const u8* v, u16 len)
{
    int i;

    //mxd_printf("[%d]%s: [ ", clock(),msg);
    mxd_printf("%s: [ ",  msg);
    for (i=0; i<len; i++) mxd_printf("%02x ", v[i]);
    mxd_printf("]\n");
}

static void hci_transport_event_handler(u8 channel, u8* buffer, u16 size)
{
    switch (channel)
    {

    case BT_EVENT_CHANNEL:
    {
        u8 event_code = buffer[0];
        if (hci.wait_for_cmd_complt_evt && HCI_CMD_CMPLT_EVT == event_code )
        {
            u16 option= 0;
            u8 *p=buffer+3;
            STREAM_TO_UINT16(option,p);
            if (hci.wait_cmd_option == option)
            {
                hci.wait_for_cmd_complt_evt=0;
                hci.wait_cmd_option =0;
            }
        }
        else if(hci.wait_for_cmd_complt_evt && HCI_CMD_STATUS_EVT == event_code)
        {
            u16 option= 0;
            u8 *p=buffer+4;
            STREAM_TO_UINT16(option,p);
            if (hci.wait_cmd_option == option)
            {
                hci.wait_for_cmd_complt_evt=0;
                hci.wait_cmd_option =0;
            }
        }

        hci_event_received(buffer, size);
    }
    break;
    case BT_ACL_IN_CHANNEL:
        hci_acl_received(buffer, size);
        break;

    default:
        break;
    }
}

static void hci_event_received(u8* data, u16 len)
{
    u8 event_code;
    u8 param_len;
    u8 subcode;
    u8 *param;
    //u8 status;
    /*
      event format
      0: event code
      1: parameter total length
      *: parameters
      */

    event_code = data[0];
    param_len = data[1];
    param = data + 2;

    //HCI_TRACE_DEBUG0("event received code=0x%x, len=0x%x\n", event_code, param_len);
    bt_dumphex("EVT<---", data, len);

    switch (event_code)
    {
    case HCI_DISCONN_CMPLT_EVT:
        hci_disconn_complt(param, param_len);
        break;
    case HCI_CMD_CMPLT_EVT:
        hci_cmd_cmplt(param, param_len);
        break;
    case HCI_CMD_STATUS_EVT:
        hci_cmd_status(param, param_len);
        break;
    case HCI_NUM_OF_CMPLT_PKTS_EVT:
        hci_num_of_cmplt_pkts(param, param_len);
        break;
    case  HCI_READ_RMT_VERSION_EVT :
        hci_read_rmt_version_evt(param, param_len);
        break;
    case HCI_ENCRYPTION_CHG_EVT:
        hci_encryption_chg_evt(param, param_len);
        break;
    case  HCI_ENCRYPTION_KEY_REFRESH_EVT :
        hci_encryption_key_refresh_evt(param, param_len);
        break;


    case HCI_HW_ERR_EVT:
        break;
    case HCI_LE_META_EVT:
        subcode = data[2];
        {
            param_len -= 1;
            param += 1;
            switch (subcode)
            {
            case HCI_LE_CONN_CMPLT_EVT:
                hci_le_conn_cmplt(param, param_len);
                break;
            case HCI_LE_ADV_REPORT_EVT:
                hci_le_adv_report(param, param_len);
                break;
            case HCI_LE_CONN_UPDATE_EVT:
                hci_le_conn_update(param, param_len);
                break;
            case HCI_LE_READ_REMOTE_FEATURES_CMPLT_EVT:
                hci_le_read_rmt_feature(param, param_len);
                break;
            case HCI_LE_LONG_TERM_KEY_REQ_EVT:
                hci_le_long_term_key_req(param, param_len);
                break;
            case HCI_LE_CON_PARA_REQ_EVT:
                hci_le_con_para_req(param, param_len);
                break;
            }
        }
        break;
    case 0xFF:
        break;
    }
}

static void hci_disconn_complt(u8* data, u8 len)
{
    HCI_TRACE_DEBUG0("disconnected\n");

    hci.hconn = 0;
    gap_disconn_cmplt_CB(data,   len);
    //  hci.tasks.set_adv_en = 1;
    //  hci.transport->request_to_write(BT_COMMAND_CHANNEL);
}


static void hci_cmd_status(u8* data, u8 len)
{
    hci.ncmds = data[1];

    HCI_TRACE_DEBUG1("HCI Command Status Event %d\n", hci.ncmds);

    //  btsnd_hcic_ble_set_adv_enable(1);;
}

static void hci_read_rmt_version_evt(u8* data, u8 len)
{
    u8    Status;
    u16 Con_Handle;
    u8 Version;
    u16 CompId=0;
    u16 Subversion;
    u8* pp = data;
    if (8!=len)
    {
        HCI_TRACE_DEBUG1("hci_read_rmt_version_evt errlen=%d", len);
        return;
    }
    STREAM_TO_UINT8(Status,pp);
    STREAM_TO_UINT16(Con_Handle, pp);
    STREAM_TO_UINT8(Version, pp);
    STREAM_TO_UINT16(CompId, pp);
    STREAM_TO_UINT16(Subversion, pp);
    HCI_TRACE_DEBUG4("hci_read_rmt_version_evt status=%d, hdl=%x,Ver=%x,CompId=%x,SubVer=%x",Status, Con_Handle,Version,CompId,Subversion);
    if(0x03B4>=CompId)
    {
        //printf("\t the manufacturer is %s \n",companyID[CompId]);
    }
    else
    {
        printf("\n Unknown manufacturer\n");
    }
}
static void hci_num_of_cmplt_pkts(u8* data, u8 len)
{
    mxd_set_cmpt_pkt();
}
//static void hci_hw_err(u8* data, u8 len)
//{
//}
static void hci_encryption_chg_evt(u8* data, u8 len)
{
    u8    Status;
    u16 Con_Handle;
    u8 Encryp_enable;

    u8* pp = data;
    if (4>len)
    {
        HCI_TRACE_DEBUG1("hci_encryption_chg_evt errlen=%d", len);
        return;
    }
    STREAM_TO_UINT8(Status,pp);
    STREAM_TO_UINT16(Con_Handle, pp);
    STREAM_TO_UINT8(Encryp_enable, pp);

    {
        HCI_TRACE_DEBUG2("hci_encryption_chg_evt  [Con_Handle=%d Status=0x%x Encryp_enable=%x]",Con_Handle, Status, Encryp_enable);
        //if ((Status == 0)&&(Encryp_enable==1))
        //    mxd_sec_end_wait(1);
    }

}
static void hci_encryption_key_refresh_evt(u8* data, u8 len)
{
    u8    Status;

    u8* pp = data;

    STREAM_TO_UINT8(Status,pp);
    HCI_TRACE_DEBUG1("hci_encryption_key_refresh_evt  status=%d", Status);
}

static void hci_acl_received(u8* data, u16 len)
{
    u8 pb_flag;

    bt_dumphex("ACL<---", data, len);
    pb_flag = (data[1]&0x30)>>4;
    //HCI_TRACE_DEBUG1("******ACLIN-PB=%d", pb_flag);

    /* acl format
           bits 0-11   handle
           bits 12-13  pb flag
           bits 14-15  bc flag
           bits 16-31  data total length
        */

    l2cap_data_input(data,   len);

    return;
}

void hci_adaptor_init(const bt_transport_t *_transport,
                      u8 initial_ncmds)
{
    memset(&hci, 0, sizeof(hci));
    hci.transport = _transport;
    hci.ncmds = initial_ncmds;
}

static int hci_adaptor_open()
{
    // hci.curr_reset_cmd = hci_reset_seq;
    return hci.transport->open(hci_transport_event_handler);
}

static void hci_adaptor_close()
{
    hci.transport->close();
}

int hci_setup(void)
{
    int ret;
    
    const bt_transport_t *trans = get_ble_trans_adaptor();
    hci_adaptor_init(trans, 1);
    
    ret = hci_adaptor_open();
    return ret;
}

void hci_down(void)
{
    hci_adaptor_close();
}

void hci_api_cmd_send(u8 *buffer,u16 size)
{
    hci.transport->data_send(BT_COMMAND_CHANNEL,buffer,size);
}

void hci_data_send(u8 *buffer,u16 size)
{
    hci.transport->data_send(BT_ACL_OUT_CHANNEL,buffer,size);
}

void hci_api_acl_send(u8 *buffer,u16 size)
{
    bt_dumphex("ACL--->", buffer, size);
    hci.transport->data_send(BT_ACL_OUT_CHANNEL,buffer,size);//hci_data_send(buffer,size);
}

void hci_api_cmd_send_and_wait_compt(u8 *buffer,u16 size,u16 timeout)
{
    u8 timecnt=0;
    u8 *p=buffer;
    bt_dumphex("CMD--->", buffer, size);
    hci.transport->data_send(BT_COMMAND_CHANNEL,buffer,size);
    hci.wait_for_cmd_complt_evt = 1;
    STREAM_TO_UINT16(hci.wait_cmd_option, p);
    //= bt_read_u16(, buffer[1]);

    while (hci.wait_for_cmd_complt_evt && timecnt*100<timeout)
    {
        bt_sleep(100);
        timecnt++;
    };
    if (timecnt*100>=timeout)
        HCI_TRACE_DEBUG1("send cmd timeout  %x\n",hci.wait_cmd_option);
    hci.wait_for_cmd_complt_evt = 0;
    hci.wait_cmd_option=0;

}

static void hci_cmd_cmplt(u8* data, u8 len)
{
    u16 cmd_code=0;
    u8 status=0;
    u8* pp = data;
    STREAM_TO_UINT8(hci.ncmds,pp);
    STREAM_TO_UINT16(cmd_code, pp);
    STREAM_TO_UINT8(status, pp);

    if (status)
    {
        //HCI_TRACE_DEBUG1("HCI cmd cmplt Evente error[%x]\n",status);
        HCI_TRACE_DEBUG1("HCI cmd cmplt Event[%.2x][%x],Error\n",cmd_code,status);
        return;
    }

    HCI_TRACE_DEBUG2("HCI cmd cmplt Event[%.2x][%x]\n",cmd_code,status);

    switch (cmd_code)
    {
    case HCI_BLE_RESET:
        HCI_TRACE_DEBUG0("chip reset ok\n");
        break;
    case HCI_READ_LOCAL_FEATURES:
        HCI_TRACE_DEBUG0("read local features ok\n");
        bt_dumphex("local features", pp,8);
        break;
    case HCI_SET_EVENT_MASK:
        HCI_TRACE_DEBUG0("set evt mask ok\n");
        break;
    case HCI_BLE_SET_EVENT_MASK:
        HCI_TRACE_DEBUG0("set le evt mask ok\n");
        break;
    case HCI_BLE_READ_BUFFER_SIZE:
    {
        u16 HC_LE_ACL_Data_Packet_Length=0;
        u8 HC_Total_Num_LE_Data_Packets=0;
        STREAM_TO_UINT16(HC_LE_ACL_Data_Packet_Length, pp);
        STREAM_TO_UINT8(HC_Total_Num_LE_Data_Packets, pp);
        gap_set_buffer_size(HC_LE_ACL_Data_Packet_Length,HC_Total_Num_LE_Data_Packets);
        HCI_TRACE_DEBUG2("read  le buffer size ok,HC_Total_Num_LE_Data_Packets=%x,HC_Total_Num_LE_Data_Packets=%x\n",HC_LE_ACL_Data_Packet_Length,HC_Total_Num_LE_Data_Packets);

    }
    break;

    case HCI_READ_BUFFER_SIZE:
    {
        u16 HC_ACL_Data_Packet_Length=0;
        u16 HC_Total_Num_Data_Packets=0;
        STREAM_TO_UINT16(HC_ACL_Data_Packet_Length, pp);
        pp++;
        STREAM_TO_UINT16(HC_Total_Num_Data_Packets, pp);
        gap_set_buffer_size(HC_ACL_Data_Packet_Length,HC_Total_Num_Data_Packets);
        HCI_TRACE_DEBUG2("read  buffer size ok,HC_ACL_Data_Packet_Length=%x,HC_Total_Num_Data_Packets=%x\n",HC_ACL_Data_Packet_Length,HC_Total_Num_Data_Packets);

    }
    break;
    case HCI_BLE_READ_LOCAL_SPT_FEAT:
        HCI_TRACE_DEBUG0("read le local spt feat ok\n");
        bt_dumphex("le local features", pp,8);
        break;
    case HCI_READ_BD_ADDR:
    {
        BD_ADDR bda;
        STREAM_TO_BDADDR(bda,pp);
        HCI_TRACE_DEBUG0("read address ok\n");
        bt_dumphex("bd_addr", bda,6);
        gap_set_bd_addr(bda);
    }
    break;
    case HCI_BLE_WRITE_ADV_ENABLE:
        HCI_TRACE_DEBUG0("adv enable ok\n");
        gap_set_state_adv();
        break;
    case HCI_BLE_WRITE_SCAN_ENABLE:
        HCI_TRACE_DEBUG0("scan enable ok\n");
        gap_set_state_scan();
        break;
    case HCI_BLE_CREATE_LL_CONN:
        HCI_TRACE_DEBUG0(" BLE_CREATE_LL_CONN ok\n");
        gap_set_state_init();
        break;
    case HCI_BLE_READ_WHITE_LIST_SIZE:
    {
        u8 size;
        HCI_TRACE_DEBUG0(" HCI_BLE_READ_WHITE_LIST_SIZE ok\n");
        STREAM_TO_UINT8(size, pp);
        gap_set_while_list_size(size);
    }
    break;
    case HCI_READ_RSSI:
    {
        u16 hdl;
        u8 rssi;
        HCI_TRACE_DEBUG0(" HCI_READ_RSSI ok\n");

        STREAM_TO_UINT16(hdl, pp);
        STREAM_TO_UINT8(rssi, pp);
        gap_set_rssi(hdl,rssi);
    }
    break;
    case HCI_BLE_TEST_END:
    {
        u16 pkt_count=0;
        STREAM_TO_UINT16(pkt_count, pp);
        if (12 == len)
        {
            u32 packetdata1;               //HC_ACL_Data_Packet_Length     2
            u16 packet_crcerror_count;               //HC_ACL_Data_Packet_Length     2
            STREAM_TO_UINT32(packetdata1, pp);
            STREAM_TO_UINT16(packet_crcerror_count, pp);
            HCI_TRACE_DEBUG4(" ----------DTM packet receive ok== %d--crcerror==%d(total=%d)--data=0x%x\n",pkt_count,packet_crcerror_count,pkt_count+packet_crcerror_count,packetdata1);
        }
        else
        {
            HCI_TRACE_DEBUG2(" ----------DTM packet receive 0x%x = %d\n",pkt_count,pkt_count);
        }

    }
    break;
    case HCI_MXD_READ_REG:
    {
        u8 i,rf_type=0;
        u8 count;
        STREAM_TO_UINT8(rf_type, pp);
        STREAM_TO_UINT8(count, pp);
        if (0==rf_type)
            HCI_TRACE_DEBUG1(" ----------HCI_MXD_READ_REG %d RF reg\n",count);
        if (1==rf_type)
            HCI_TRACE_DEBUG1(" ----------HCI_MXD_READ_REG %d modem reg\n",count);
        for (i=0; i<count; i++)
        {
            u32 val=0;
            STREAM_TO_UINT32(val, pp);
            mxd_printf(" ---------reg value 0x%x\n",val);

        }


    }
    break;
    case HCI_MXD_WRITE_REG:
    {

        HCI_TRACE_DEBUG0(" ---------HCI_MXD_WRITE_REG  ok\n");

    }
    break;

    default:
        break;
    }
}

static void hci_le_conn_cmplt(u8* data, u8 len)
{
    // = bt_read_u16(data[1], data[2]);
    u8 *p=data+1;
    STREAM_TO_UINT16(hci.hconn , p);
    gap_conn_cmplt_CB(data,   len);

}

static void hci_le_adv_report(u8* data, u8 len)
{
    if(hci_le_adv_report_bearer_cb(data, len) != 0) return;

    gap_scan_adv_report_CB(data,   len);
}
static void hci_le_read_rmt_feature(u8* data, u8 len)
{
    u8 status=0;
    int i=0;
    u16 Connection_Handle=0;
    u8 feature[8];
    u8 *p=data;

    if (len==11)
    {
        STREAM_TO_UINT8(status , p);
        if (status!=0)
        {
            HCI_TRACE_DEBUG1(" --LE read remote feature-l-error- status =0x%x\n",status);
            return;
        }
        STREAM_TO_UINT16(Connection_Handle , p);
        mxd_printf("-LE read remote feature- handle=%d:", Connection_Handle);
        for (i=0; i<8; i++) mxd_printf("%02x ", p[i]);
        mxd_printf("]\n");
        STREAM_TO_ARRAY8(feature , p);



    }
    else
    {
        HCI_TRACE_DEBUG0(" -LE read remote feature-len--error");

    }
}
static void hci_le_conn_update(u8* data, u8 len)
{
    u8 status=0;
    u16 Connection_Handle=0;
    u16 Conn_Interval=0;
    u16 Conn_Latency=0;
    u16 Supervision_Timeout=0;
    u8 *p=data;//EVT: [ 3e 0a 03 00 46 00 56 00 00 00 c8 00 ]

    if (len==8+1)
    {
        STREAM_TO_UINT8(status , p);
        if (status!=0)
        {
            HCI_TRACE_DEBUG1(" -LE Con Update Complete-error- status =0x%x\n",status);
            return;
        }
        STREAM_TO_UINT16(Connection_Handle , p);
        STREAM_TO_UINT16(Conn_Interval , p);
        STREAM_TO_UINT16(Conn_Latency , p);
        STREAM_TO_UINT16(Supervision_Timeout , p);

        HCI_TRACE_DEBUG4(" -LE Con Update Complete-Con_Handle=0x%x ,Conn_Interval=0x%x,Conn_Latency=0x%x,Supervision_Timeout=0x%x\n",Connection_Handle,Conn_Interval,Conn_Latency,Supervision_Timeout);


    }
    else
    {
        HCI_TRACE_DEBUG0(" -LE Con Update Complete-len--error");

    }
}

//extern int test_LL_on;
static void hci_le_long_term_key_req(u8* data, u8 len)
{
#if 0
    u8* p=data;
    u8 code=0;
    u16 hdl=0;
    u8 rand[8];
    u16 ediv=0;
    bt_dumphex("-LE ltk req-", p,len);
    // STREAM_TO_UINT8(code , p);
    STREAM_TO_UINT16(hdl , p);
    STREAM_TO_ARRAY(rand , p,8);
    STREAM_TO_UINT16(ediv , p);
    HCI_TRACE_DEBUG1(" -LE ltk req--ediv=0x%x",ediv);
    if (test_LL_on==1)
    {
        mxd_sec_end_wait(0);
    }
    else
    {
        mxd_send_ltk_reply();
    }
#endif
    return;
}

//extern int test_con_reply;
static void hci_le_con_para_req(u8* data, u8 len)
{
#if 0
    u8* p=data;
    u8 subcode=0;
    u16 hdl=0;
    u16 minv,maxv,lat,tout;
    bt_dumphex("-LE Remote Connection-", p,len);
    STREAM_TO_UINT8(subcode , p);
    STREAM_TO_UINT16(hdl , p);
    STREAM_TO_UINT16(minv , p);
    STREAM_TO_UINT16(maxv , p);
    STREAM_TO_UINT16(lat , p);
    STREAM_TO_UINT16(tout , p);
    HCI_TRACE_DEBUG5(" -LE Remote Connection-0x%x-0x%x-0x%x-0x%x-0x%x",hdl,minv,maxv,lat,tout);

    if (0==test_con_reply)
    {
        btsnd_hcic_ble_con_para_req_negative_reply(hdl,0x3b);
    }
    else
    {
        btsnd_hcic_ble_con_para_req_reply(hdl,
                                          minv,  maxv,
                                          lat, tout,
                                          0xffff,0xffff);
    }
#endif
    return;
}


