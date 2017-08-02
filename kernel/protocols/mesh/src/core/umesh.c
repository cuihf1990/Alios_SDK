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
#include <yos/framework.h>

#include "hal/base.h"
#include "umesh.h"
#include "umesh_hal.h"
#include "umesh_utils.h"
#include "core/mcast.h"
#include "core/link_mgmt.h"
#include "core/lowpan6.h"
#include "core/router_mgr.h"
#include "core/network_data.h"
#include "core/mesh_mgmt.h"
#include "core/mesh_forwarder.h"
#include "core/network_data.h"
#include "core/mesh_forwarder.h"
#include "core/master_key.h"
#include "core/crypto.h"
#include "core/address_mgmt.h"
#include "ipv6/lwip_adapter.h"
#include "hal/interfaces.h"
#include "tools/cli.h"

typedef struct transmit_frame_s {
    message_t     *message;
    ur_ip6_addr_t dest;
} transmit_frame_t;

typedef struct urmesh_state_s {
    mm_cb_t                mm_cb;
    ur_netif_ip6_address_t ucast_address[IP6_UCAST_ADDR_NUM];
    ur_netif_ip6_address_t mcast_address[IP6_MCAST_ADDR_NUM];
    nd_updater_t           network_data_updater;
    ur_adapter_callback_t  *adapter_callback;
    bool                   initialized;
    bool                   started;
} urmesh_state_t;

static urmesh_state_t g_um_state = {.initialized = false , .started = false};

static void update_ipaddr(void)
{
    const ur_ip6_addr_t *mcast;
    uint32_t addr;
    network_context_t *network;

    network = get_default_network_context();
    memset(g_um_state.ucast_address[0].addr.m8, 0,
           sizeof(g_um_state.ucast_address[0].addr.m8));
    g_um_state.ucast_address[0].addr.m32[0] = htonl(0xfc000000);
    g_um_state.ucast_address[0].addr.m32[1] = htonl(nd_get_stable_meshnetid());
    addr = (get_sub_netid(network->meshnetid) << 16) | umesh_mm_get_local_sid();
    g_um_state.ucast_address[0].addr.m32[3] = htonl(addr);
    g_um_state.ucast_address[0].prefix_length = 64;

    g_um_state.ucast_address[0].next = &g_um_state.ucast_address[1];
    memset(g_um_state.ucast_address[1].addr.m8, 0,
           sizeof(g_um_state.ucast_address[1].addr.m8));
    g_um_state.ucast_address[1].addr.m32[0] = htonl(0xfc000000);
    g_um_state.ucast_address[1].addr.m32[1] = htonl(nd_get_stable_meshnetid());
    memcpy(&g_um_state.ucast_address[1].addr.m8[8], umesh_mm_get_local_ueid(), 8);
    g_um_state.ucast_address[1].prefix_length = 64;

    mcast = nd_get_subscribed_mcast();
    memcpy(&g_um_state.mcast_address[0].addr, mcast,
           sizeof(g_um_state.mcast_address[0].addr));
    g_um_state.mcast_address[0].prefix_length = 64;
}

static void network_data_update_handler(bool stable)
{
}

static ur_error_t ur_mesh_interface_up(void)
{
    update_ipaddr();

    if (g_um_state.adapter_callback) {
        g_um_state.adapter_callback->interface_up();
    }

    yos_post_event(EV_MESH, CODE_MESH_CONNECTED, 0);
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_API, "mesh interface up\r\n");
    return UR_ERROR_NONE;
}

static ur_error_t ur_mesh_interface_down(void)
{
    if (g_um_state.adapter_callback) {
        g_um_state.adapter_callback->interface_down();
    }
    yos_post_event(EV_MESH, CODE_MESH_DISCONNECTED, 0);
    return UR_ERROR_NONE;
}

static inline bool is_sid_address(const uint8_t *addr)
{
    uint8_t index;

    for (index = 0; index < 5; index++) {
        if (addr[index] != 0) {
            break;
        }
    }
    if (index == 5) {
        return true;
    }
    return false;
}

static void output_message_handler(void *args)
{
    ur_error_t error = UR_ERROR_NONE;
    message_info_t *info;
    transmit_frame_t *frame = (transmit_frame_t *)args;
    network_context_t *network;

    info = frame->message->info;
    if ((ur_is_mcast(&frame->dest)) && (nd_is_subscribed_mcast(&frame->dest))) {
        info->dest.addr.len = SHORT_ADDR_SIZE;
        info->dest.addr.short_addr = BCAST_SID;
        network = get_default_network_context();
        info->dest.netid = mm_get_main_netid(network);
    } else if (ur_is_unique_local(&frame->dest)) {
        if (is_sid_address(&frame->dest.m8[8]) == false) {
            info->dest.addr.len = EXT_ADDR_SIZE;
            memcpy(info->dest.addr.addr, &frame->dest.m8[8], sizeof(info->dest.addr.addr));
        } else {
            info->dest.addr.len = SHORT_ADDR_SIZE;
            info->dest.addr.short_addr = ntohs(frame->dest.m16[7]);
            info->dest.netid = ntohs(frame->dest.m16[3]) | ntohs(frame->dest.m16[6]);
        }
    } else {
        error = UR_ERROR_DROP;
        goto exit;
    }

    info->type = MESH_FRAME_TYPE_DATA;
    info->flags = 0;

    error = address_resolve(frame->message);
    if (error == UR_ERROR_NONE) {
        mf_send_message(frame->message);
    }

exit:
    if (error == UR_ERROR_DROP) {
        message_free(frame->message);
    }
    ur_mem_free(frame, sizeof(transmit_frame_t));
}

ur_error_t ur_mesh_ipv6_output(umessage_t *message, const ur_ip6_addr_t *dest)
{
    transmit_frame_t *frame;
    uint8_t append_length;
    ur_error_t error = UR_ERROR_NONE;
    uint8_t ip_hdr_len;
    uint8_t lowpan_hdr_len;
    uint8_t *ip_payload;
    uint8_t *lowpan_payload;
    int16_t offset;

    if (umesh_mm_get_device_state() < DEVICE_STATE_LEAF) {
        return UR_ERROR_FAIL;
    }

    ip_payload = ur_mem_alloc((UR_IP6_HLEN + UR_UDP_HLEN) * 2);
    if (ip_payload == NULL) {
        return UR_ERROR_MEM;
    }
    if (message_get_msglen((message_t *)message) >= (UR_IP6_HLEN + UR_UDP_HLEN)) {
        message_copy_to((message_t *)message, 0, ip_payload, UR_IP6_HLEN + UR_UDP_HLEN);
    } else if (message_get_msglen((message_t *)message) >= UR_IP6_HLEN) {
        message_copy_to((message_t *)message, 0, ip_payload, UR_IP6_HLEN);
    }
    lowpan_payload = ip_payload + UR_IP6_HLEN + UR_UDP_HLEN;
    lp_header_compress(ip_payload, lowpan_payload, &ip_hdr_len, &lowpan_hdr_len);
    offset = ip_hdr_len - lowpan_hdr_len;

    frame = (transmit_frame_t *)ur_mem_alloc(sizeof(transmit_frame_t));
    if (frame == NULL) {
        ur_mem_free(ip_payload, (UR_IP6_HLEN + UR_UDP_HLEN) * 2);
        return UR_ERROR_FAIL;
    }
    append_length = sizeof(mcast_header_t) + 2;
    frame->message = message_alloc(((message_t *)message)->data->tot_len +
                                   append_length - offset, UMESH_1);
    if (frame->message == NULL) {
        ur_mem_free(frame, sizeof(transmit_frame_t));
        ur_mem_free(ip_payload, (UR_IP6_HLEN + UR_UDP_HLEN) * 2);
        return UR_ERROR_FAIL;
    }

    message_set_payload_offset(frame->message, -(append_length + lowpan_hdr_len));
    message_set_payload_offset((message_t *)message, -ip_hdr_len);
    message_copy(frame->message, (message_t *)message);
    message_set_payload_offset(frame->message, lowpan_hdr_len);
    message_copy_from(frame->message, lowpan_payload, lowpan_hdr_len);

    memcpy(&frame->dest, dest, sizeof(frame->dest));
    error = umesh_task_schedule_call(output_message_handler, frame);
    if (error != UR_ERROR_NONE) {
        message_free(frame->message);
        ur_mem_free(frame, sizeof(transmit_frame_t));
    }

    ur_mem_free(ip_payload, (UR_IP6_HLEN + UR_UDP_HLEN) * 2);
    return UR_ERROR_NONE;
}

static void input_message_handler(void *args)
{
    if (g_um_state.adapter_callback) {
        g_um_state.adapter_callback->input((message_t *)args);
    } else {
        message_free((message_t *)args);
    }
}

ur_error_t ur_mesh_input(umessage_t *message)
{
    ur_error_t error = UR_ERROR_FAIL;
    message_t *dec_message;

    dec_message = lp_header_decompress((message_t *)message);
    if (dec_message == NULL) {
        message_free((message_t *)message);
        return error;
    }

    if (g_um_state.adapter_callback) {
        error = umesh_task_schedule_call(input_message_handler, dec_message);
    }
    if (error != UR_ERROR_NONE) {
        message_free(dec_message);
    }

    return error;
}

ur_error_t umesh_send_raw_data(ur_addr_t *dest, ur_addr_t *dest2,
                               uint8_t *payload, uint8_t length)
{
    ur_error_t        error = UR_ERROR_NONE;
    network_context_t *network;

    network = get_default_network_context();
    error = send_raw_data(network, dest, dest2, payload, length);
    return error;
}

ur_error_t umesh_raw_data_receiver(ur_addr_t *src,
                                   uint8_t *payload, uint8_t length)
{
    return UR_ERROR_NONE;
}

ur_error_t umesh_register_raw_data_receiver(umesh_raw_data_received receiver)
{
    return UR_ERROR_NONE;
}

#ifdef CONFIG_YOS_DDA
int csp_get_args(const char ***pargv);
static void parse_args(void)
{
    int i, argc;
    const char **argv;
    argc = csp_get_args(&argv);
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            g_cli_silent = 1;
            continue;
        }

        if (strcmp(argv[i], "--mesh-log") == 0) {
            ur_log_set_level(str2lvl(argv[i + 1]));

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "--mesh-mode") == 0) {
            int mode = atoi(argv[i + 1]);
            umesh_mm_set_mode(mode);

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "--mesh-router") == 0) {
            int id = atoi(argv[i + 1]);
            ur_router_set_default_router(id);

            i += 1;
            continue;
        }
    }
}
#endif

ur_error_t ur_mesh_init(node_mode_t mode)
{
    if (g_um_state.initialized) {
        return UR_ERROR_NONE;
    }

    hal_umesh_init();
    g_um_state.mm_cb.interface_up = ur_mesh_interface_up;
    g_um_state.mm_cb.interface_down = ur_mesh_interface_down;
    ur_adapter_interface_init();
    ur_router_register_module();
    interface_init();

    umesh_mm_init(mode);
    neighbors_init();
    mesh_cli_init();
    mf_init();
    nd_init();
    message_stats_reset();
    g_um_state.initialized = true;
    register_raw_data_receiver(umesh_raw_data_receiver);

#ifdef CONFIG_YOS_DDA
    parse_args();
#endif
    return UR_ERROR_NONE;
}

bool ur_mesh_is_initialized(void)
{
    return g_um_state.initialized;
}

ur_error_t ur_mesh_start()
{
    ur_mesh_hal_module_t *wifi_hal = NULL;
    umesh_extnetid_t extnetid;
    int extnetid_len = 6;

    if (g_um_state.started) {
        return UR_ERROR_NONE;
    }

    g_um_state.started = true;


    interface_start();
    umesh_mm_start(&g_um_state.mm_cb);
    lp_start();

    if (yos_kv_get("extnetid", extnetid.netid, &extnetid_len) == 0) {
        extnetid.len = extnetid_len;
        umesh_set_extnetid(&extnetid);
    }

    wifi_hal = hal_umesh_get_default_module();
    if (wifi_hal) {
        hal_umesh_enable(wifi_hal);
    }
    g_um_state.network_data_updater.handler = network_data_update_handler;
    nd_register_update_handler(&g_um_state.network_data_updater);

    yos_post_event(EV_MESH, CODE_MESH_STARTED, 0);

    return UR_ERROR_NONE;
}

ur_error_t ur_mesh_stop(void)
{
    ur_mesh_hal_module_t *wifi_hal = NULL;

    if (g_um_state.started == false) {
        return UR_ERROR_NONE;
    }

    g_um_state.started = false;
    wifi_hal = hal_umesh_get_default_module();
    if (wifi_hal) {
        hal_umesh_disable(wifi_hal);
    }

    nd_unregister_update_handler(&g_um_state.network_data_updater);

    lp_stop();
    umesh_mm_stop();
    interface_stop();
    return UR_ERROR_NONE;
}

/* per device APIs */
uint8_t ur_mesh_get_device_state(void)
{
    return (uint8_t)umesh_mm_get_device_state();
}

ur_error_t ur_mesh_register_callback(ur_adapter_callback_t *callback)
{
    g_um_state.adapter_callback = callback;
    return UR_ERROR_NONE;
}

uint8_t ur_mesh_get_mode(void)
{
    return (uint8_t)umesh_mm_get_mode();
}

ur_error_t ur_mesh_set_mode(uint8_t mode)
{
    return umesh_mm_set_mode(mode);
}

int8_t ur_mesh_get_seclevel(void)
{
    return umesh_mm_get_seclevel();
}

ur_error_t ur_mesh_set_seclevel(int8_t level)
{
    return umesh_mm_set_seclevel(level);
}

/* per network APIs */
const mac_address_t *ur_mesh_net_get_mac_address(ur_mesh_net_index_t nettype)
{
    return umesh_mm_get_mac_address();
}

uint16_t ur_mesh_net_get_meshnetid(ur_mesh_net_index_t nettype)
{
    return umesh_mm_get_meshnetid(NULL);
}

void ur_mesh_net_set_meshnetid(ur_mesh_net_index_t nettype, uint16_t meshnetid)
{
    umesh_mm_set_meshnetid(NULL, meshnetid);
}

uint16_t ur_mesh_net_get_meshnetsize(ur_mesh_net_index_t nettype)
{
    return umesh_mm_get_meshnetsize();
}

uint16_t ur_mesh_net_get_sid(ur_mesh_net_index_t nettype)
{
    return umesh_mm_get_local_sid();
}

bool ur_mesh_is_mcast_subscribed(const ur_ip6_addr_t *addr)
{
    return nd_is_subscribed_mcast(addr);
}

const ur_netif_ip6_address_t *ur_mesh_get_ucast_addr(void)
{
    return g_um_state.ucast_address;
}

const ur_netif_ip6_address_t *ur_mesh_get_mcast_addr(void)
{
    return g_um_state.mcast_address;
}

ur_error_t ur_mesh_resolve_dest(const ur_ip6_addr_t *dest, ur_addr_t *dest_addr)
{
    return mf_resolve_dest(dest, dest_addr);
}

bool ur_mesh_is_whitelist_enabled(void)
{
    return is_whitelist_enabled();
}

void ur_mesh_enable_whitelist(void)
{
    whitelist_enable();
}

void ur_mesh_disable_whitelist(void)
{
    whitelist_disable();
}

const whitelist_entry_t *ur_mesh_get_whitelist_entries(void)
{
    return whitelist_get_entries();
}

ur_error_t ur_mesh_add_whitelist(const mac_address_t *address)
{
    whitelist_entry_t *entry;

    entry = whitelist_add(address);
    if (entry) {
        return UR_ERROR_NONE;
    }
    return UR_ERROR_MEM;
}

ur_error_t ur_mesh_add_whitelist_rssi(const mac_address_t *address, int8_t rssi)
{
    whitelist_entry_t *entry;

    entry = whitelist_add(address);
    if (entry == NULL) {
        return UR_ERROR_MEM;
    }
    whitelist_set_constant_rssi(entry, rssi);
    return UR_ERROR_NONE;
}

void ur_mesh_remove_whitelist(const mac_address_t *address)
{
    whitelist_remove(address);
}

void ur_mesh_clear_whitelist(void)
{
    whitelist_clear();
}

void ur_mesh_get_channel(channel_t *channel)
{
    ur_mesh_hal_module_t   *ur_wifi_hal = NULL;

    if (channel) {
        ur_wifi_hal = hal_umesh_get_default_module();

        channel->wifi_channel = (uint16_t)hal_umesh_get_channel( ur_wifi_hal);
        channel->channel = channel->wifi_channel;
        channel->hal_ucast_channel = (uint16_t)hal_umesh_get_channel(ur_wifi_hal);
        channel->hal_bcast_channel = (uint16_t)hal_umesh_get_channel(ur_wifi_hal);
    }
}

void umesh_get_extnetid(umesh_extnetid_t *extnetid)
{
    if (extnetid == NULL) {
        return;
    }
    umesh_mm_get_extnetid(extnetid);
}

ur_error_t umesh_set_extnetid(const umesh_extnetid_t *extnetid)
{
    if (extnetid == NULL) {
        return UR_ERROR_FAIL;
    }

    return umesh_mm_set_extnetid(extnetid);
}

const ur_link_stats_t *ur_mesh_get_link_stats(media_type_t type)
{
    hal_context_t *hal;

    hal = get_hal_context(type);
    return mf_get_stats(hal);
}

const frame_stats_t *ur_mesh_get_hal_stats(media_type_t type)
{
    hal_context_t *hal;

    hal = get_hal_context(type);
    if (hal == NULL) {
        return NULL;
    }

    return hal_umesh_get_stats(hal->module);
}

const ur_message_stats_t *ur_mesh_get_message_stats(void)
{
    return message_get_stats();
}

const ur_mem_stats_t *ur_mesh_get_mem_stats(void)
{
    return ur_mem_get_stats();
}

slist_t *ur_mesh_get_hals(void)
{
    return get_hal_contexts();
}

slist_t *ur_mesh_get_networks(void)
{
    return get_network_contexts();
}
