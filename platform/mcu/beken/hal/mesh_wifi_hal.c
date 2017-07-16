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
#include "rw_msdu.h"
#include "txu_cntrl.h"
#include "mm.h"
#include "reg_mdm_cfg.h"
#include "phy_trident.h"

#include "umesh_hal.h"
#include <umesh_80211.h>
#include <hal/wifi.h>

enum {
    WIFI_MESH_OFFSET = 32,
    WIFI_DST_OFFSET = 4,
    WIFI_SRC_OFFSET = 10,
    WIFI_BSSID_OFFSET = 16,

    WIFI_MAC_ADDR_SIZE = 6,
};

enum {
    DEFAULT_MTU_SIZE = 1024,
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

static mac_entry_t entries[ENT_NUM];

static mesh_hal_priv_t *g_hal_priv;

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

static void wifi_monitor_cb(uint8_t *data, int len,
                            hal_wifi_link_info_t *info)
{
    compound_msg_t *pf;
    mesh_hal_priv_t *priv = g_hal_priv;
    ur_mesh_hal_module_t *module = priv->module;

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
    pf->fino.rssi = info->rssi;

    pf->priv = priv;
    memcpy(pf->frm.data, data + WIFI_MESH_OFFSET, pf->frm.len);

    if (yos_schedule_call(pass_to_umesh, pf) < 0) {
        yos_free(pf);
    } else {
        priv->stats.in_frames++;
    }
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
    struct tx_policy_tbl *pol;

    if (bk_wlan_is_ap() == 0 && bk_wlan_is_sta() == 0) {
        bk_wlan_start_monitor();
        hal_machw_exit_monitor_mode();

        tpc_update_tx_power(PHY_TRIDENT_LIMIT_PWR);
        pol = &txl_buffer_control_24G.policy_tbl;
        pol->powercntrlinfo[0] =
              (nxmac_dsss_max_pwr_level_getf() << TX_PWR_LEVEL_PT_RCX_OFT);
    }

    wlan_register_mesh_monitor_cb(wifi_monitor_cb);
    wlan_set_mesh_bssid(priv->bssid);
    return 0;
}

static int beken_wifi_mesh_disable(ur_mesh_hal_module_t *module)
{
    wlan_register_mesh_monitor_cb(NULL);
    return 0;
}

static int send_frame(ur_mesh_hal_module_t *module, frame_t *frame,
                      mac_address_t *dest, send_cxt_t *cxt)
{
    static uint16_t nb_pkt_sent;
    mesh_hal_priv_t *priv = module->base.priv_dev;
    uint8_t *pkt;
    int len = frame->len + WIFI_MESH_OFFSET;
    ur_mesh_handle_sent_ucast_t sent;
    MSDU_NODE_T *node;
    UINT8 *content_ptr;
    UINT32 queue_idx = AC_VI;
    int result = 0;
    struct txdesc *txdesc_new;
    struct umacdesc *umac;

    node = rwm_tx_node_alloc(len);
    if (node == NULL) {
        result = -1;
        goto tx_exit;
    }

    pkt = yos_malloc(len);
    umesh_80211_make_frame(module, frame, dest, pkt);

    rwm_tx_msdu_renew(pkt, len, node->msdu_ptr);
    content_ptr = rwm_get_msdu_content_ptr(node);

    txdesc_new = tx_txdesc_prepare(queue_idx);
    if(txdesc_new == NULL || TXDESC_STA_USED == txdesc_new->status) {
        rwm_node_free(node);
        result = -1;
        goto tx_exit;
    }

    txdesc_new->status = TXDESC_STA_USED;
    txdesc_new->host.flags = TXU_CNTRL_MGMT;
    txdesc_new->host.orig_addr = (UINT32)node->msdu_ptr;
    txdesc_new->host.packet_addr = (UINT32)content_ptr;
    txdesc_new->host.packet_len = len;
    txdesc_new->host.status_desc_addr = (UINT32)content_ptr;
    txdesc_new->host.tid = 0xff;

    umac = &txdesc_new->umac;
    umac->payl_len = len;
    umac->head_len = 0;
    umac->tail_len = 0;
    umac->hdr_len_802_2 = 0;

    umac->buf_control = &txl_buffer_control_24G;

    txdesc_new->lmac.agg_desc = NULL;
    txdesc_new->lmac.hw_desc->cfm.status = 0;

    rwm_push_tx_list(node);
    txl_cntrl_push(txdesc_new, queue_idx);
    priv->stats.out_frames++;

tx_exit:

    yos_free(pkt);

    if (cxt) {
        sent = cxt->sent;
        (*sent)(cxt->context, cxt->frame, result);
    }

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

    g_send_ucast_cxt.context = context;
    g_send_ucast_cxt.sent = sent;
    g_send_ucast_cxt.frame = frame;
    error = send_frame(module, frame, dest, &g_send_ucast_cxt);
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

    g_send_bcast_cxt.context = context;
    g_send_bcast_cxt.sent = sent;
    g_send_bcast_cxt.frame = frame;

    dest.len = 8;
    memset(dest.addr, 0xff, sizeof(dest.addr));
    error = send_frame(module, frame, &dest, &g_send_bcast_cxt);
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
static const uint8_t g_wifi_channels[] = {1, 4, 6, 9, 11};
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
