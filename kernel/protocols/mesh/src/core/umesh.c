/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
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
#include "core/crypto.h"
#include "core/address_mgmt.h"
#include "ipv6/lwip_adapter.h"
#include "hal/interfaces.h"
#include "tools/cli.h"

typedef struct transmit_frame_s {
    struct pbuf *buf;
    ur_ip6_addr_t dest;
} transmit_frame_t;

typedef struct urmesh_state_s {
    mm_cb_t                mm_cb;
    ur_netif_ip6_address_t ucast_address[IP6_UCAST_ADDR_NUM];
    ur_netif_ip6_address_t mcast_address[IP6_MCAST_ADDR_NUM];
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

static ur_error_t umesh_interface_up(void)
{
    update_ipaddr();

    if (g_um_state.adapter_callback) {
        g_um_state.adapter_callback->interface_up();
    }

    yos_post_event(EV_MESH, CODE_MESH_CONNECTED, 0);
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_API, "mesh interface up\r\n");
    return UR_ERROR_NONE;
}

static ur_error_t umesh_interface_down(void)
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

static void output_message(message_t *message, ur_ip6_addr_t *dest)
{
    ur_error_t error = UR_ERROR_NONE;
    message_info_t *info;
    network_context_t *network;

    info = message->info;
    if ((ur_is_mcast(dest)) && (nd_is_subscribed_mcast(dest))) {
        info->dest.addr.len = SHORT_ADDR_SIZE;
        info->dest.addr.short_addr = BCAST_SID;
        network = get_default_network_context();
        info->dest.netid = mm_get_main_netid(network);
    } else if (ur_is_unique_local(dest)) {
        if (is_sid_address(&dest->m8[8]) == false) {
            info->dest.addr.len = EXT_ADDR_SIZE;
            memcpy(info->dest.addr.addr, &dest->m8[8], sizeof(info->dest.addr.addr));
        } else {
            info->dest.addr.len = SHORT_ADDR_SIZE;
            info->dest.addr.short_addr = ntohs(dest->m16[7]);
            info->dest.netid = ntohs(dest->m16[3]) | ntohs(dest->m16[6]);
        }
    } else {
        error = UR_ERROR_DROP;
        goto exit;
    }

    info->type = MESH_FRAME_TYPE_DATA;
    info->flags = 0;

    error = address_resolve(message);
    if (error == UR_ERROR_NONE) {
        mf_send_message(message);
    }

exit:
    if (error == UR_ERROR_DROP) {
        message_free(message);
    }
}

static void output_ipv4_frame_handler(void *args)
{
    transmit_frame_t *frame = (transmit_frame_t *)args;
    struct pbuf *buf = frame->buf;
    message_t *message;
    message_info_t *info;
    uint16_t sid = frame->dest.m16[7];
    uint8_t append_length;

    append_length = sizeof(mcast_header_t) + 1;
    message = message_alloc(buf->tot_len + append_length, UMESH_1);
    if (message == NULL) {
        goto out;
    }
    message_set_payload_offset(message, -append_length);
    pbuf_copy(message->data, buf);

    info = message->info;
    info->dest.addr.len = SHORT_ADDR_SIZE;
    info->dest.addr.short_addr = sid;
    info->dest.netid = umesh_mm_get_meshnetid(NULL);
    info->type = MESH_FRAME_TYPE_DATA;
    if (sid == BCAST_SID) {
        info->flags |= INSERT_MCAST_FLAG;
    }

    mf_send_message(message);

out:
    pbuf_free(buf);
}

ur_error_t umesh_ipv4_output(struct pbuf *buf, uint16_t sid)
{
    transmit_frame_t *frame;
    ur_error_t error = UR_ERROR_NONE;

    if (umesh_mm_get_device_state() < DEVICE_STATE_LEAF) {
        return UR_ERROR_FAIL;
    }

    frame = (transmit_frame_t *)ur_mem_alloc(sizeof(transmit_frame_t));
    if (frame == NULL) {
        return UR_ERROR_FAIL;
    }

    frame->dest.m16[7] = sid;
    pbuf_ref(buf);
    frame->buf = buf;

    error = umesh_task_schedule_call(output_ipv4_frame_handler, frame);
    if (error != UR_ERROR_NONE) {
        ur_mem_free(frame, sizeof(transmit_frame_t));
        pbuf_free(frame->buf);
    }

    return error;
}

#define dump_data(p, sz) do { \
    uint16_t *p16 = (uint16_t *)(p); \
    int i; \
    printf("%s-%04d: ", __func__, __LINE__); \
    for(i=0;i<(sz)/2;i++) { \
        printf("%04x ", p16[i]); \
        if (i && i % 8 == 0) \
            printf("\n"); \
    } \
    printf("\n"); \
} while(0)

static void output_frame_handler(void *args)
{
    transmit_frame_t *frame = (transmit_frame_t *)args;
    struct pbuf *buf = frame->buf;
    message_t *message = NULL;
    uint8_t append_length;
    ur_error_t error = UR_ERROR_NONE;
    uint16_t ip_hdr_len;
    uint16_t lowpan_hdr_len;
    uint8_t *ip_payload;
    uint8_t *lowpan_payload;

    ip_payload = ur_mem_alloc((UR_IP6_HLEN + UR_UDP_HLEN) * 2);
    if (ip_payload == NULL) {
        error = UR_ERROR_MEM;
        goto out;
    }

    pbuf_copy_partial(buf, ip_payload, UR_IP6_HLEN + UR_UDP_HLEN, 0);
    lowpan_payload = ip_payload + UR_IP6_HLEN + UR_UDP_HLEN;
    error = lp_header_compress(ip_payload, lowpan_payload, &ip_hdr_len, &lowpan_hdr_len);
    if (error != UR_ERROR_NONE) {
        goto out;
    }

    append_length = sizeof(mcast_header_t) + 1;
    message = message_alloc(lowpan_hdr_len + append_length, UMESH_1);
    if (message == NULL) {
        error = UR_ERROR_FAIL;
        goto out;
    }

    pbuf_header(buf, -ip_hdr_len);
    pbuf_chain(message->data, buf);

    message_set_payload_offset(message, -append_length);
    message_copy_from(message, lowpan_payload, lowpan_hdr_len);
    output_message(message, &frame->dest);

out:
    ur_mem_free(ip_payload, (UR_IP6_HLEN + UR_UDP_HLEN) * 2);
    ur_mem_free(frame, sizeof(*frame));
    pbuf_free(buf);
}

ur_error_t umesh_ipv6_output(struct pbuf *buf, const ur_ip6_addr_t *dest)
{
    transmit_frame_t *frame;
    ur_error_t error = UR_ERROR_NONE;

    if (umesh_mm_get_device_state() < DEVICE_STATE_LEAF) {
        return UR_ERROR_FAIL;
    }

    frame = (transmit_frame_t *)ur_mem_alloc(sizeof(transmit_frame_t));
    if (frame == NULL) {
        return UR_ERROR_FAIL;
    }

    memcpy(&frame->dest, dest, sizeof(frame->dest));
    pbuf_ref(buf);
    frame->buf = buf;

    error = umesh_task_schedule_call(output_frame_handler, frame);
    if (error != UR_ERROR_NONE) {
        ur_mem_free(frame, sizeof(transmit_frame_t));
        pbuf_free(frame->buf);
    }

    return error;
}

ur_error_t umesh_input(message_t *message)
{
#if LWIP_IPV6
    ur_error_t error = UR_ERROR_NONE;
    uint8_t *header;
    uint16_t header_size;
    uint16_t lowpan_header_size;
    message_info_t *info;
    message_t *message_header;

    header = ur_mem_alloc(UR_IP6_HLEN + UR_UDP_HLEN);
    if (header == NULL) {
        error = UR_ERROR_FAIL;
        goto exit;
    }
    message_copy_to(message, 0, header, UR_IP6_HLEN + UR_UDP_HLEN);
    info = message->info;

    header_size = message_get_msglen(message);
    if (header_size < MIN_LOWPAN_FRM_SIZE) {
        error = UR_ERROR_FAIL;
        goto handle_non_ipv6;
    }
    error = lp_header_decompress(header, &header_size, &lowpan_header_size,
                                 &info->src, &info->dest);
    if (error != UR_ERROR_NONE) {
        goto handle_non_ipv6;
    }

    message_set_payload_offset(message, -lowpan_header_size);
    message_header = message_alloc(header_size, UMESH_2);
    if (message_header == NULL) {
        error = UR_ERROR_FAIL;
        goto exit;
    }

    message_copy_from(message_header, header, header_size);
    message_concatenate(message_header, message, false);
    message = message_header;

handle_non_ipv6:
    if (g_um_state.adapter_callback) {
        g_um_state.adapter_callback->input(message->data);
    }

exit:
    message_free((message_t *)message);

    ur_mem_free(header, UR_IP6_HLEN + UR_UDP_HLEN);
    return error;
#else
    if (g_um_state.adapter_callback) {
        g_um_state.adapter_callback->input(message->data);
    }
    message_free((message_t *)message);
    return UR_ERROR_NONE;
#endif
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

ur_error_t umesh_init(node_mode_t mode)
{
    if (g_um_state.initialized) {
        return UR_ERROR_NONE;
    }

    umesh_mem_init();
    hal_umesh_init();
    g_um_state.mm_cb.interface_up = umesh_interface_up;
    g_um_state.mm_cb.interface_down = umesh_interface_down;
    ur_adapter_interface_init();
    ur_router_register_module();
    interface_init();

    umesh_mm_init(mode);
    neighbors_init();
    mf_init();
    nd_init();
    message_stats_reset();
    g_um_state.initialized = true;
    mesh_cli_init();

#ifdef CONFIG_YOS_DDA
    parse_args();
#endif
    return UR_ERROR_NONE;
}

bool umesh_is_initialized(void)
{
    return g_um_state.initialized;
}

ur_error_t umesh_start()
{
    umesh_hal_module_t *wifi_hal = NULL;
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

    yos_post_event(EV_MESH, CODE_MESH_STARTED, 0);

    return UR_ERROR_NONE;
}

ur_error_t umesh_stop(void)
{
    umesh_hal_module_t *wifi_hal = NULL;

    if (g_um_state.started == false) {
        return UR_ERROR_NONE;
    }

    g_um_state.started = false;
    wifi_hal = hal_umesh_get_default_module();
    if (wifi_hal) {
        hal_umesh_disable(wifi_hal);
    }

    lp_stop();
    umesh_mm_stop();
    interface_stop();
    return UR_ERROR_NONE;
}

/* per device APIs */
uint8_t umesh_get_device_state(void)
{
    return (uint8_t)umesh_mm_get_device_state();
}

ur_error_t umesh_register_callback(ur_adapter_callback_t *callback)
{
    g_um_state.adapter_callback = callback;
    return UR_ERROR_NONE;
}

uint8_t umesh_get_mode(void)
{
    return (uint8_t)umesh_mm_get_mode();
}

ur_error_t umesh_set_mode(uint8_t mode)
{
    return umesh_mm_set_mode(mode);
}

int8_t umesh_get_seclevel(void)
{
    return umesh_mm_get_seclevel();
}

ur_error_t umesh_set_seclevel(int8_t level)
{
    return umesh_mm_set_seclevel(level);
}

/* per network APIs */
const mac_address_t *umesh_net_get_mac_address(umesh_net_index_t nettype)
{
    return umesh_mm_get_mac_address();
}

uint16_t umesh_net_get_meshnetid(umesh_net_index_t nettype)
{
    return umesh_mm_get_meshnetid(NULL);
}

uint16_t umesh_net_get_meshnetsize(umesh_net_index_t nettype)
{
    return umesh_mm_get_meshnetsize();
}

uint16_t umesh_net_get_sid(umesh_net_index_t nettype)
{
    return umesh_mm_get_local_sid();
}

bool umesh_is_mcast_subscribed(const ur_ip6_addr_t *addr)
{
    return nd_is_subscribed_mcast(addr);
}

const ur_netif_ip6_address_t *umesh_get_ucast_addr(void)
{
    return g_um_state.ucast_address;
}

const ur_netif_ip6_address_t *umesh_get_mcast_addr(void)
{
    return g_um_state.mcast_address;
}

ur_error_t umesh_resolve_dest(const ur_ip6_addr_t *dest, ur_addr_t *dest_addr)
{
    return mf_resolve_dest(dest, dest_addr);
}

#ifdef CONFIG_YOS_MESH_DEBUG
bool umesh_is_whitelist_enabled(void)
{
    return is_whitelist_enabled();
}

void umesh_enable_whitelist(void)
{
    whitelist_enable();
}

void umesh_disable_whitelist(void)
{
    whitelist_disable();
}

const whitelist_entry_t *umesh_get_whitelist_entries(void)
{
    return whitelist_get_entries();
}

ur_error_t umesh_add_whitelist(const mac_address_t *address)
{
    whitelist_entry_t *entry;

    entry = whitelist_add(address);
    if (entry) {
        return UR_ERROR_NONE;
    }
    return UR_ERROR_MEM;
}

ur_error_t umesh_add_whitelist_rssi(const mac_address_t *address, int8_t rssi)
{
    whitelist_entry_t *entry;

    entry = whitelist_add(address);
    if (entry == NULL) {
        return UR_ERROR_MEM;
    }
    whitelist_set_constant_rssi(entry, rssi);
    return UR_ERROR_NONE;
}

void umesh_remove_whitelist(const mac_address_t *address)
{
    whitelist_remove(address);
}

void umesh_clear_whitelist(void)
{
    whitelist_clear();
}
#endif

void umesh_get_channel(channel_t *channel)
{
    umesh_hal_module_t   *ur_wifi_hal = NULL;

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

const ur_link_stats_t *umesh_get_link_stats(media_type_t type)
{
    hal_context_t *hal;

    hal = get_hal_context(type);
    return mf_get_stats(hal);
}

const frame_stats_t *umesh_get_hal_stats(media_type_t type)
{
    hal_context_t *hal;

    hal = get_hal_context(type);
    if (hal == NULL) {
        return NULL;
    }

    return hal_umesh_get_stats(hal->module);
}

const ur_message_stats_t *umesh_get_message_stats(void)
{
    return message_get_stats();
}

const ur_mem_stats_t *umesh_get_mem_stats(void)
{
    return ur_mem_get_stats();
}

slist_t *umesh_get_hals(void)
{
    return get_hal_contexts();
}

slist_t *umesh_get_networks(void)
{
    return get_network_contexts();
}
