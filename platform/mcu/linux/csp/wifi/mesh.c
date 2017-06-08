/*
 * mesh hal impl.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>

#include <yos/log.h>
#include <yos/kernel.h>

#include <cpu_event.h>
#include <umesh_hal.h>

#include "osdep.h"

#define MYTAG "DDA_HAL"
#define DEFAULT_MTU_SIZE 1024

#ifndef LINUX_WIFI_MESH_IF_NAME
#define LINUX_WIFI_MESH_IF_NAME "mon0"
#endif

#define OFF_DST 4
#define OFF_SRC 10
#define OFF_BSS 16

#undef USE_ACTION_FRAME
#undef NEED_ACK

typedef struct {
    uint32_t u_mtu;
    uint32_t b_mtu;
    uint8_t channel;
    uint8_t chn_num;
    const uint8_t *channels;
    ur_mesh_handle_received_frame_t rxcb;

    struct wif *wif;

    void *context;
    ur_mesh_hal_module_t *module;
    mesh_key_t keys[2];
    unsigned char bssid[6];
    unsigned char macaddr[6];

    frame_stats_t stats;
} mesh_hal_priv_t;

static void payload_encrypt(mesh_hal_priv_t *priv, int8_t key_index, frame_t *frame)
{
#if 0
    uint8_t index;

    if (key_index < 0) {
        return;
    }

    for (index = 0; (index < priv->keys[key_index].len) && (index < (frame->len -2)); index++) {
        frame->data[index + 2] ^= priv->keys[key_index].key[index];
    }
#endif
}

typedef struct {
    frame_t frm;
    frame_info_t fino;
    mesh_hal_priv_t *priv;
} compound_msg_t;

static void linuxhost_mesh_recv(frame_t *frm, frame_info_t *fino, void *cb_data)
{
    mesh_hal_priv_t *priv = cb_data;
    uint8_t control;

    if (!priv->rxcb)
        return;

    control = frm->data[1];
    if (control & 0x01) {
        payload_encrypt(priv, fino->key_index, frm);
    }
    priv->rxcb(priv->context, frm, fino, 0);
}

static void pass_to_urmesh(const void* arg)
{
    compound_msg_t *cmsg = (compound_msg_t *)arg;

    linuxhost_mesh_recv(&cmsg->frm, &cmsg->fino, cmsg->priv);

    cpu_event_free(cmsg);
}

static inline uint16_t calc_seqctrl(unsigned char *pkt)
{
    return (pkt[23] << 4) | (pkt[22] >> 4);
}

typedef struct mac_entry_s {
    uint64_t mactime;
    uint16_t last_seq;
    uint8_t  macaddr[6];
} mac_entry_t;

#define ENT_NUM 32
static mac_entry_t entries[ENT_NUM];
static mac_entry_t *find_mac_entry(uint8_t  macaddr[6])
{
    mac_entry_t *ment, *yent = NULL;
    uint64_t youngest = -1ULL;
    int i;

    for (i=0;i<ENT_NUM;i++) {
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

static void p_addr(unsigned char *pkt)
{
	printf(":%02x %02x %02x %02x %02x %02x", pkt[0] , pkt[1] , pkt[2] , pkt[3] , pkt[4] , pkt[5]);
}

static inline void dump_packet(unsigned char *pkt, int count)
{
    int seqno = calc_seqctrl(pkt);
	printf("%s(%d) %02x %02x %02x", __func__, count, pkt[0], pkt[1], seqno);
	p_addr(pkt+OFF_DST);
	p_addr(pkt+OFF_SRC);
	p_addr(pkt+OFF_BSS);
	printf("\n");
}

static int filter_packet(mesh_hal_priv_t *priv, unsigned char *pkt, struct rx_info *ri)
{
    uint16_t seqno = calc_seqctrl(pkt) << 4;
    mac_entry_t *ent;

    if (memcmp(pkt+OFF_BSS, priv->bssid, 6) ||
            memcmp(pkt+OFF_SRC, priv->macaddr, 6) == 0)
        return 1;

    priv->stats.in_frames ++;
    ent = find_mac_entry(pkt+OFF_SRC);
    /* if longer than 100ms */
    if (ri->ri_mactime - ent->mactime > 100000) {
        ent->mactime = ri->ri_mactime;
        ent->last_seq = seqno;
        return 0;
    }

    ent->mactime = ri->ri_mactime;
    if ((int16_t)(seqno - ent->last_seq) <= 0) {
#ifdef MDEBUG
        dump_packet(pkt, 32);
        printf("duplicate seqno %02x %02x %02x %02x %02x %02x\n", ent->last_seq, seqno, pkt[0], pkt[1], pkt[22], pkt[23]);
#endif
        return 1;
    }

    ent->last_seq = seqno;
	return 0;
}

#define MESH_DATA_OFF 32
static void *wifi_recv_entry(void *arg)
{
    mesh_hal_priv_t *priv = arg;
    fd_set rfds;
    int fd = wi_fd(priv->wif);
    unsigned char pkt[2048];

    while (1) {
        int ret;
        int count = sizeof pkt;
        struct rx_info ri;

        FD_SET( fd, &rfds );

        ret = select(fd + 1, &rfds, NULL, NULL, NULL);
        if (ret < 0)
            continue;

        if (!FD_ISSET(fd, &rfds))
            continue;

        count = wi_read(priv->wif, pkt, count, &ri);
        if (count < 25)
            continue;
        if (filter_packet(priv, pkt, &ri))
            continue;

#ifdef MDEBUG
        dump_packet(pkt, count);
#endif

        compound_msg_t *pf;
        pf = cpu_event_malloc(sizeof(*pf) + count - MESH_DATA_OFF);
        bzero(pf, sizeof(*pf));
        pf->frm.len = count - MESH_DATA_OFF;
        pf->frm.data = (void *)(pf + 1);
        pf->fino.channel = ri.ri_channel;
        memcpy(pf->fino.peer.addr, pkt + OFF_SRC, 6);
        pf->fino.peer.len = 8;
        pf->priv = priv;
        memcpy(pf->frm.data, pkt + MESH_DATA_OFF, pf->frm.len);
        cpu_call_handler(pass_to_urmesh, pf);
    }

    return NULL;
}

static int linux_80211_mesh_init(ur_mesh_hal_module_t *module, void *something)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    priv->wif = wi_open(LINUX_WIFI_MESH_IF_NAME);
    if (priv->wif == NULL)
        return -1;

    wi_get_mac(priv->wif, priv->macaddr);
    pthread_t th;
    pthread_create(&th, NULL, wifi_recv_entry, priv);
    return 0;
}

static int send_frame(ur_mesh_hal_module_t *module, frame_t *frame, mac_address_t *dest)
{
    static unsigned long nb_pkt_sent;
    mesh_hal_priv_t *priv = module->base.priv_dev;
    unsigned char *pkt;
    int count = frame->len + MESH_DATA_OFF;

    pkt = yos_malloc(count);
    bzero(pkt, MESH_DATA_OFF);
#ifdef USE_ACTION_FRAME
    pkt[0] = 0xd0;
#else
    pkt[0] = 0x08;
#endif
    memcpy(pkt + OFF_DST, dest->addr, 6);
    memcpy(pkt + OFF_SRC, priv->macaddr, 6);
    memcpy(pkt + OFF_BSS, priv->bssid, 6);

    /* sequence control */
    pkt[22] = (nb_pkt_sent & 0x0000000F) << 4;
    pkt[23] = (nb_pkt_sent & 0x00000FF0) >> 4;
    nb_pkt_sent++;

#ifdef USE_ACTION_FRAME
    pkt[24] = 127;
#endif

    memcpy(pkt + MESH_DATA_OFF, frame->data, frame->len);

    wi_write(priv->wif, pkt, count, NULL);

    yos_free(pkt);

    priv->stats.out_frames ++;
    return 0;
}

static int linux_80211_mesh_send_ucast(ur_mesh_hal_module_t *module, frame_t *frame,
                                   mac_address_t *dest,
                                   ur_mesh_handle_sent_ucast_t sent,
                                   void *context)
{
    int error;
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if(frame == NULL) {
        LOGE(MYTAG, "frame is NULL, cannot proceed");
        return -1;
    }

    if(frame->len > priv->u_mtu) {
        LOGE(MYTAG, "frame->len(%d) > MAX_FRAME_SIZE(%d), will not proceed", frame->len, priv->u_mtu);
        return -2;
    }

    payload_encrypt(priv, frame->key_index, frame);
    error = send_frame(module, frame, dest);
    if(sent) {
        (*sent)(context, frame, error);
    }
    return error;
}

static int linux_80211_mesh_send_bcast(ur_mesh_hal_module_t *module, frame_t *frame,
                                   ur_mesh_handle_sent_bcast_t sent,
                                   void *context)
{
    int error;
    mesh_hal_priv_t *priv = module->base.priv_dev;
    mac_address_t dest;

    if(frame == NULL) {
        LOGE(MYTAG, "frame is NULL, cannot proceed");
        return -1;
    }

    if(frame->len > priv->b_mtu) {
        LOGE(MYTAG, "frame->len(%d) > MAX_FRAME_SIZE(%d), will not proceed", frame->len, priv->u_mtu);
        return -2;
    }

    dest.len = 8;
    memset(dest.addr, 0xff, sizeof(dest.addr));
    payload_encrypt(priv, frame->key_index, frame);
    error = send_frame(module, frame, &dest);
    if(sent) {
        (*sent)(context, frame, error);
    }
    return error;
}

static int linux_80211_mesh_set_mtu(ur_mesh_hal_module_t *module, uint16_t mtu)
{
    return -1;
}

static int linux_80211_mesh_get_u_mtu(ur_mesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    return priv->u_mtu;
}

static int linux_80211_mesh_get_b_mtu(ur_mesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    return priv->b_mtu;
}

static int linux_80211_mesh_set_rxcb(ur_mesh_hal_module_t *module,
                          ur_mesh_handle_received_frame_t received, void *context)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if(received == NULL)
        return -1;

    priv->rxcb = received;
    priv->context = context;
    return 0;
}

static const mac_address_t *linux_80211_mesh_get_mac_address(
                                        ur_mesh_hal_module_t *module)
{
    static mac_address_t addr;
    mesh_hal_priv_t *priv = module->base.priv_dev;

    memcpy(addr.addr, priv->macaddr, 6);
    addr.len = 8;
    return &addr;
}

static int linux_80211_mesh_set_key(struct ur_mesh_hal_module_s *module,
                                uint8_t index, uint8_t *key, uint8_t length)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if (index > 1) {
        return -1;
    }

    if (length > MAX_KEY_SIZE) {
        priv->keys[index].len = 0;
        return -1;
    }
    priv->keys[index].len = length;
    memcpy(priv->keys[index].key, key, length);

    return 0;
}

static int linux_80211_mesh_activate_key(struct ur_mesh_hal_module_s *module,
                                     uint8_t index)
{
    return 0;
}

static int linux_80211_mesh_is_sec_enabled(struct ur_mesh_hal_module_s *module)
{
    return 0;
}

static int linux_80211_mesh_hal_set_channel(ur_mesh_hal_module_t *module, uint8_t channel)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
printf("setting channel to %d\n", channel);
    wi_set_channel(priv->wif, channel);
    /* channel will appended to each data packet sent */
    priv->channel = channel;
    return 0;
}

static int linux_80211_mesh_get_channel_list(ur_mesh_hal_module_t *module, const uint8_t **chnlist)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    if (chnlist == NULL) {
        return -1;
    }

    *chnlist = priv->channels;

    return priv->chn_num;
}

static const frame_stats_t *linux_80211_mesh_get_stats(struct ur_mesh_hal_module_s *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    return &priv->stats;
}

static ur_mesh_hal_module_t linux_80211_mesh_wifi_module;
static const uint8_t g_wifi_channels[] = {1, 6, 11};
static mesh_hal_priv_t wifi_priv = {
    .u_mtu = DEFAULT_MTU_SIZE,
    .b_mtu = DEFAULT_MTU_SIZE,
    .channel = 0,
    .chn_num = sizeof(g_wifi_channels),
    .channels = g_wifi_channels,
    .module = &linux_80211_mesh_wifi_module,
    .bssid = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5},
};

static ur_mesh_hal_module_t linux_80211_mesh_wifi_module = {
    .base.name = "linux_80211_mesh_wifi_module",
    .base.priv_dev = &wifi_priv,
    .type = MEDIA_TYPE_WIFI,
    .ur_mesh_hal_init = linux_80211_mesh_init,
    .ur_mesh_hal_send_ucast_request = linux_80211_mesh_send_ucast,
    .ur_mesh_hal_send_bcast_request = linux_80211_mesh_send_bcast,
    .ur_mesh_hal_register_receiver = linux_80211_mesh_set_rxcb,
    .ur_mesh_hal_get_bcast_mtu = linux_80211_mesh_get_b_mtu,
    .ur_mesh_hal_get_ucast_mtu = linux_80211_mesh_get_u_mtu,
    .ur_mesh_hal_set_bcast_mtu = linux_80211_mesh_set_mtu,
    .ur_mesh_hal_set_ucast_mtu = linux_80211_mesh_set_mtu,
    .ur_mesh_hal_get_mac_address = linux_80211_mesh_get_mac_address,
    .ur_mesh_hal_set_ucast_channel = linux_80211_mesh_hal_set_channel,
    .ur_mesh_hal_set_bcast_channel = linux_80211_mesh_hal_set_channel,
    .ur_mesh_hal_get_bcast_chnlist = linux_80211_mesh_get_channel_list,
    .ur_mesh_hal_get_ucast_chnlist = linux_80211_mesh_get_channel_list,
    .ur_mesh_hal_set_key = linux_80211_mesh_set_key,
    .ur_mesh_hal_activate_key = linux_80211_mesh_activate_key,
    .ur_mesh_hal_is_sec_enabled = linux_80211_mesh_is_sec_enabled,
    .ur_mesh_hal_get_stats = linux_80211_mesh_get_stats,
};

void linux_wifi_register(void)
{
    hal_ur_mesh_register_module(&linux_80211_mesh_wifi_module);
}
