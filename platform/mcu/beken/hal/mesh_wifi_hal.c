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

#include "umesh_hal.h"
#include "hal/wifi.h"

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

static inline uint16_t calc_seqctrl(uint8_t *data)
{
    return (data[23] << 4) | (data[22] >> 4);
}

static inline void dump_packet(bool rx, uint8_t *data, int len)
{
    ur_mesh_hal_module_t *module;
    int seqno = calc_seqctrl(data);
    uint8_t index;
    uint8_t channel;

    if (rx) {
        printf("rx: ");
    } else {
        printf("tx: ");
    }
    module = hal_ur_mesh_get_default_module();
    channel = hal_ur_mesh_get_ucast_channel(module);
    printf("on channel %d, seq %d, size %d, ctrl %02x:%02x\r\n", channel, seqno, len, data[0], data[1]);
    printf("  to");
    for (index = WIFI_DST_OFFSET; index < WIFI_DST_OFFSET + 6; index++) {
        printf(":%02x", data[index]);
    }
    printf("\r\n");
    printf("  from");
    for (index = WIFI_SRC_OFFSET; index < WIFI_SRC_OFFSET + 6; index++) {
        printf(":%02x", data[index]);
    }
    printf("\r\n");
    printf("  bssid");
    for (index = WIFI_BSSID_OFFSET; index < WIFI_BSSID_OFFSET + 6; index++) {
        printf(":%02x", data[index]);
    }
    printf("\r\n");

    for (index = 0; index < len; index++) {
        printf("%02x:", data[index]);
    }
    printf("\r\n");
}

static mac_entry_t *find_mac_entry(uint8_t  macaddr[6])
{
    mac_entry_t *ment, *yent = NULL;
    uint64_t youngest = -1ULL;
    int i;

    for (i=0; i < ENT_NUM; i++) {
        ment = entries + i;
        if (memcmp(ment->macaddr, macaddr, 6) == 0)
            return ment;

        if (ment->mactime > youngest)
            continue;

        youngest = ment->mactime;
        yent = ment;
    }

    bzero(yent, sizeof(*yent));
    memcpy(yent->macaddr, macaddr, 6);
    return yent;
}

static int filter_packet(mesh_hal_priv_t *priv, uint8_t *data, int len)
{
    uint16_t seqno = calc_seqctrl(data) << 4;
    mac_entry_t *ent;
    uint32_t now;
    uint8_t index;

    if (len < WIFI_MESH_OFFSET) {
        return 1;
    }

    if (memcmp(data + WIFI_BSSID_OFFSET, priv->bssid, WIFI_MAC_ADDR_SIZE) ||
        memcmp(data + WIFI_SRC_OFFSET, priv->macaddr, WIFI_MAC_ADDR_SIZE) == 0) {
        return 1;
    }

    priv->stats.in_frames++;

    now = yos_now() / 1000000;
    ent = find_mac_entry(data + WIFI_SRC_OFFSET);
    if (now - ent->mactime > 100000) {
        ent->mactime = now;
        ent->last_seq = seqno;
        return 0;
    }
    ent->mactime = now;

    if ((int16_t)(seqno - ent->last_seq) <= 0) {
        return 1;
    }

    ent->last_seq = seqno;
    return 0;
}

static void wifi_monitor_cb(uint8_t *data, int len)
{
    compound_msg_t *pf;
    mesh_hal_priv_t *priv;
    ur_mesh_hal_module_t *module;

    priv = g_hal_priv;
    if (filter_packet(priv, data, len)) {
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

    module = hal_ur_mesh_get_default_module();
    pf->fino.channel = hal_ur_mesh_get_ucast_channel(module);

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
    if (bk_wlan_is_ap() == 0 && bk_wlan_is_sta() == 0) {
        bk_wlan_start_monitor();
        hal_machw_exit_monitor_mode();
    }
    wlan_register_mesh_monitor_cb(wifi_monitor_cb);
    return 0;
}

static int beken_wifi_mesh_disable(ur_mesh_hal_module_t *module)
{
    wlan_register_mesh_monitor_cb(NULL);
    return 0;
}

static int send_frame(ur_mesh_hal_module_t *module, frame_t *frame, mac_address_t *dest)
{
    static unsigned long nb_pkt_sent;
    mesh_hal_priv_t *priv = module->base.priv_dev;
    unsigned char *pkt;
    int len = frame->len + WIFI_MESH_OFFSET;
    struct txl_frame_desc_tag *tx_frame;
    int txtype = TX_DEFAULT_24G;

    tx_frame = txl_frame_get(txtype, len);
    if (tx_frame == NULL) {
        return 1;
    }

    pkt = (struct preq_frame *)tx_frame->txdesc.lmac.buffer->payload;
    bzero(pkt, WIFI_MESH_OFFSET);
    pkt[0] = 0x08;

    memcpy(pkt + WIFI_DST_OFFSET, dest->addr, WIFI_MAC_ADDR_SIZE);
    memcpy(pkt + WIFI_SRC_OFFSET, priv->macaddr, WIFI_MAC_ADDR_SIZE);
    memcpy(pkt + WIFI_BSSID_OFFSET, priv->bssid, WIFI_MAC_ADDR_SIZE);

    /* sequence control */
    pkt[22] = (nb_pkt_sent & 0x0000000F) << 4;
    pkt[23] = (nb_pkt_sent & 0x00000FF0) >> 4;
    nb_pkt_sent++;

    pkt[24] = 0xaa;
    pkt[25] = 0xaa;

    tx_frame->cfm.cfm_func = NULL;
    tx_frame->cfm.env = NULL;

    memcpy(pkt + WIFI_MESH_OFFSET, frame->data, frame->len);

#ifdef YOS_DEBUG_MESH
    dump_packet(false, pkt, len);
#endif

    txl_frame_push(tx_frame, AC_VO);

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

    error = send_frame(module, frame, dest);
    if(sent) {
        (*sent)(context, frame, error);
    }
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

    dest.len = 8;
    memset(dest.addr, 0xff, sizeof(dest.addr));
    error = send_frame(module, frame, &dest);
    if(sent) {
        (*sent)(context, frame, error);
    }
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

static int beken_wifi_mesh_set_meshnetid(ur_mesh_hal_module_t *module,
                                         const meshnetid_t *meshnetid)
{
    return 0;
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
    .ur_mesh_hal_set_meshnetid = 0,
    .ur_mesh_hal_get_stats = beken_wifi_mesh_get_stats,
    .ur_mesh_hal_get_bcast_chnlist = beken_wifi_mesh_get_channel_list,
    .ur_mesh_hal_get_ucast_chnlist = beken_wifi_mesh_get_channel_list,
};

void beken_wifi_mesh_register(void)
{
    hal_ur_mesh_register_module(&beken_wifi_mesh_module);
}
