/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>
#include <aos/aos.h>

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
#include "ip/lwip_adapter.h"
#include "hal/interfaces.h"
#include "tools/cli.h"

enum {
    TRANSMIT_RAW_FRAME = 0,
    TRANSMIT_IP6_FRAME = 1,
    TRANSMIT_IP4_FRAME = 2,
};

typedef struct transmit_frame_s {
    struct pbuf *buf;
    ur_addr_t dest;
    uint8_t type;
} transmit_frame_t;

typedef struct urmesh_state_s {
    mm_cb_t mm_cb;
    ur_netif_ip6_address_t ucast_address[IP6_UCAST_ADDR_NUM];
    ur_netif_ip6_address_t mcast_address[IP6_MCAST_ADDR_NUM];
    slist_t adapter_callback;
    bool initialized;
    bool started;
} urmesh_state_t;

static urmesh_state_t g_um_state = {.initialized = false , .started = false};

static void update_ipaddr(void)
{
#if LWIP_IPV6
    const ur_ip6_addr_t *mcast;
    uint32_t addr;
    network_context_t *network;

    network = get_default_network_context();
    memset(g_um_state.ucast_address[0].addr.ip6_addr.m8, 0,
           sizeof(g_um_state.ucast_address[0].addr.ip6_addr.m8));
    g_um_state.ucast_address[0].addr.ip6_addr.m32[0] = htonl(0xfc000000);
    g_um_state.ucast_address[0].addr.ip6_addr.m32[1] = htonl(nd_get_stable_meshnetid());
    addr = (get_sub_netid(network->meshnetid) << 16) | umesh_mm_get_local_sid();
    g_um_state.ucast_address[0].addr.ip6_addr.m32[3] = htonl(addr);
    g_um_state.ucast_address[0].prefix_length = 64;

    g_um_state.ucast_address[0].next = &g_um_state.ucast_address[1];
    memset(g_um_state.ucast_address[1].addr.ip6_addr.m8, 0,
           sizeof(g_um_state.ucast_address[1].addr.ip6_addr.m8));
    g_um_state.ucast_address[1].addr.ip6_addr.m32[0] = htonl(0xfc000000);
    g_um_state.ucast_address[1].addr.ip6_addr.m32[1] = htonl(nd_get_stable_meshnetid());
    memcpy(&g_um_state.ucast_address[1].addr.ip6_addr.m8[8], umesh_mm_get_local_ueid(), 8);
    g_um_state.ucast_address[1].prefix_length = 64;

    mcast = nd_get_subscribed_mcast();
    memcpy(&g_um_state.mcast_address[0].addr.ip6_addr, mcast,
           sizeof(g_um_state.mcast_address[0].addr.ip6_addr));
    g_um_state.mcast_address[0].prefix_length = 64;
#else
    uint16_t sid;

    sid = umesh_mm_get_local_sid() + 2;
    g_um_state.ucast_address[0].addr.ip4_addr.m8[0] = 10;
    g_um_state.ucast_address[0].addr.ip4_addr.m8[1] = 0;
    g_um_state.ucast_address[0].addr.ip4_addr.m8[2] = sid >> 8;
    g_um_state.ucast_address[0].addr.ip4_addr.m8[3] = sid & 0xff;

    g_um_state.mcast_address[0].addr.ip4_addr.m8[0] = 224;
    g_um_state.mcast_address[0].addr.ip4_addr.m8[1] = 0;
    g_um_state.mcast_address[0].addr.ip4_addr.m8[2] = 0;
    g_um_state.mcast_address[0].addr.ip4_addr.m8[3] = 252;
#endif
}

static ur_error_t umesh_interface_up(void)
{
    ur_adapter_callback_t *callback;

    update_ipaddr();

    slist_for_each_entry(&g_um_state.adapter_callback, callback,
                         ur_adapter_callback_t, next) {
        callback->interface_up();
    }

    aos_post_event(EV_MESH, CODE_MESH_CONNECTED, 0);
    MESH_LOG_DEBUG("mesh interface up");

    return UR_ERROR_NONE;
}

static ur_error_t umesh_interface_down(void)
{
    ur_adapter_callback_t *callback;

    slist_for_each_entry(&g_um_state.adapter_callback, callback,
                         ur_adapter_callback_t, next) {
        callback->interface_down();
    }

    aos_post_event(EV_MESH, CODE_MESH_DISCONNECTED, 0);
    MESH_LOG_DEBUG("mesh interface up");
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
    uint8_t *ip_payload = NULL;
    uint8_t *lowpan_payload = NULL;
    message_info_t *info;
    uint16_t frame_len;

    append_length = sizeof(mcast_header_t) + 1;
    frame_len = buf->tot_len + append_length;
    if (frame->type == TRANSMIT_IP6_FRAME) {
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
        frame_len = lowpan_hdr_len + append_length;
    }

    message = message_alloc(frame_len, UMESH_1);
    if (message == NULL) {
        error = UR_ERROR_FAIL;
        goto out;
    }
    message_set_payload_offset(message, -append_length);
    if (lowpan_payload) {
        pbuf_header(buf, -ip_hdr_len);
        pbuf_chain(message->data, buf);
        message_copy_from(message, lowpan_payload, lowpan_hdr_len);
    } else {
        pbuf_copy(message->data, buf);
    }

    info = message->info;
    memcpy(&info->dest, &frame->dest, sizeof(frame->dest));
    info->type = MESH_FRAME_TYPE_DATA;
    info->flags = 0;

    error = address_resolve(message);
    if (error == UR_ERROR_NONE) {
        mf_send_message(message);
    } else if (error == UR_ERROR_DROP) {
        message_free(message);
    }

out:
    ur_mem_free(ip_payload, (UR_IP6_HLEN + UR_UDP_HLEN) * 2);
    ur_mem_free(frame, sizeof(*frame));
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

    frame->dest.addr.len = SHORT_ADDR_SIZE;
    frame->dest.addr.short_addr = sid;
    frame->dest.netid = umesh_mm_get_meshnetid(NULL);
    pbuf_ref(buf);
    frame->buf = buf;
    frame->type = TRANSMIT_IP4_FRAME;

    error = umesh_task_schedule_call(output_frame_handler, frame);
    if (error != UR_ERROR_NONE) {
        pbuf_free(frame->buf);
        ur_mem_free(frame, sizeof(transmit_frame_t));
    }

    return error;
}

ur_error_t umesh_ipv6_output(struct pbuf *buf, const ur_ip6_addr_t *dest)
{
    transmit_frame_t *frame;
    ur_error_t error = UR_ERROR_NONE;
    network_context_t *network;

    if (umesh_mm_get_device_state() < DEVICE_STATE_LEAF) {
        return UR_ERROR_FAIL;
    }

    frame = (transmit_frame_t *)ur_mem_alloc(sizeof(transmit_frame_t));
    if (frame == NULL) {
        return UR_ERROR_FAIL;
    }

    if ((ip6_is_mcast(dest)) && (nd_is_subscribed_mcast(dest))) {
        frame->dest.addr.len = SHORT_ADDR_SIZE;
        frame->dest.addr.short_addr = BCAST_SID;
        network = get_default_network_context();
        frame->dest.netid = mm_get_main_netid(network);
    } else if (ip6_is_unique_local(dest)) {
        if (is_sid_address(&dest->m8[8]) == false) {
            frame->dest.addr.len = EXT_ADDR_SIZE;
            memcpy(frame->dest.addr.addr, &dest->m8[8], sizeof(frame->dest.addr.addr));
        } else {
            frame->dest.addr.len = SHORT_ADDR_SIZE;
            frame->dest.addr.short_addr = ntohs(dest->m16[7]);
            frame->dest.netid = ntohs(dest->m16[3]) | ntohs(dest->m16[6]);
        }
    } else {
        ur_mem_free(frame, sizeof(transmit_frame_t));
        return UR_ERROR_FAIL;
    }

    pbuf_ref(buf);
    frame->buf = buf;
    frame->type = TRANSMIT_IP6_FRAME;

    error = umesh_task_schedule_call(output_frame_handler, frame);
    if (error != UR_ERROR_NONE) {
        ur_mem_free(frame, sizeof(transmit_frame_t));
        pbuf_free(frame->buf);
    }

    return error;
}

ur_error_t umesh_input(message_t *message)
{
    ur_adapter_callback_t *callback;
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
    slist_for_each_entry(&g_um_state.adapter_callback, callback,
                         ur_adapter_callback_t, next) {
        callback->input(message->data);
    }

exit:
    message_free((message_t *)message);

    ur_mem_free(header, UR_IP6_HLEN + UR_UDP_HLEN);
    return error;
#else
    slist_for_each_entry(&g_um_state.adapter_callback, callback,
                         ur_adapter_callback_t, next) {
        callback->input(message->data);
    }
    message_free((message_t *)message);
    return UR_ERROR_NONE;
#endif
}

#ifdef CONFIG_AOS_DDA
#include <stdlib.h>
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
#ifdef CONFIG_AOS_MESH_DEBUG
            ur_log_set_level(str2lvl(argv[i + 1]));
#endif

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
#if (defined CONFIG_NET_LWIP) || (defined CONFIG_AOS_MESH_TAPIF)
    ur_adapter_interface_init();
#endif
    ur_router_register_module();
    interface_init();

    umesh_mm_init(mode);
    neighbors_init();
    mf_init();
    nd_init();
    message_stats_reset();
    g_um_state.initialized = true;
    mesh_cli_init();

#ifdef CONFIG_AOS_DDA
    parse_args();
#endif
    return UR_ERROR_NONE;
}
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_init, "ur_error_t umesh_init(node_mode_t mode)")

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

    if (aos_kv_get("extnetid", extnetid.netid, &extnetid_len) == 0) {
        extnetid.len = extnetid_len;
        umesh_set_extnetid(&extnetid);
    }

    wifi_hal = hal_umesh_get_default_module();
    if (wifi_hal) {
        hal_umesh_enable(wifi_hal);
    }

    aos_post_event(EV_MESH, CODE_MESH_STARTED, 0);

    return UR_ERROR_NONE;
}
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_start, "ur_error_t umesh_start(void)")

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
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_stop, "ur_error_t umesh_stop(void)")

/* per device APIs */
uint8_t umesh_get_device_state(void)
{
    return (uint8_t)umesh_mm_get_device_state();
}
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_device_state, "uint8_t umesh_get_device_state(void)")

ur_error_t umesh_register_callback(ur_adapter_callback_t *callback)
{
    slist_add(&callback->next, &g_um_state.adapter_callback);
    return UR_ERROR_NONE;
}

uint8_t umesh_get_mode(void)
{
    return (uint8_t)umesh_mm_get_mode();
}
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_mode, "uint8_t umesh_get_mode(void)")

ur_error_t umesh_set_mode(uint8_t mode)
{
    return umesh_mm_set_mode(mode);
}
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_set_mode, "ur_error_t umesh_set_mode(uint8_t mode)")

const mac_address_t *umesh_get_mac_address(media_type_t type)
{
    return umesh_mm_get_mac_address();
}
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_mac_address, "const mac_address_t *umesh_get_mac_address(media_type_t type)")

uint16_t umesh_get_meshnetid(void)
{
    return umesh_mm_get_meshnetid(NULL);
}

uint16_t umesh_get_meshnetsize(void)
{
    return umesh_mm_get_meshnetsize();
}

uint16_t umesh_get_sid(void)
{
    return umesh_mm_get_local_sid();
}

slist_t *umesh_get_nbrs(media_type_t type)
{
    hal_context_t *hal;
    slist_t *nbrs = NULL;

    hal = get_hal_context(type);
    if (hal) {
        nbrs = &hal->neighbors_list;
    }
    return nbrs;
}

bool umesh_is_mcast_subscribed(const ur_ip6_addr_t *addr)
{
    return nd_is_subscribed_mcast(addr);
}

const ur_netif_ip6_address_t *umesh_get_ucast_addr(void)
{
    return g_um_state.ucast_address;
}
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_ucast_addr, "const ur_netif_ip6_address_t *umesh_get_ucast_addr(void)")

const ur_netif_ip6_address_t *umesh_get_mcast_addr(void)
{
    return g_um_state.mcast_address;
}
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_mcast_addr, "const ur_netif_ip6_address_t *umesh_get_mcast_addr(void)")

ur_error_t umesh_resolve_dest(const ur_ip6_addr_t *dest, ur_addr_t *dest_addr)
{
    return mf_resolve_dest(dest, dest_addr);
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
