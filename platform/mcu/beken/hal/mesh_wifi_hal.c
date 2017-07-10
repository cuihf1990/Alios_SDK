/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "wlan_ui_pub.h"
#include "txl_frame.h"
#include "mm.h"

#include <yos/framework.h>
#include <yos/kernel.h>

#include <umesh_hal.h>
#include <umesh_80211.h>
#include <hal/wifi.h>

static yos_timer_t g_retransmit_timer;
static yos_mutex_t g_info_mutex;

enum {
    WIFI_MESH_OFFSET = MESH_DATA_OFF,
    WIFI_SRC_OFFSET = OFF_SRC,

    WIFI_MAC_ADDR_SIZE = 6,
};

enum {
    DEFAULT_MTU_SIZE = 256,
};

typedef struct {
    uint32_t u_mtu;
    uint32_t b_mtu;
    uint8_t channel;
    uint8_t chn_num;
    const uint8_t *channels;
    ur_mesh_handle_received_frame_t rxcb;

    void *context;
    ur_mesh_hal_module_t *module;
    mesh_key_t keys[2];
    unsigned char bssid[WIFI_MAC_ADDR_SIZE];
    unsigned char macaddr[WIFI_MAC_ADDR_SIZE];

    frame_stats_t stats;
} mesh_hal_priv_t;

typedef struct send_ext_s {
    void *context;
    void *sent;
    frame_t *frame;
} send_cxt_t;

static send_cxt_t g_send_ucast_cxt;
static send_cxt_t g_send_bcast_cxt;

typedef struct {
    frame_t frm;
    frame_info_t fino;
    mesh_hal_priv_t *priv;
} compound_msg_t;

typedef struct mac_entry_s {
    uint64_t mactime;
    uint16_t last_seq;
    uint8_t  macaddr[6];
} mac_entry_t;

enum {
    ENT_NUM = 32,
};

enum {
    RETRANSMIT_TIMES = 6,
    ROLE_RETRANSMIT_TIMES = 1,
    RETRANSMIT_INTERVAL = 20,  // 20 ms

    FRAME_MCAST = 1 << 1,
    FRAME_CONFIRMED = 1 << 2,
    FRAME_ACKED = 1 << 3,
};

typedef struct tx_frame_info_s {
    ur_mesh_hal_module_t *module;
    send_cxt_t *cxt;
    uint8_t retransmits;
    mac_address_t dest;
    uint8_t flags;
} tx_frame_info_t;
static tx_frame_info_t g_tx_frame_info;

static mac_entry_t entries[ENT_NUM];

static mesh_hal_priv_t *g_hal_priv;

static int send_frame(ur_mesh_hal_module_t *module, frame_t *frame,
                      mac_address_t *dest, send_cxt_t *cxt, bool retry);

static void pass_to_umesh(const void* arg)
{
    compound_msg_t *cmsg = (compound_msg_t *)arg;
    frame_t *frm = &cmsg->frm;
    frame_info_t *fino = &cmsg->fino;
    mesh_hal_priv_t *priv = cmsg->priv;

    if (priv->rxcb) {
        priv->rxcb(priv->context, frm, fino, 0);
    }

    yos_free(cmsg);
}

static void ack_timeout_handler(void *args)
{
    ur_mesh_handle_sent_ucast_t sent;
    uint8_t times;

    if (bk_wlan_is_ap() == 0 && bk_wlan_is_sta() == 0) {
        times = RETRANSMIT_TIMES;
    } else {
        times = ROLE_RETRANSMIT_TIMES;
    }

    g_tx_frame_info.retransmits++;
    if (g_tx_frame_info.retransmits > times) {
        sent = g_tx_frame_info.cxt->sent;
        (*sent)(g_tx_frame_info.cxt->context, g_tx_frame_info.cxt->frame,
                times == ROLE_RETRANSMIT_TIMES? 0: -1);
        g_tx_frame_info.cxt = NULL;
    } else {
        g_tx_frame_info.flags = 0;
        send_frame(g_tx_frame_info.module, g_tx_frame_info.cxt->frame,
                   &g_tx_frame_info.dest, g_tx_frame_info.cxt, true);
    }
}

static void wifi_monitor_cb(uint8_t *data, int len)
{
    compound_msg_t *pf;
    mesh_hal_priv_t *priv = g_hal_priv;
    ur_mesh_hal_module_t *module = priv->module;
    struct mac_hdr *hdr;
    ur_mesh_handle_sent_ucast_t sent;

    hdr = (struct mac_hdr *)data;
    if ((hdr->fctl & MAC_FCTRL_TYPESUBTYPE_MASK) == MAC_FCTRL_ACK) {
        yos_timer_stop(&g_retransmit_timer);
        if (g_tx_frame_info.cxt == NULL) {
            return;
        }

        yos_mutex_lock(&g_info_mutex, YOS_WAIT_FOREVER);
        g_tx_frame_info.flags |= FRAME_ACKED;
        if ((g_tx_frame_info.flags & FRAME_ACKED) &&
            (g_tx_frame_info.flags & FRAME_CONFIRMED)) {
            sent = g_tx_frame_info.cxt->sent;
            (*sent)(g_tx_frame_info.cxt->context,
                    g_tx_frame_info.cxt->frame, 0);
            g_tx_frame_info.cxt = NULL;
        }
        yos_mutex_unlock(&g_info_mutex);
        return;
    }

    if (umesh_80211_filter_frame(module, data, len)) {
        return;
    }

#ifdef YOS_DEBUG_MESH
    dump_packet(true, data, len);
#endif

    pf = yos_malloc(sizeof(*pf) + len - WIFI_MESH_OFFSET);
    bzero(pf, sizeof(*pf));
    pf->frm.len = len - WIFI_MESH_OFFSET;
    pf->frm.data = (void *)(pf + 1);
    memcpy(pf->fino.peer.addr, data + WIFI_SRC_OFFSET, WIFI_MAC_ADDR_SIZE);
    pf->fino.peer.len = 8;

    module = hal_umesh_get_default_module();
    pf->fino.channel = hal_umesh_get_ucast_channel(module);

    pf->priv = priv;
    memcpy(pf->frm.data, data + WIFI_MESH_OFFSET, pf->frm.len);
    pass_to_umesh((const void *)pf);
}

static int beken_wifi_mesh_init(ur_mesh_hal_module_t *module, void *config)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    g_hal_priv = priv;
    wifi_get_mac_address(priv->macaddr);
    return 0;
}

static int beken_wifi_mesh_enable(ur_mesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if (bk_wlan_is_ap() == 0 && bk_wlan_is_sta() == 0) {
        bk_wlan_start_monitor();
        hal_machw_exit_monitor_mode();
        mm_rx_filter_umac_set(MM_RX_FILTER_ACTIVE | NXMAC_ACCEPT_ACK_BIT);
    }

    wlan_register_mesh_monitor_cb(wifi_monitor_cb);
    wlan_set_mesh_bssid(priv->bssid);

    memset(&g_tx_frame_info, 0, sizeof(g_tx_frame_info));

    yos_timer_new(&g_retransmit_timer, ack_timeout_handler,
                  NULL, RETRANSMIT_INTERVAL, 0);
    yos_timer_stop(&g_retransmit_timer);

    yos_mutex_new(&g_info_mutex);
    return 0;
}

static int beken_wifi_mesh_disable(ur_mesh_hal_module_t *module)
{
    wlan_register_mesh_monitor_cb(NULL);
    yos_timer_stop(&g_retransmit_timer);
    yos_timer_free(&g_retransmit_timer);
    yos_mutex_free(&g_info_mutex);
    return 0;
}

static void confirmation_handler(void *args, uint32_t statinfo)
{
    int result = -1;
    send_cxt_t *cxt = NULL;
    ur_mesh_handle_sent_ucast_t sent;

    yos_mutex_lock(&g_info_mutex, YOS_WAIT_FOREVER);
    if ((statinfo & FRAME_SUCCESSFUL_TX_BIT) &&
        (statinfo & DESC_DONE_TX_BIT)) {
        result = 0;
        g_tx_frame_info.flags |= FRAME_CONFIRMED;
    }

    if (result != 0 || (g_tx_frame_info.flags & FRAME_MCAST) ||
        ((g_tx_frame_info.flags & FRAME_CONFIRMED) &&
         (g_tx_frame_info.flags & FRAME_ACKED))) {
        cxt = g_tx_frame_info.cxt;
    }

    if (cxt) {
        sent = cxt->sent;
        (*sent)(cxt->context, cxt->frame, result);
        g_tx_frame_info.cxt = NULL;
    } else {
        yos_timer_start(&g_retransmit_timer);
    }
    yos_mutex_unlock(&g_info_mutex);
}

static int send_frame(ur_mesh_hal_module_t *module, frame_t *frame,
                      mac_address_t *dest, send_cxt_t *cxt, bool retry)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    uint8_t *pkt;
    int len = frame->len + WIFI_MESH_OFFSET;
    struct txl_frame_desc_tag *tx_frame;
    int txtype = TX_DEFAULT_24G;
    struct mac_hdr *hdr;
    struct tx_hd *thd;

    tx_frame = txl_frame_get(txtype, len);
    if (tx_frame == NULL) {
        return -1;
    }

    pkt = (uint8_t *)tx_frame->txdesc.lmac.buffer->payload;

    umesh_80211_make_frame(module, frame, dest, pkt, retry);

    tx_frame->cfm.cfm_func = confirmation_handler;
    tx_frame->cfm.env = cxt;

    thd = &tx_frame->txdesc.lmac.hw_desc->thd;
    hdr = HW2CPU(thd->datastartptr);
    thd->nextfrmexseq_ptr = 0;
    thd->nextmpdudesc_ptr = 0;
    thd->macctrlinfo2 &= ~(WHICHDESC_MSK | UNDER_BA_SETUP_BIT);

    if (bk_wlan_is_ap() == 0 && bk_wlan_is_sta() == 0) {
        thd->macctrlinfo2 |= CO_BIT(29);  // don't touch FC
    }

    if (MAC_ADDR_GROUP(&hdr->addr1)) {
        thd->macctrlinfo1 = EXPECTED_ACK_NO_ACK;
    } else {
        thd->macctrlinfo1 = WRITE_ACK;
    }

    thd->statinfo = 0;

    txl_cntrl_push_int((struct txdesc *)tx_frame, AC_VO);

    priv->stats.out_frames++;
    return 0;
}

static int beken_wifi_mesh_send_ucast(ur_mesh_hal_module_t *module,
                                      frame_t *frame, mac_address_t *dest,
                                      ur_mesh_handle_sent_ucast_t sent,
                                      void *context)
{
    int error;
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if(frame == NULL) {
        return -1;
    }

    if(frame->len > priv->u_mtu) {
        return -2;
    }

    if (g_tx_frame_info.cxt) {
        return -3;
    }

    g_send_ucast_cxt.context = context;
    g_send_ucast_cxt.sent = sent;
    g_send_ucast_cxt.frame = frame;

    g_tx_frame_info.module = module;
    g_tx_frame_info.retransmits = 0;
    g_tx_frame_info.cxt = &g_send_ucast_cxt;
    memcpy(&g_tx_frame_info.dest, dest, sizeof(g_tx_frame_info.dest));
    g_tx_frame_info.flags = 0;

    error = send_frame(module, frame, dest, &g_send_ucast_cxt, false);
    return error;
}

static int beken_wifi_mesh_send_bcast(ur_mesh_hal_module_t *module,
                                      frame_t *frame,
                                      ur_mesh_handle_sent_bcast_t sent,
                                      void *context)
{
    int error;
    mesh_hal_priv_t *priv = module->base.priv_dev;
    mac_address_t dest;

    if(frame == NULL) {
        return -1;
    }

    if(frame->len > priv->b_mtu) {
        return -2;
    }

    if (g_tx_frame_info.cxt) {
        return -3;
    }

    g_send_bcast_cxt.context = context;
    g_send_bcast_cxt.sent = sent;
    g_send_bcast_cxt.frame = frame;
    g_tx_frame_info.cxt = &g_send_bcast_cxt;
    g_tx_frame_info.flags = FRAME_MCAST;

    dest.len = 8;
    memset(dest.addr, 0xff, sizeof(dest.addr));
    error = send_frame(module, frame, &dest, &g_send_bcast_cxt, false);
    return error;
}

static int beken_wifi_mesh_register_receiver(ur_mesh_hal_module_t *module,
                                    ur_mesh_handle_received_frame_t received,
                                    void *context)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if(received == NULL) {
        return -1;
    }

    priv->rxcb = received;
    priv->context = context;
    return 0;
}

static int beken_wifi_mesh_get_mtu(ur_mesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    return priv->u_mtu;
}

static const mac_address_t *beken_wifi_mesh_get_mac_address(
                                         ur_mesh_hal_module_t *module)
{
    static mac_address_t addr;
    mesh_hal_priv_t *priv = module->base.priv_dev;

    memcpy(addr.addr, priv->macaddr, WIFI_MAC_ADDR_SIZE);
    addr.len = 8;
    return &addr;
}

static int beken_wifi_mesh_get_channel(ur_mesh_hal_module_t *module)
{
    int channel;
    struct phy_channel_info phy_info;

    phy_get_channel(&phy_info, PHY_PRIM);
    channel = rw_ieee80211_get_chan_id(phy_info.info1 >> 16);
    return channel;
}

static int beken_wifi_mesh_set_channel(ur_mesh_hal_module_t *module,
                                       uint8_t channel)
{
    uint32_t freq;

    freq = rw_ieee80211_get_centre_frequency(channel);
    phy_set_channel(PHY_BAND_2G4, PHY_CHNL_BW_20, freq, freq, 0, PHY_PRIM);
    return 0;
}

static int beken_wifi_mesh_get_channel_list(ur_mesh_hal_module_t *module,
                                            const uint8_t **chnlist)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    if (chnlist == NULL) {
        return -1;
    }

    *chnlist = priv->channels;
    return priv->chn_num;
}

int beken_wifi_mesh_set_extnetid(ur_mesh_hal_module_t *module,
                                 const umesh_extnetid_t *extnetid)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    memcpy(priv->bssid, extnetid->netid, WIFI_MAC_ADDR_SIZE);
    wlan_set_mesh_bssid(priv->bssid);
    return 0;
}

void beken_wifi_mesh_get_extnetid(ur_mesh_hal_module_t *module,
                                  umesh_extnetid_t *extnetid)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if (extnetid == NULL) {
        return;
    }
    extnetid->len = WIFI_MAC_ADDR_SIZE;
    memcpy(extnetid->netid, priv->bssid, extnetid->len);
}

static const frame_stats_t *beken_wifi_mesh_get_stats(
                                         ur_mesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    return &priv->stats;
}

static ur_mesh_hal_module_t beken_wifi_mesh_module;
static const uint8_t g_wifi_channels[] = {1, 6, 11};
static mesh_hal_priv_t g_wifi_priv = {
    .u_mtu = DEFAULT_MTU_SIZE,
    .b_mtu = DEFAULT_MTU_SIZE,
    .channel = 0,
    .chn_num = sizeof(g_wifi_channels),
    .channels = g_wifi_channels,
    .module = &beken_wifi_mesh_module,
    .bssid = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5},
};

static ur_mesh_hal_module_t beken_wifi_mesh_module = {
    .base.name = "beken7231_mesh_wifi_module",
    .base.priv_dev = &g_wifi_priv,
    .type = MEDIA_TYPE_WIFI,
    .ur_mesh_hal_init = beken_wifi_mesh_init,
    .ur_mesh_hal_enable = beken_wifi_mesh_enable,
    .ur_mesh_hal_disable = beken_wifi_mesh_disable,
    .ur_mesh_hal_send_ucast_request = beken_wifi_mesh_send_ucast,
    .ur_mesh_hal_send_bcast_request = beken_wifi_mesh_send_bcast,
    .ur_mesh_hal_register_receiver = beken_wifi_mesh_register_receiver,
    .ur_mesh_hal_set_bcast_mtu = 0,
    .ur_mesh_hal_get_bcast_mtu = beken_wifi_mesh_get_mtu,
    .ur_mesh_hal_get_ucast_mtu = beken_wifi_mesh_get_mtu,
    .ur_mesh_hal_set_mac_address = 0,
    .ur_mesh_hal_get_mac_address = beken_wifi_mesh_get_mac_address,
    .ur_mesh_hal_get_ucast_channel = beken_wifi_mesh_get_channel,
    .ur_mesh_hal_get_bcast_channel = beken_wifi_mesh_get_channel,
    .ur_mesh_hal_set_ucast_channel = beken_wifi_mesh_set_channel,
    .ur_mesh_hal_set_bcast_channel = beken_wifi_mesh_set_channel,
    .umesh_hal_set_extnetid = beken_wifi_mesh_set_extnetid,
    .umesh_hal_get_extnetid = beken_wifi_mesh_get_extnetid,
    .ur_mesh_hal_get_stats = beken_wifi_mesh_get_stats,
    .ur_mesh_hal_get_bcast_chnlist = beken_wifi_mesh_get_channel_list,
    .ur_mesh_hal_get_ucast_chnlist = beken_wifi_mesh_get_channel_list,
};

void beken_wifi_mesh_register(void)
{
    hal_umesh_register_module(&beken_wifi_mesh_module);
}
