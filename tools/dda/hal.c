/*
 * mesh hal impl.
 */
#include <stdio.h>

#include <yos/log.h>

#include "umesh_hal.h"
#include "msg.h"
#include "packet.h"

#define MYTAG "DDA_HAL"

typedef struct {
    uint32_t u_mtu;
    uint32_t b_mtu;
    uint8_t channel;
    uint8_t chn_num;
    uint8_t media_type;
    uint8_t media_index;
    const uint8_t *channels;
    ur_mesh_handle_received_frame_t rxcb;
    void *context;
    ur_mesh_hal_module_t *module;
    mesh_key_t keys[2];
} mesh_hal_priv_t;

static void linuxhost_mesh_recv(frame_t *frm, frame_info_t *fino, void *cb_data)
{
    mesh_hal_priv_t *priv = cb_data;

    if (!priv->rxcb)
        return;

    priv->rxcb(priv->context, frm, fino, 0);
}

static int linuxhost_ur_init(ur_mesh_hal_module_t *module, void *something)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    dda_mesh_register_receiver(priv->media_type, linuxhost_mesh_recv, module->base.priv_dev);
    return 0;
}

static int send_frame(ur_mesh_hal_module_t *module, frame_t *frame, mac_address_t *dest)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    int dst_id = (int)dest->value;
    cmd_priv_t cmd_priv = {
        .mesh = {
            .channel = priv->channel,
            .media_type = priv->media_type,
            .key_index = frame->key_index,
        },
    };
    dda_mesh_send_data(TYPE_MESH, CMD_MESH_DATA, cmd_priv.opaque, dst_id, frame->data, frame->len);

    return 0;
}

static int linuxhost_ur_send_ucast(ur_mesh_hal_module_t *module, frame_t *frame,
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

    error = send_frame(module, frame, dest);
    if(sent) {
        (*sent)(context, frame, error);
    }
    return error;
}

static int linuxhost_ur_send_bcast(ur_mesh_hal_module_t *module, frame_t *frame,
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
    error = send_frame(module, frame, &dest);
    if(sent) {
        (*sent)(context, frame, error);
    }
    return error;
}

static int linuxhost_ur_get_u_mtu(ur_mesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    return priv->u_mtu;
}

static int linuxhost_ur_get_b_mtu(ur_mesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    return priv->b_mtu;
}

static int linuxhost_ur_set_rxcb(ur_mesh_hal_module_t *module,
                          ur_mesh_handle_received_frame_t received, void *context)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    if(received == NULL)
        return -1;

    priv->rxcb = received;
    priv->context = context;
    return 0;
}

static const mac_address_t *linuxhost_ur_get_mac_address(
                                        ur_mesh_hal_module_t *module)
{
    static mac_address_t mac;
    mac.len = 8;
    mac.value = dda_mesh_get_agent_id();
    return &mac;
}

static int linuxhost_ur_set_key(struct ur_mesh_hal_module_s *module,
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

static int linuxhost_ur_is_sec_enabled(struct ur_mesh_hal_module_s *module)
{
    return 0;
}

static int linuxhost_ur_hal_set_channel(ur_mesh_hal_module_t *module, uint8_t channel)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;

    /* channel will appended to each data packet sent */
    priv->channel = channel;
    cmd_priv_t cmd_priv = {
        .mesh = {
            .channel = priv->channel,
            .media_type = priv->media_type,
        },
    };
    dda_mesh_send_data(TYPE_MESH, CMD_MESH_SET_CHN, cmd_priv.opaque, 0, NULL, 0);
    return 0;
}

static int linuxhost_ur_get_channel_list(ur_mesh_hal_module_t *module, const uint8_t **chnlist)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    if (chnlist == NULL) {
        return -1;
    }

    *chnlist = priv->channels;

    return priv->chn_num;
}

static int linuxhost_ur_get_channel(ur_mesh_hal_module_t *module)
{
    mesh_hal_priv_t *priv = module->base.priv_dev;
    return priv->channel;
}

static ur_mesh_hal_module_t linuxhost_ur_wifi_module;
static const uint8_t g_wifi_channels[] = {1, 2, 3};
static mesh_hal_priv_t wifi_priv = {
    .u_mtu = DEFAULT_MTU_SIZE,
    .b_mtu = DEFAULT_MTU_SIZE,
    .channel = 0,
    .media_type = IF_WIFI,
    .chn_num = sizeof(g_wifi_channels),
    .channels = g_wifi_channels,
    .module = &linuxhost_ur_wifi_module,
};

static ur_mesh_hal_module_t linuxhost_ur_wifi_module = {
    .base.name = "linuxhost_ur_wifi_module",
    .base.priv_dev = &wifi_priv,
    .type = MEDIA_TYPE_WIFI,
    .ur_mesh_hal_init = linuxhost_ur_init,
    .ur_mesh_hal_send_ucast_request = linuxhost_ur_send_ucast,
    .ur_mesh_hal_send_bcast_request = linuxhost_ur_send_bcast,
    .ur_mesh_hal_register_receiver = linuxhost_ur_set_rxcb,
    .ur_mesh_hal_get_bcast_mtu = linuxhost_ur_get_b_mtu,
    .ur_mesh_hal_get_ucast_mtu = linuxhost_ur_get_u_mtu,
    .ur_mesh_hal_get_mac_address = linuxhost_ur_get_mac_address,
    .ur_mesh_hal_set_channel = linuxhost_ur_hal_set_channel,
    .ur_mesh_hal_get_chnlist = linuxhost_ur_get_channel_list,
    .ur_mesh_hal_get_channel = linuxhost_ur_get_channel,
    .ur_mesh_hal_set_key = linuxhost_ur_set_key,
    .ur_mesh_hal_is_sec_enabled = linuxhost_ur_is_sec_enabled,
};

static ur_mesh_hal_module_t linuxhost_ur_ble_module;
static const uint8_t g_ble_channels[] = {0, 1};
static mesh_hal_priv_t ble_priv = {
    .u_mtu = DEFAULT_MTU_SIZE,
    .b_mtu = 24,
    .channel = 0,
    .media_type = IF_BLE,
    .chn_num = sizeof(g_ble_channels),
    .channels = g_ble_channels,
    .module = &linuxhost_ur_ble_module,
};

static ur_mesh_hal_module_t linuxhost_ur_ble_module = {
    .base.name = "linuxhost_ur_ble_module",
    .base.priv_dev = &ble_priv,
    .type = MEDIA_TYPE_BLE,
    .ur_mesh_hal_init = linuxhost_ur_init,
    .ur_mesh_hal_send_ucast_request = linuxhost_ur_send_ucast,
    .ur_mesh_hal_send_bcast_request = linuxhost_ur_send_bcast,
    .ur_mesh_hal_register_receiver = linuxhost_ur_set_rxcb,
    .ur_mesh_hal_get_bcast_mtu = linuxhost_ur_get_b_mtu,
    .ur_mesh_hal_get_ucast_mtu = linuxhost_ur_get_u_mtu,
    .ur_mesh_hal_get_mac_address = linuxhost_ur_get_mac_address,
    .ur_mesh_hal_set_channel = linuxhost_ur_hal_set_channel,
    .ur_mesh_hal_get_chnlist = linuxhost_ur_get_channel_list,
    .ur_mesh_hal_get_channel = linuxhost_ur_get_channel,
};

int csp_get_args(const char ***pargv);
void linuxhost_hal_urmesh_register(void)
{
    int i, argc;
    const char **argv;

    argc = csp_get_args(&argv);
    for (i=0;i<argc;i++) {
        if (strcmp(argv[i], "--mesh-ifs") != 0)
            continue;

        int bitmap = atoi(argv[i+1]);
        if (bitmap & (1 << IF_WIFI))
            hal_umesh_register_module(&linuxhost_ur_wifi_module);
        if (bitmap & (1 << IF_BLE))
            hal_umesh_register_module(&linuxhost_ur_ble_module);

        return;
    }

    hal_umesh_register_module(&linuxhost_ur_wifi_module);

}
