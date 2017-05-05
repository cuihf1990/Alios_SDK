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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#include <hal/mesh.h>
#include "ble_hcicmds.h"
#include "ble_hci_adaptor.h"
#include "ble_gap.h"

#define HAL_MESH_DATA_MAX_LEN 29
#define ASYNC_CMD_ADV_SEND    1
#define ASYNC_CMD_CONNECT 2


/**********************************************
 * Private Function Declare
 ***********************************************/
static int mxd_run_send_thread(void);
static int asyn_buffer_add(uint8_t type, uint8_t *pdata, int len, ur_mesh_handle_sent_frame_t send_cb);

/**********************************************
 * Private Data Define
 ***********************************************/
static ur_mesh_handle_received_frame_t g_hal_cb = NULL;
static int g_is_run_mesh = 0;
static uint8_t g_tlv[16]= {1, 1, HAL_MESH_DATA_MAX_LEN};

/**********************************************
 * adv reciev callback, ble stack will call this function
 ***********************************************/
int hci_le_adv_report_bearer_cb(uint8_t * data, uint8_t len)
{
    uint8_t evt_type=0;
    uint8_t adr_type=0;
    uint8_t* p = data;
    BD_ADDR Address;
    uint8_t dlen;
    uint8_t vendor_flag;

    p++;
    STREAM_TO_UINT8(evt_type, p);
    STREAM_TO_UINT8(adr_type, p);
    STREAM_TO_BDADDR(Address, p);

    if (0)
        printf("ADV report type=%x,  adr_type=%d, %02x-%02x-%02x-%02x-%02x-%02x] len=%d\n",
               evt_type, adr_type, Address[0],Address[1],Address[2],Address[3],Address[4],Address[5], *p);

    if (evt_type == BLE_ADV_DIRECT_IND_HI || evt_type == BLE_ADV_DIRECT_IND_LOW)
    {
        //[ 3e 0c 02 01 01 01 03 a5 a5 a5 a5 a5 00 7f ]
        if (gap_get_state() !=GAP_STATE_CONN)
        {
            printf("[BLE] auto connect to %d\n", Address[5]);
            asyn_buffer_add(ASYNC_CMD_CONNECT, &Address[5], 1, NULL);
        }
       
        return g_is_run_mesh;
    }

    p++;//ignore a len byte, not seem as adv_data, report_adv_data have total len

    dlen = *p - 1;
    p++;
    vendor_flag = *p;
    p++;

    if (vendor_flag == 0xff)
    {
        frame_t recv_frame;
        memset(&recv_frame, 0, sizeof(recv_frame));

        if (g_hal_cb)
        {
            memcpy(recv_frame.data, p, dlen);
            recv_frame.len = dlen;
            g_hal_cb(&recv_frame, 0);
        }
    }
    return g_is_run_mesh;
}

/**********************************************
 * Init mesh device
 *      set blue tooth address
 *      set adv an scan param
 *      and start scan
 ***********************************************/
static int hal_ble_mesh_init()
{
    BD_ADDR bda_null= {0,0,0,0,0,0};
    //BD_ADDR bda_random={0xA5,0xA5,0xA5,0xA5,0xA5,0x02};

    printf("[BLE]mesh hal init\n");

    //setup hci adaptor,and open
    if (hci_setup()< 0 )
        return -1;

    //send reset hci cmd
    btsnd_hcic_ble_reset();

    //download mxd fw to chip
    mxd_fw_init();

    //printf("set random bda\n");
    //bda_random[5] = g_mid;
    //btsnd_hcic_ble_set_random_addr (bda_random);

    gap_init();

    //printf("set adv param\n");
    btsnd_hcic_ble_write_adv_params (MS2BT(20), MS2BT(20),
                                     BLE_ADV_IND/*BLE_ADV_NONCONN_IND*/, BLE_ADDR_RANDOM,
                                     BLE_ADDR_RANDOM, bda_null,
                                     BLE_ADV_CHNL_37, AP_SCAN_CONN_ALL);

    //printf("set scan param\n");
    btsnd_hcic_ble_set_scan_params (BLE_SCAN_PASSIVE,
                                    MS2BT(10), MS2BT(10),
                                    BLE_ADDR_RANDOM, BLE_SFP_ACCEPT_ALL);

    //printf("start scan\n");
    btsnd_hcic_ble_set_scan_enable(1, 1);
    //btsnd_hcic_ble_set_adv_enable(1);

    g_is_run_mesh = 1;

    //start asyn_send thread
    mxd_run_send_thread();

    return 0;
}

/**********************************************
 * data send by adv,after send switch to scan
 ***********************************************/
static int hal_ble_mesh_send(uint8_t *pdata, int len)
{
    const int switch_wait = 10;

    uint8_t sbuf[HAL_MESH_DATA_MAX_LEN + 2];

    if (len > HAL_MESH_DATA_MAX_LEN)
    {
        printf("send error, data len = %d\n", len);
        return -1;
    }

    printf("mesh hal send (%d)...\n", switch_wait);

    sbuf[0] = (uint8_t)(len+1);
    sbuf[1] = 0xFF;
    memcpy(&sbuf[2], pdata, len);

    //printf("stop scan\n");
    btsnd_hcic_ble_set_scan_enable (0, 1);

    //printf("stop adv\n");
    //btsnd_hcic_ble_set_adv_enable(0);

    //printf("set adv data(%d)\n", len + 2);
    btsnd_hcic_ble_set_adv_data ((uint8_t)len + 2, sbuf);

    //printf("start adv\n", len);
    btsnd_hcic_ble_set_adv_enable(1);

    //Sleep(switch_wait);

    //printf("stop adv\n");
    btsnd_hcic_ble_set_adv_enable(0);

    //printf("start scan\n");
    btsnd_hcic_ble_set_scan_enable(1, 1);

    return 0;
}

/**********************************************
 * Register recv callback
 ***********************************************/
static int hal_ble_mesh_register_recv_cb(ur_mesh_handle_received_frame_t cb)
{
    g_hal_cb = cb;
    return 0;
}


/**********************************************
 * connect
 ***********************************************/
int hal_ble_mesh_connect_adv(uint8_t mid, int direct)
{
    BD_ADDR bda_random= {0xA5,0xA5,0xA5,0xA5,0xA5,0x00};
    BD_ADDR bda_null= {0,0,0,0,0,0};
    bda_random[5] = mid;

    //printf("stop scan\n");
    btsnd_hcic_ble_set_scan_enable (0, 1);

    //set adv param
    if (direct)
    {
        btsnd_hcic_ble_write_adv_params(MS2BT(100), MS2BT(200),
                                    BLE_ADV_DIRECT_IND_LOW, BLE_ADDR_RANDOM,
                                    BLE_ADDR_RANDOM, bda_random,
                                    BLE_ADV_CHNL_37, AP_SCAN_CONN_ALL);
    }
    else
    {
        btsnd_hcic_ble_write_adv_params (MS2BT(1000), MS2BT(2000),
                                     BLE_ADV_IND/*BLE_ADV_NONCONN_IND*/, BLE_ADDR_RANDOM,
                                     BLE_ADDR_RANDOM, bda_null,
                                     BLE_ADV_CHNL_37, AP_SCAN_CONN_ALL);   
    }

    //printf("start adv\n", len);
    btsnd_hcic_ble_set_adv_enable(1);

    return 0;
}

int hal_ble_mesh_connect(uint8_t mid)
{
    BD_ADDR bda_peer= {0xA5,0xA5,0xA5,0xA5,0xA5,0x00};

    //printf("stop scan\n");
    btsnd_hcic_ble_set_scan_enable (0, 1);

    bda_peer[5] = mid;
    btsnd_hcic_ble_create_ll_conn(100,   100,
                                  0,
                                  BLE_ADDR_RANDOM,   bda_peer,
                                  BLE_ADDR_RANDOM,
                                  0x24,   0x24,
                                  0,   1000,
                                  0x20,   0x200);
    return 0;
}


void hal_ble_mesh_acl_send(void)
{
    char acl_buf[32];
    char *puser = acl_buf + 9;

    strcpy(puser, "123456");
    //hci_api_acl_send(acl_buf, sizeof(acl_buf));
    l2cap_data_output(0, (u8*)acl_buf, 6);
}

/**********************************************
 *async send implement
 ***********************************************/
#define ASYNC_SEND_BUF_COUNT 5
static uint8_t g_async_buf[ASYNC_SEND_BUF_COUNT][HAL_MESH_DATA_MAX_LEN];
static uint8_t g_async_len[ASYNC_SEND_BUF_COUNT];
/*1 mesh adv send, 2 mesh connect*/
static uint8_t g_async_type[ASYNC_SEND_BUF_COUNT];
static ur_mesh_handle_sent_frame_t g_async_cb[ASYNC_SEND_BUF_COUNT];


static sem_t g_send_sem;

static void asyn_buffer_init(void)
{
    memset(g_async_len, 0, sizeof(g_async_len));
    memset(g_async_cb, 0, sizeof(g_async_cb));
}

static int asyn_buffer_add(uint8_t type, uint8_t *pdata, int len, ur_mesh_handle_sent_frame_t send_cb)
{
    int i = 0;

    for(i = 0; i < ASYNC_SEND_BUF_COUNT; i++)
    {
        if (g_async_len[i] == 0)
            break;
    }

    if (i < ASYNC_SEND_BUF_COUNT)
    {
        memcpy(g_async_buf[i], pdata, len);
        g_async_len[i] = len;
        g_async_cb[i] = send_cb;
        g_async_type[i] = type;
        sem_post(&g_send_sem);
        return i;
    }
    return -1;
}

static int asyn_buffer_get(uint8_t *type, uint8_t **pdata, int *len, ur_mesh_handle_sent_frame_t * cb)
{
    int i = 0;

    for(i = 0; i < ASYNC_SEND_BUF_COUNT; i++)
    {
        if (g_async_len[i] != 0)
            break;
    }

    if (i < ASYNC_SEND_BUF_COUNT)
    {
        *pdata = g_async_buf[i];
        *len = g_async_len[i];
        *cb = g_async_cb[i];
        *type = g_async_type[i];
        return i;
    }

    return -1;
}

static int asyn_buffer_clear(int idx)
{
    if (idx >= 0 && idx < ASYNC_SEND_BUF_COUNT)
    {
        g_async_len[idx] = 0;
    }
    return 0;
}

static void * send_thread_proc(void * arg)
{
    uint8_t *pdata;
    int len;
    ur_mesh_handle_sent_frame_t send_cb;
    int idx;
    uint8_t type = 0;

    while(1)
    {
        sem_wait(&g_send_sem);

        idx = asyn_buffer_get(&type, &pdata, &len, &send_cb);
        if (idx >= 0)
        {
            switch(type)
            {
            case ASYNC_CMD_ADV_SEND:
                hal_ble_mesh_send(pdata, len);
                if (send_cb) send_cb(NULL);
                break;
            case ASYNC_CMD_CONNECT:
                if (gap_get_state() !=GAP_STATE_CONN)
                {
                    hal_ble_mesh_connect(pdata[0]);
                }
                break;
            default:
                ;
            }

            asyn_buffer_clear(idx);
        }
    }
    return 0;
}

static int mxd_run_send_thread(void)
{
    int ret;
    pthread_t tid;

    //init buffer
    asyn_buffer_init();

    sem_init(&g_send_sem, 0, 0);

    ret = pthread_create(&tid, NULL, send_thread_proc, NULL);

    return ret;
}

static int hal_mesh_send_async(uint8_t *pdata, int len, ur_mesh_handle_sent_frame_t send_cb)
{
    int ret = -1;

    if (asyn_buffer_add(ASYNC_CMD_ADV_SEND, pdata, len, send_cb) >= 0)
    {
        printf("mesh hal send async...\n");
        sem_post(&g_send_sem);
        ret = len;
    }
    else
    {
        printf("mesh hal send,error,busy\n");
    }
    return ret;
}

/**********************************************
 *set mid,for debug. random bda use mid
 ***********************************************/
void hal_mesh_set_mid(int mid)
{
    BD_ADDR bda_random= {0xA5,0xA5,0xA5,0xA5,0xA5,0x02};

    printf("mesh set mid = %d\n", mid);
    bda_random[5] = mid;

    gap_set_random_bd_addr(bda_random);
}

void hal_mesh_reset(uint8_t mid)
{
    BD_ADDR bda_null= {0,0,0,0,0,0};

    //send reset hci cmd
    btsnd_hcic_ble_reset();

    //set random address
    hal_mesh_set_mid(mid);

    gap_init();

   //printf("set adv param\n");
   btsnd_hcic_ble_write_adv_params (MS2BT(20), MS2BT(20),
                                    BLE_ADV_IND/*BLE_ADV_NONCONN_IND*/, BLE_ADDR_RANDOM,
                                    BLE_ADDR_RANDOM, bda_null,
                                    BLE_ADV_CHNL_37, AP_SCAN_CONN_ALL);

   //printf("set scan param\n");
   btsnd_hcic_ble_set_scan_params (BLE_SCAN_PASSIVE,
                                   MS2BT(10), MS2BT(10),
                                   BLE_ADDR_RANDOM, BLE_SFP_ACCEPT_ALL);

   //printf("start scan\n");
   //btsnd_hcic_ble_set_scan_enable(1, 1);

}

/**********************************************
 * Yoc Hal Implement
 ***********************************************/
static int imp_ur_hal_init(struct ur_hal_module_t *module, void *something)
{
    return hal_ble_mesh_init();
}

static int imp_ur_hal_enable(struct ur_hal_module_t *module)
{
    return 0;
}

static int imp_ur_hal_disable(struct ur_hal_module_t *module)
{
    return 0;
}

static int imp_ur_send_ucast_request(struct ur_hal_module_t *module,
                                     frame_t *frame,
                                     ur_mesh_handle_sent_ucast_t sent)
{
    return hal_mesh_send_async(frame->data, frame->len, sent);
}

static int imp_ur_register_receiver(struct ur_hal_module_t *module, ur_mesh_handle_received_frame_t received)
{
    return hal_ble_mesh_register_recv_cb(received);
}

static struct ur_hal_module_t g_ur_hal_module = {
    .base.name = "ur hal",
    .ur_hal_init = imp_ur_hal_init,
    .ur_hal_enable      = imp_ur_hal_enable,
    .ur_hal_disable     = imp_ur_hal_disable,
    .ur_send_ucast_request = imp_ur_send_ucast_request,
    .ur_register_receiver  = imp_ur_register_receiver,
};

/**********************************************
 * Call this function in hw_start_hal
 ***********************************************/
void linuxhost_hal_ble_mesh_register_module(void)
{
    hal_ur_mesh_register_module(&g_ur_hal_module);
}
