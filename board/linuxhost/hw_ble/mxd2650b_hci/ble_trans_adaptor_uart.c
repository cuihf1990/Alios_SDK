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
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "ble_trans_adaptor.h"
#include "uart_trans.h"

/*****************************************************
* Pre-processor Define
*****************************************************/
#define HCI_TRACE_DEBUG0 printf
#define HCI_TRACE_DEBUG1 printf
#define mxd_printf printf

/*****************************************************
* Private Data Define
*****************************************************/
#define CMD_BUFFER_SIZE 80
static u8 cmd_buffer[CMD_BUFFER_SIZE ];
//#define EVT_BUFFER_SIZE 255
//static u8 evt_buffer[CMD_BUFFER_SIZE];
#define ACL_BUFFER_SIZE 566
static u8 acl_in_buffer[ACL_BUFFER_SIZE];
static u8 acl_out_buffer[ACL_BUFFER_SIZE];
static u8 acl_in_buff_size=0;
static u8 acl_in_buff_size_want=0;
static u8 dat_in_size=0;

static u8 cmd_completed, acl_out_completed;

static bt_transport_listener_t libusb_transfer_listener;

#define MXD_UB_SZ 1124
static u8 mxd_ubuff[MXD_UB_SZ];
static u8 mxd_ubuff_t[MXD_UB_SZ];
static u16 mxd_ubuff_sz=0;

/*****************************************************
* Private Function Define
*****************************************************/
static void uart_data_cb(uint8_t * pData, int uLen)
{
    int i,len=0;
    u8 *pbuff;
    if (uLen == 0)
    {
        HCI_TRACE_DEBUG0("uart no data \n");
        return;
    }
    //if(get_output_log())
    //{
    //    printf("%s",(char*)pData);
    //    return;
    //}
    //if (hci_get_debug_on())
    //{
    //   HCI_TRACE_DEBUG1("Uart_data_cb uLen=%d---\n",uLen);
    //   bt_dumphex("uartD", pData, uLen);
    //}

    if (uLen+mxd_ubuff_sz>MXD_UB_SZ)
    {
        HCI_TRACE_DEBUG0("error in buff overflow\n");
        return;
    }
    memcpy(mxd_ubuff+mxd_ubuff_sz,pData,uLen);
    mxd_ubuff_sz+=uLen;
RETEL:
    pbuff=mxd_ubuff;
    if (*pbuff == 4)    //evt data  1: CMD, 2:ACL, 3:SCO 4:Event
    {
        if (mxd_ubuff_sz <= 3)
        {
            HCI_TRACE_DEBUG0("evt buff not complete\n");
            return ;
        }
        if (pbuff[1]==0x3e && pbuff[2]==1)
        {
            memcpy(mxd_ubuff_t,mxd_ubuff,MXD_UB_SZ);
            memcpy(mxd_ubuff+3,mxd_ubuff_t+2,MXD_UB_SZ-(mxd_ubuff_sz-2));
            pbuff[2]=0x13;
            mxd_ubuff_sz+=1;


        }

        len=pbuff[2];
        if (mxd_ubuff_sz>=len+3)
        {
            libusb_transfer_listener(BT_EVENT_CHANNEL, pbuff+1, len+2);
            mxd_ubuff_sz -= len+3;
            memcpy(mxd_ubuff_t,mxd_ubuff,MXD_UB_SZ);
            memcpy(mxd_ubuff,mxd_ubuff_t+len+3,MXD_UB_SZ-len-3);
        }
        else
        {
            HCI_TRACE_DEBUG0("evt buff not complete\n");
            return ;


        }

    }
    else if (*pbuff == 2)    //acl data
    {
        if (mxd_ubuff_sz <= 5)
        {
            HCI_TRACE_DEBUG0("acl buff not complete\n");
            return ;


        }
        len=pbuff[3]+((unsigned short)pbuff[4]<<8);
        if (mxd_ubuff_sz >= len+5)
        {
            libusb_transfer_listener(BT_ACL_IN_CHANNEL, pbuff+1, len+4);
            mxd_ubuff_sz -= len+5;
            memcpy(mxd_ubuff_t,mxd_ubuff,MXD_UB_SZ);
            memcpy(mxd_ubuff,mxd_ubuff_t+len+5,MXD_UB_SZ-len-5);
        }
        else
        {
            HCI_TRACE_DEBUG0("acl buff not complete\n");
            return ;


        }
    }
    else if (*pbuff == 0x5a)    //debug data
    {
        if (mxd_ubuff_sz <= 2)
        {
            HCI_TRACE_DEBUG0("debug buff not complete\n");
            return ;
        }
        len=pbuff[1] +2;
        if (mxd_ubuff_sz >= len)
        {
            u8 tmpbuf[256];
            memset(tmpbuf,0,sizeof(tmpbuf));
            memcpy(tmpbuf,pbuff+2,len-2);
            mxd_printf("<><><>[debug]:%s\n",tmpbuf);
            mxd_ubuff_sz -= len;
            memcpy(mxd_ubuff_t,mxd_ubuff,MXD_UB_SZ);
            memcpy(mxd_ubuff,mxd_ubuff_t+len,MXD_UB_SZ-len);
        }
        else
        {
            HCI_TRACE_DEBUG0("debug buff not complete\n");
            return ;
        }
    }
    else if (*pbuff == 0x57)    //read phy return
    {
        int dlen8bit=0;
        if (mxd_ubuff_sz <  12)
        {
            HCI_TRACE_DEBUG0("read phy return buff not complete\n");
            return ;
        }
        dlen8bit=pbuff[1] ;
        len=dlen8bit*4 + 8;
        if (mxd_ubuff_sz >= len)
        {
            u8* pbb=pbuff+4;
            u8 tmpbuf[256];
            u32 addr=0;
            memset(tmpbuf,0,sizeof(tmpbuf));
            memcpy(tmpbuf,pbuff+4,len-4);

            STREAM_TO_UINT32(addr, pbb);
            //mxd_reg_read_save_file(pbb,  addr,dlen8bit);
            mxd_printf("[read phy return[start addr:0x%x--len-0x%x]]:\n",addr,dlen8bit);
            for (i=0; i<dlen8bit; i++)
            {
                STREAM_TO_UINT32(addr, pbb);
                mxd_printf("%08x ", addr);
                if ((i & 3) ==3)  mxd_printf("\n");
            }
            mxd_printf("]\n");
            mxd_ubuff_sz -= len;
            memcpy(mxd_ubuff_t,mxd_ubuff,MXD_UB_SZ);
            memcpy(mxd_ubuff,mxd_ubuff_t+len,MXD_UB_SZ-len);
        }
        else
        {
            HCI_TRACE_DEBUG0("debug buff not complete\n");
            return ;


        }
    }
    else if (*pbuff == 0x55)    //read phy return
    {
        int dlen8bit=0;
        if (mxd_ubuff_sz <  8)
        {
            HCI_TRACE_DEBUG0("w phy   0x55 not complete\n");
            return ;
        }
        dlen8bit=pbuff[1] ;
        len=dlen8bit*4 + 8;
        if (mxd_ubuff_sz >= len)
        {
            mxd_ubuff_sz -= len;
            memcpy(mxd_ubuff_t,mxd_ubuff,MXD_UB_SZ);
            memcpy(mxd_ubuff,mxd_ubuff_t+len,MXD_UB_SZ-len);
        }
        else
        {
            HCI_TRACE_DEBUG0("0x55 buff not complete\n");
            return ;


        }
    }
#if 0
    else if (*pbuff == 0x22)    //apl data
    {
        if (mxd_ubuff_sz <= 2)
        {
            HCI_TRACE_DEBUG0("api buff not complete\n");
            return ;
        }
        len=pbuff[1] +2;
        if (mxd_ubuff_sz >= len)
        {
            u8 tmpbuf[256];
            memset(tmpbuf,0,sizeof(tmpbuf));
            memcpy(tmpbuf,pbuff+2,len-2);
            mxd_printf("<><><>[api data]len:%d\n",len);
            mxd_ubuff_sz -= len;
            memcpy(mxd_ubuff_t,mxd_ubuff,MXD_UB_SZ);
            memcpy(mxd_ubuff,mxd_ubuff_t+len,MXD_UB_SZ-len);
            if (mxd_recv_apl_flag>0)
            {
                mxd_apl_save_file(len-2,  tmpbuf);
            }
            if (mxd_wait_loop_packet_apl_flag>0)
            {
                int sz=len-2;
                if (sz>sizeof(mxd_loop_pkt))
                {
                    sz=sizeof(mxd_loop_pkt);
                }
                memcpy(mxd_loop_pkt, tmpbuf,sz);
                mxd_wait_loop_packet_apl_flag=0;
            }

        }
        else
        {
            HCI_TRACE_DEBUG0("api data buff not complete\n");
            return ;


        }
    }
    else if (*pbuff == 0x26)    //apl data
    {
        if (mxd_ubuff_sz <= 2)
        {
            HCI_TRACE_DEBUG0("api buff not complete\n");
            return ;
        }
        len=3;
        if (mxd_ubuff_sz >= len)
        {
            if (pbuff[1]==0x22)
            {
                mxd_send_apl_flag=pbuff[2];
            }
            mxd_ubuff_sz -= len;
            memcpy(mxd_ubuff_t,mxd_ubuff,MXD_UB_SZ);
            memcpy(mxd_ubuff,mxd_ubuff_t+len,MXD_UB_SZ-len);

        }
        else
        {
            HCI_TRACE_DEBUG0("api retrurn buff not complete\n");
            return ;


        }
    }
#endif
    else
    {
        mxd_ubuff_sz=0;
#if 0
        {
            int i;

            if (!hci_get_debug_on())
            {

                mxd_printf("[%d]uartD:%d [ ", clock(),uLen);
                for (i=0; i<uLen; i++)
                {

                    mxd_printf("%02x ", pData[i]);
                    if ((i & 7) ==7)  mxd_printf("\n");
                }
                mxd_printf("]\n");
            }
        }
#endif
        mxd_printf("error data=u========\n");
        return ;
    }
    if (mxd_ubuff_sz>3)
        goto RETEL;
}

static int uart_transport_open(bt_transport_listener_t listener)
{
    int ret;

    ret = uart_trans_init(uart_data_cb);

    if (ret < 0)
    {
        HCI_TRACE_DEBUG0("Cannot open Bluetooth device\n");
        return -1;
    }

    libusb_transfer_listener = listener;

    cmd_completed = acl_out_completed = 1;

    memset(acl_in_buffer,0,sizeof(acl_in_buffer));
    acl_in_buff_size=0;
    acl_in_buff_size_want=0;
    dat_in_size=0;
    mxd_ubuff_sz=0;
    memset(mxd_ubuff,0,sizeof(mxd_ubuff));

    return ret;
}

static int uart_transport_close()
{
    memset(acl_in_buffer,0,sizeof(acl_in_buffer));
    acl_in_buff_size=0;
    acl_in_buff_size_want=0;
    dat_in_size=0;
    mxd_ubuff_sz=0;
    memset(mxd_ubuff,0,sizeof(mxd_ubuff));

    return uart_trans_destroy();
}

static void uart_transport_request_to_write(u8 channel)
{
    switch (channel)
    {
    case BT_COMMAND_CHANNEL:
        if (cmd_completed)
        {
            cmd_buffer[0]=1;
            libusb_transfer_listener(BT_COMMAND_CHANNEL, cmd_buffer + 1, CMD_BUFFER_SIZE);
        }
        break;
    case BT_ACL_OUT_CHANNEL:
        if (acl_out_completed)
        {
            acl_out_buffer[0]=2;
            libusb_transfer_listener(BT_ACL_OUT_CHANNEL, acl_out_buffer+1, ACL_BUFFER_SIZE);
        }
        break;
    }
}

static void uart_transport_write(u8 channel, u16 size)
{
    int r;

    switch (channel)
    {
    case BT_COMMAND_CHANNEL:
        r= uart_trans_send(cmd_buffer,size+1);
        if (r == 0)
        {
            HCI_TRACE_DEBUG1("Error submitting control transfer %d", r);
        }
        break;
    case BT_ACL_OUT_CHANNEL:
        r= uart_trans_send(acl_out_buffer,size+1);
        if (r < 0)
        {
            HCI_TRACE_DEBUG1("Error submitting acl out transfer, %d", r);
        }
    }
}

static void uart_transport_data_send(u8 channel, u8* buffer,u16 size)
{
    int r;

    switch (channel)
    {
    case BT_COMMAND_CHANNEL:
        if (size<CMD_BUFFER_SIZE)
        {
            cmd_buffer[0]=1;
            memcpy(cmd_buffer+1,buffer,size);
            r= uart_trans_send(cmd_buffer,size+1);
            if (r == 0)
                HCI_TRACE_DEBUG0("Error BT_COMMAND_CHANNEL data transfer \n");

        }
        else
        {
            HCI_TRACE_DEBUG1("Error  cmd  send,size is too large %d", size);
        }
        break;
    case BT_ACL_OUT_CHANNEL:
        if (size<ACL_BUFFER_SIZE)
        {
            //acl_out_buffer[0]=2;
            //memcpy(acl_out_buffer+1,buffer,size);
            r= uart_trans_send(buffer,size);
            if (r == 0)
                HCI_TRACE_DEBUG0("Error  BT_ACL_OUT_CHANNEL  data transfer \n");
        }
        else
        {
            HCI_TRACE_DEBUG1("Error  acl  send,size is too large %d", size);
        }
        break;
    default:
        break;
    }
}


static int uart_transport_poll()
{
    return 0;
}

static const bt_transport_t g_uart_transport =
{
    "uart_transport-transport",
    uart_transport_open,
    uart_transport_close,
    uart_transport_request_to_write,
    uart_transport_write,
    uart_transport_data_send,
    uart_transport_poll,
};

/*****************************************************
* Public Data Define
*****************************************************/
const bt_transport_t * get_ble_trans_adaptor(void)
{
    return &g_uart_transport;
}
