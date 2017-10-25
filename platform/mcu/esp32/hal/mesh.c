/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_wifi_internal.h"

#include "hal/wifi.h"
#include "umesh_hal.h"

enum {
    DEFAULT_MTU_SIZE = 1024,
    WIFI_MAC_ADDR_SIZE = 6,
};

typedef struct {
    uint32_t u_mtu;
    uint32_t b_mtu;
    uint8_t channel;
    uint8_t chn_num;
    const uint8_t *channels;
    umesh_handle_received_frame_t rxcb;

    void *context;
    umesh_hal_module_t *module;
    mesh_key_t keys[2];
    unsigned char bssid[WIFI_MAC_ADDR_SIZE];
    unsigned char macaddr[WIFI_MAC_ADDR_SIZE];

    frame_stats_t stats;
} mesh_hal_priv_t;

static umesh_hal_module_t esp32_wifi_mesh_module;

/* WiFi HAL */
static void esp_wifi_eibss_rx_func(const void* buffer, int len,
                                   const uint8_t* src)
{
    mesh_hal_priv_t *priv = esp32_wifi_mesh_module.base.priv_dev;
    frame_info_t frame_info;
    frame_t frame;

    priv->stats.in_frames++;
    if (priv->rxcb) {
        memset(&frame_info.peer, 0, sizeof(mac_address_t));
        frame_info.peer.len = 8;
        memcpy(frame_info.peer.addr, src, 6);
        frame.len = len;
        frame.data = (uint8_t *)malloc(frame.len);
        memcpy(frame.data, (uint8_t *)buffer, frame.len);
        priv->rxcb(priv->context, &frame, &frame_info, 0);
        free(frame.data);
    }
}

static int esp32_wifi_mesh_init(umesh_hal_module_t *module, void *config)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    esp_wifi_eibss_set_bssid(priv->bssid);
    return 0;
}

static int esp32_wifi_mesh_enable(umesh_hal_module_t *module)
{
    esp_wifi_eibss_start();
    return 0;
}

static int esp32_wifi_mesh_disable(umesh_hal_module_t *module)
{
    return 0;
}

static int esp32_wifi_mesh_send_ucast(umesh_hal_module_t *module,
                                    frame_t *frame, mac_address_t *dest,
                                    umesh_handle_sent_ucast_t sent,
                                    void *context)
{
    int32_t result;
    mesh_hal_priv_t *priv = module->base.priv_dev;

    result = esp_wifi_eibss_tx(frame->data, frame->len, dest->addr);
    if (sent) {
        sent(priv->context, frame, result == 0? 0: -1);
        if (result == 0) {
            priv->stats.out_frames++;
        }
    }
    return result;
}

static int esp32_wifi_mesh_send_bcast(umesh_hal_module_t *module,
                                      frame_t *frame,
                                      umesh_handle_sent_bcast_t sent,
                                      void *context)
{
    int32_t result;
    mesh_hal_priv_t *priv = module->base.priv_dev;
    uint8_t dest[6];

    if(frame == NULL) {
        return -1;
    }

    if(frame->len > priv->b_mtu) {
        return -2;
    }

    memset(dest, 0xff, sizeof(dest));
    result = esp_wifi_eibss_tx(frame->data, frame->len, dest);
    if (sent) {
        sent(priv->context, frame, result == 0? 0: -1);
        if (result == 0) {
            priv->stats.out_frames++;
        }
    }
    return result;
}

static int esp32_wifi_mesh_register_receiver(umesh_hal_module_t *module,
                                    umesh_handle_received_frame_t received,
                                    void *context)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if(received == NULL) {
        return -1;
    }

    priv->rxcb = received;
    priv->context = context;
    esp_wifi_eibss_reg_rxcb((esp_wifi_eibss_rxcb_t)esp_wifi_eibss_rx_func);
    return 0;
}

static int esp32_wifi_mesh_get_mtu(umesh_hal_module_t *module)
{
    return 1024;
}

static const mac_address_t *esp32_wifi_mesh_get_mac_address(umesh_hal_module_t *module)
{
    static mac_address_t addr;
    mesh_hal_priv_t *priv = module->base.priv_dev;

    esp_wifi_eibss_get_mac(priv->macaddr);
    memcpy(addr.addr, priv->macaddr, WIFI_MAC_ADDR_SIZE);
    addr.len = 8;
    return &addr;
}

static int esp32_wifi_mesh_get_channel(umesh_hal_module_t *module)
{
    int channel = 0;
    return channel;
}

static int esp32_wifi_mesh_set_channel(umesh_hal_module_t *module,
                                  uint8_t channel)
{
    /* disable channel switch to avoid interfere with AP */
    return 0;

    /* dont change channel if connected to AP */
#if 0
    if (hal_wifi_is_connected(NULL))
        return 0;
#endif

    esp_wifi_eibss_set_channel(channel, 0);
    return 0;
}

static int esp32_wifi_mesh_get_channel_list(umesh_hal_module_t *module,
                                            const uint8_t **chnlist)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    if (chnlist == NULL) {
        return -1;
    }

    *chnlist = priv->channels;
    return priv->chn_num;
}

static int esp32_wifi_mesh_set_extnetid(umesh_hal_module_t *module,
                                   const umesh_extnetid_t *extnetid)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if (extnetid) {
        memcpy(priv->bssid, (uint8_t *)&extnetid->netid, extnetid->len);
        esp_wifi_eibss_set_bssid((uint8_t *)&extnetid->netid);
    }
    return 0;
}

void esp32_wifi_mesh_get_extnetid(umesh_hal_module_t *module,
                                  umesh_extnetid_t *extnetid)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if (extnetid == NULL) {
        return;
    }
    extnetid->len = WIFI_MAC_ADDR_SIZE;
    memcpy(extnetid->netid, priv->bssid, extnetid->len);
}

static const frame_stats_t *esp32_wifi_mesh_get_stats(umesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    return &priv->stats;
}

static const uint8_t g_wifi_channels[] = {1, 4, 6, 9, 11};
static mesh_hal_priv_t g_wifi_priv = {
    .u_mtu = DEFAULT_MTU_SIZE,
    .b_mtu = DEFAULT_MTU_SIZE,
    .channel = 0,
    .chn_num = sizeof(g_wifi_channels),
    .channels = g_wifi_channels,
    .module = &esp32_wifi_mesh_module,
    .bssid = {0xb0, 0xf8, 0x93, 0x00, 0x00, 0x07},
};

static umesh_hal_module_t esp32_wifi_mesh_module = {
    .base.name = "esp32_mesh_wifi_module",
    .base.priv_dev = &g_wifi_priv,
    .type = MEDIA_TYPE_WIFI,
    .umesh_hal_init = esp32_wifi_mesh_init,
    .umesh_hal_enable = esp32_wifi_mesh_enable,
    .umesh_hal_disable = esp32_wifi_mesh_disable,
    .umesh_hal_send_ucast_request = esp32_wifi_mesh_send_ucast,
    .umesh_hal_send_bcast_request = esp32_wifi_mesh_send_bcast,
    .umesh_hal_register_receiver = esp32_wifi_mesh_register_receiver,
    .umesh_hal_get_bcast_mtu = esp32_wifi_mesh_get_mtu,
    .umesh_hal_get_ucast_mtu = esp32_wifi_mesh_get_mtu,
    .umesh_hal_get_mac_address = esp32_wifi_mesh_get_mac_address,
    .umesh_hal_get_channel = esp32_wifi_mesh_get_channel,
    .umesh_hal_set_channel = esp32_wifi_mesh_set_channel,
    .umesh_hal_set_extnetid = esp32_wifi_mesh_set_extnetid,
    .umesh_hal_get_extnetid = esp32_wifi_mesh_get_extnetid,
    .umesh_hal_get_stats = esp32_wifi_mesh_get_stats,
    .umesh_hal_get_chnlist = esp32_wifi_mesh_get_channel_list,
};

void esp32_wifi_mesh_register(void)
{
    hal_umesh_register_module(&esp32_wifi_mesh_module);
}
