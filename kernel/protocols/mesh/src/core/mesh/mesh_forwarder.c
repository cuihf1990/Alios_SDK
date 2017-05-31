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
#include <stdint.h>
#include <assert.h>

#include "umesh.h"
#include "umesh_hal.h"
#include "core/mesh_mgmt.h"
#include "core/mesh_forwarder.h"
#include "core/router_mgr.h"
#include "core/lowpan6.h"
#include "core/network_data.h"
#include "core/mcast.h"
#include "core/sid_allocator.h"
#include "core/address_mgmt.h"
#include "core/keys_mgr.h"
#include "core/link_mgmt.h"
#include "utilities/logging.h"
#include "utilities/message.h"
#include "utilities/encoding.h"
#include "utilities/mac_whitelist.h"
#include "utilities/timer.h"
#include "ipv6/ip6.h"
#include "hal/interface_context.h"
#include "hal/interfaces.h"
#include "tools/diags.h"

typedef void (*yos_call_t)(void *);
extern int yos_schedule_call(yos_call_t f, void *arg);

typedef struct received_frame_s {
    hal_context_t *hal;
    message_t    *message;
    frame_info_t frame_info;
} received_frame_t;

enum {
    SENDING_TIMEOUT = 5000,
};

enum {
    SENT_SUCCESS = 0,
    SENT_FAIL = -1,
};

static void send_datagram(void *args);
static void handle_datagram(void *message);
static neighbor_t *get_next_node(network_context_t *network, message_info_t *info);
static neighbor_t *get_neighbor(uint8_t type, uint16_t meshnetid,
                                mac_address_t *addr);

static inline bool is_lowpan_iphc(uint8_t control)
{
    return (control & LOWPAN_IPHC_DISPATCH_MASK) == LOWPAN_IPHC_DISPATCH;
}

static inline bool is_uncompressed(uint8_t control)
{
    return control == UNCOMPRESSED_DISPATCH;
}

static inline bool is_mesh_header(uint8_t control)
{
    return (control & MESH_HEADER_DISPATCH_MASK) == MESH_HEADER_DISPATCH;
}

static inline bool is_fragment_header(uint8_t control)
{
    return (control & FRAG_HEADER_DISPATCH_MASK) == FRAG_HEADER_DISPATCH;
}

static inline bool is_mcast_header(uint8_t control)
{
    return (control & MCAST_HEADER_DISPATCH_MASK) == MCAST_HEADER_DISPATCH;
}

static inline bool is_local_ucast_address(message_info_t *info)
{
    mac_address_t *addr;
    bool matched = false;
    network_context_t *network;

    addr = &info->dest.addr;
    switch (addr->len) {
        case SHORT_ADDR_SIZE:
            network = get_network_context_by_meshnetid(info->dest.netid);
            if (network == NULL) {
                return matched;
            }
            if (addr->short_addr == mm_get_local_sid()) {
                matched = true;
            }
            break;
        case EXT_ADDR_SIZE:
            if (memcmp(addr->addr, mm_get_local_ueid(), sizeof(addr->addr)) == 0) {
                matched = true;
            }
            break;
        default:
            matched = false;
    }

    if (matched && info->dest2.addr.len != 0) {
        matched = false;
        memcpy(&info->dest, &info->dest2, sizeof(info->dest));
        info->dest2.addr.len = 0;
    }
    return matched;
}

void message_sent_task(void *args)
{
    hal_context_t *hal = (hal_context_t *)args;
    message_t *message;
    uint16_t msg_length;

    ur_stop_timer(&hal->sending_timer, hal);
    if (hal->send_message == NULL) {
        return;
    }
    message = hal->send_message;
    msg_length = message_get_msglen(message);
    if (hal->frag_info.offset < msg_length) {
        message->frag_offset = hal->frag_info.offset;
    }
    if (hal->frag_info.offset >= msg_length ||
        hal->last_sent != SENT_SUCCESS) {
        message_queue_dequeue(message);
        message_free(message);
        hal->send_message = NULL;
        hal->frag_info.offset = 0;
    }
    send_datagram(hal);
}

static void handle_sent(void *context, frame_t *frame, int error)
{
    hal_context_t *hal = (hal_context_t *)context;

    hal->link_stats.out_frames++;
    if (error != SENT_SUCCESS) {
        error = SENT_FAIL;
        hal->link_stats.out_errors++;
    }
    hal->last_sent = error;
    yos_schedule_call(message_sent_task, hal);
}

static message_t *handle_lowpan_iphc(message_t *message)
{
    return lp_header_decompress(message);
}

static void resolve_message_info(received_frame_t *frame)
{
    message_t *message;
    mesh_header_control_t *control;
    message_info_t *info;
    uint8_t offset;
    uint8_t *buf;
    mesh_short_addr_t *short_addr;
    mesh_ext_addr_t *ext_addr;
    mesh_subnetid_t *subnetid;
    mesh_netid_t *netid;

    message = frame->message;
    info = message->info;
    info->hal_type = frame->hal->module->type;
    info->channel = frame->frame_info.channel;
    info->key_index = frame->frame_info.key_index;
    memcpy(&info->src_mac.addr, &frame->frame_info.peer, sizeof(info->src_mac.addr));
    info->src_mac.netid = BCAST_NETID;
    buf = message_get_payload(message);
    control = (mesh_header_control_t *)buf;
    offset = sizeof(mesh_header_control_t);
    netid = (mesh_netid_t *)(buf + offset);
    info->src.netid = netid->netid;
    offset += sizeof(mesh_netid_t);

    info->type = (control->control[0] & MESH_FRAME_TYPE_MASK) >> MESH_FRAME_TYPE_OFFSET;
    info->hops = (control->control[0] & MESH_HOPS_LEFT_MASK) >> MESH_HOPS_LEFT_OFFSET;
    switch ((control->control[0] & MESH_HEADER_SRC_MASK) >> MESH_HEADER_SRC_OFFSET) {
        case SHORT_ADDR_MODE:
            short_addr = (mesh_short_addr_t *)(buf + offset);
            info->src.addr.len = SHORT_ADDR_SIZE;
            info->src.addr.short_addr = short_addr->addr;
            offset += SHORT_ADDR_SIZE;
            break;
        default:
            info->src.addr.len = 0;
            break;
    }

    switch ((control->control[1] & MESH_HEADER_DESTNETID_MASK) >> MESH_HEADER_DESTNETID_OFFSET) {
        case NO_DEST_NETID:
            info->dest.netid = netid->netid;
            break;
        case BCAST_DEST_NETID:
            info->dest.netid = BCAST_NETID;
            break;
        case SUB_DEST_NETID:
            subnetid = (mesh_subnetid_t *)(buf + offset);
            info->dest.netid = mk_sub_netid(netid->netid, subnetid->netid);
            offset += sizeof(mesh_subnetid_t);
            break;
        case DEST_NETID:
            netid = (mesh_netid_t *)(buf + offset);
            info->dest.netid = netid->netid;
            offset += sizeof(mesh_netid_t);
            break;
        default:
            break;
    }

    switch ((control->control[1] & MESH_HEADER_DEST_MASK) >> MESH_HEADER_DEST_OFFSET) {
        case NO_ADDR_MODE:
            info->dest.addr.len = 0;
            break;
        case SHORT_ADDR_MODE:
            short_addr = (mesh_short_addr_t *)(buf + offset);
            info->dest.addr.len = SHORT_ADDR_SIZE;
            info->dest.addr.short_addr = short_addr->addr;
            offset += sizeof(mesh_short_addr_t);
            break;
        case EXT_ADDR_MODE:
            ext_addr = (mesh_ext_addr_t *)(buf + offset);
            info->dest.addr.len = EXT_ADDR_SIZE;
            memcpy(info->dest.addr.addr, ext_addr->addr, sizeof(ext_addr->addr));
            offset += sizeof(mesh_ext_addr_t);
            break;
        case BCAST_ADDR_MODE:
            info->dest.addr.len = SHORT_ADDR_SIZE;
            info->dest.addr.short_addr = BCAST_SID;
            break;
        default:
            break;
    }

    info->dir = (control->control[1] & MESH_HEADER_DIR_MASK) >> MESH_HEADER_DIR_OFFSET;

    switch ((control->control[1] & MESH_HEADER_DEST2_MASK) >> MESH_HEADER_DEST2_OFFSET) {
        case NO_ADDR_MODE:
            break;
        case SHORT_ADDR_MODE:
            short_addr = (mesh_short_addr_t *)(buf + offset);
            info->dest2.addr.len = SHORT_ADDR_SIZE;
            info->dest2.addr.short_addr = short_addr->addr;
            offset += sizeof(mesh_short_addr_t);
            break;
        case EXT_ADDR_MODE:
            ext_addr = (mesh_ext_addr_t *)(buf + offset);
            info->dest2.addr.len = EXT_ADDR_SIZE;
            memcpy(info->dest2.addr.addr, ext_addr->addr, sizeof(ext_addr->addr));
            offset += sizeof(mesh_ext_addr_t);
            break;
        case BCAST_ADDR_MODE:
            info->dest2.addr.len = SHORT_ADDR_SIZE;
            info->dest2.addr.short_addr = BCAST_SID;
            break;
        default:
            break;
    }

    if (control->control[1] & (ENABLE_SEC << MESH_HEADER_SEC_OFFSET)) {
        info->flags |= ENCRYPT_ENABLE_FLAG;
    }

    info->payload_offset = offset;
}

static uint8_t insert_mesh_header(network_context_t *network,
                                  message_info_t *info)
{
    mesh_header_control_t *control;
    mesh_short_addr_t *short_addr;
    mesh_ext_addr_t *ext_addr;
    mesh_netid_t *netid;
    mesh_subnetid_t *subnetid;
    uint8_t length;
    hal_context_t *hal;
    uint8_t hops;

    hal = network->hal;
    control = (mesh_header_control_t *)hal->frame.data;
    control->control[0] = MESH_HEADER_DISPATCH;
    control->control[1] = 0;
    length = sizeof(mesh_header_control_t);
    if (info->type != MESH_FRAME_TYPE_DATA) {
        control->control[0] |= (MESH_FRAME_TYPE_CMD << MESH_FRAME_TYPE_OFFSET);
    }

    if (info->hops) {
        hops = info->hops;
    } else {
        hops = FORWARD_HOP_LIMIT;
    }
    control->control[0] = (control->control[0] & ~MESH_HOPS_LEFT_MASK) |
                          (hops << MESH_HOPS_LEFT_OFFSET);

    netid = (mesh_netid_t *)(hal->frame.data + length);
    netid->netid = mm_get_meshnetid(network);
    length += sizeof(mesh_netid_t);

    switch (info->src.addr.len) {
        case 0:
            control->control[0] |= (NO_ADDR_MODE << MESH_HEADER_SRC_OFFSET);
            break;
        case SHORT_ADDR_SIZE:
            control->control[0] |= (SHORT_ADDR_MODE << MESH_HEADER_SRC_OFFSET);
            short_addr = (mesh_short_addr_t *)(hal->frame.data + length);
            short_addr->addr = info->src.addr.short_addr;
            length += sizeof(mesh_short_addr_t);
            break;
        case EXT_ADDR_SIZE:
            ext_addr = (mesh_ext_addr_t *)(hal->frame.data + length);
            memcpy(ext_addr->addr, info->src.addr.addr, sizeof(ext_addr->addr));
            control->control[0] |= (EXT_ADDR_MODE << MESH_HEADER_SRC_OFFSET);
            length += sizeof(mesh_ext_addr_t);
            break;
        default:
            return 0;
    }

    if (info->dest.netid == BCAST_NETID) {
        control->control[1] |= (BCAST_DEST_NETID << MESH_HEADER_DESTNETID_OFFSET);
    } else if (info->dest.netid == netid->netid) {
        control->control[1] |= (NO_DEST_NETID << MESH_HEADER_DESTNETID_OFFSET);
    } else if (is_same_mainnet(info->dest.netid, netid->netid)) {
        control->control[1] |= (SUB_DEST_NETID << MESH_HEADER_DESTNETID_OFFSET);
        subnetid = (mesh_subnetid_t *)(hal->frame.data + length);
        subnetid->netid = get_sub_netid(info->dest.netid);
        length += sizeof(mesh_subnetid_t);
    } else {
        control->control[1] |= (DEST_NETID << MESH_HEADER_DESTNETID_OFFSET);
        netid = (mesh_netid_t *)(hal->frame.data + length);
        netid->netid = info->dest.netid;
        length += sizeof(mesh_netid_t);
    }

    if (info->dest.addr.len == SHORT_ADDR_SIZE) {
        if (info->dest.addr.short_addr == BCAST_SID) {
            control->control[1] |= (BCAST_ADDR_MODE << MESH_HEADER_DEST_OFFSET);
        } else {
            control->control[1] |= (SHORT_ADDR_MODE << MESH_HEADER_DEST_OFFSET);
            short_addr = (mesh_short_addr_t *)(hal->frame.data + length);
            short_addr->addr = info->dest.addr.short_addr;
            length += sizeof(mesh_short_addr_t);
        }
    } else {
        return 0;
    }

    switch (info->dest2.addr.len) {
        case 0:
            control->control[1] |= (NO_ADDR_MODE << MESH_HEADER_DEST2_OFFSET);
            break;
        case SHORT_ADDR_SIZE:
            control->control[1] |= (SHORT_ADDR_MODE << MESH_HEADER_DEST2_OFFSET);
            short_addr = (mesh_short_addr_t *)(hal->frame.data + length);
            short_addr->addr = info->dest2.addr.short_addr;
            length += sizeof(mesh_short_addr_t);
            break;
        case EXT_ADDR_SIZE:
            control->control[1] |= (EXT_ADDR_MODE << MESH_HEADER_DEST2_OFFSET);
            ext_addr = (mesh_ext_addr_t *)(hal->frame.data + length);
            memcpy(ext_addr->addr, info->dest2.addr.addr, sizeof(ext_addr->addr));
            length += sizeof(mesh_ext_addr_t);
            break;
        default:
            return 0;
    }

    if (info->flags & ENCRYPT_ENABLE_FLAG) {
        control->control[1] |= (ENABLE_SEC << MESH_HEADER_SEC_OFFSET);
    }

    info->payload_offset = length;
    return length;
}

static void handle_sending_timer(void *args)
{
    hal_context_t *hal = args;

    hal->sending_timer = NULL;
    if (hal->send_message == NULL) {
        return;
    }
    hal->last_sent = SENT_FAIL;
    message_sent_task(hal);
    hal->link_stats.sending_timeouts++;
}

static ur_error_t send_fragment(network_context_t *network, message_t *message)
{
    ur_error_t     error = UR_ERROR_NONE;
    uint16_t       frag_length;
    uint16_t       msg_length;
    frag_header_t  frag_header;
    uint16_t       mtu;
    message_info_t *info;
    uint8_t        header_length = 0;
    uint16_t       append_length = 0;
    hal_context_t  *hal;
    neighbor_t     *next_node = NULL;
    uint8_t        *payload;

    hal = network->hal;
    info = message->info;
    if (info->dest.addr.short_addr == BCAST_SID) {
        mtu = hal_ur_mesh_get_bcast_mtu(network->hal->module);
    } else {
        mtu = hal_ur_mesh_get_ucast_mtu(network->hal->module);
    }

    if (info->dest.addr.len == EXT_ADDR_SIZE ||
        info->dest.addr.short_addr != BCAST_SID) {
        next_node = get_next_node(network, info);
        if (next_node == NULL) {
            return UR_ERROR_DROP;
        }
    }
    if (info->dest.addr.len == EXT_ADDR_SIZE) {
        info->dest.addr.len = SHORT_ADDR_SIZE;
        info->dest.addr.short_addr = next_node->addr.addr.short_addr;
    }

    if (info->flags & INSERT_MESH_HEADER) {
        header_length = insert_mesh_header(network, info);
        if (header_length == 0) {
            return UR_ERROR_DROP;
        }
    }

    header_length = info->payload_offset;
    msg_length = message_get_msglen(message);
    memset(&frag_header, 0, sizeof(frag_header));

    if (message->frag_offset == 0) {
        frag_length = msg_length;
        if (frag_length > (mtu - header_length)) {
            frag_header.dispatch = FRAG_1_DISPATCH;
            frag_header.size     = msg_length;
            *((uint16_t *)&frag_header) = ur_swap16(*((uint16_t *)&frag_header));
            hal->frag_info.tag++;
            frag_header.tag = ur_swap16(hal->frag_info.tag);
            frag_length = (mtu + 1 - sizeof(frag_header_t) - header_length) & 0xff8;
            append_length = sizeof(frag_header) - 1;
        }
    } else {
        frag_header.dispatch = FRAG_N_DISPATCH;
        frag_header.size     = msg_length;
        *((uint16_t *)&frag_header) = ur_swap16(*((uint16_t *)&frag_header));
        frag_header.offset = (uint8_t)(message->frag_offset >> 3);
        frag_header.tag = ur_swap16(hal->frag_info.tag);
        frag_length = (mtu - sizeof(frag_header_t) - header_length) & 0xff8;
        if (frag_length > (msg_length - message->frag_offset)) {
            frag_length = msg_length - message->frag_offset;
        }
        append_length = sizeof(frag_header);
    }

    if (append_length > 0) {
        memcpy(hal->frame.data + header_length, (uint8_t *)&frag_header, append_length);
    }
    payload = hal->frame.data + header_length + append_length;
    message_copy_to(message, message->frag_offset, payload, frag_length);
    hal->frame.len = header_length + append_length + frag_length;
    hal->frame.key_index = info->key_index;

    if (next_node) {
        error = hal_ur_mesh_send_ucast_request(hal->module, &hal->frame, &next_node->mac,
                                               handle_sent, hal);
    } else {
        error = hal_ur_mesh_send_bcast_request(network->hal->module, &hal->frame,
                                               handle_sent, hal);
    }

    if (error != UR_ERROR_NONE) {
        error = UR_ERROR_FAIL;
    }
    hal->frag_info.offset += frag_length;
    return error;
}

static neighbor_t *get_next_node(network_context_t *network, message_info_t *info)
{
    uint16_t   local_sid;
    uint16_t   next_hop;
    neighbor_t *next = NULL;
    bool       same_subnet = false;
    bool       same_net = true;

    local_sid = mm_get_local_sid();
    if (info->dest.addr.len == EXT_ADDR_SIZE) {
        next = get_neighbor_by_mac_addr(&(info->dest.addr));
        return next;
    }

    if (local_sid == BCAST_SID) {
        next = mm_get_attach_candidate(network);
        return next;
    }

    if (is_partial_function_sid(local_sid)) {
        next = mm_get_attach_node(network);
        return next;
    }

    if (mm_get_meshnetid(network) == info->dest.netid) {
        same_subnet = true;
    } else if (is_same_mainnet(info->dest.netid, mm_get_meshnetid(network)) == false) {
        same_net = false;
    }

    if (same_net == true) {
        if (same_subnet == true) {
            next_hop = ur_router_get_next_hop(network, info->dest.addr.short_addr);
            next = get_neighbor_by_sid(network->hal, next_hop, info->dest.netid);
        } else if (is_subnet(network->meshnetid)) {
            next = mm_get_attach_node(network);
        } else {
            next_hop = ur_router_get_next_hop(network, get_leader_sid(info->dest.netid));
            next = get_neighbor_by_sid(network->hal, next_hop, get_main_netid(info->dest.netid));
        }
    } else {
        next = mm_get_attach_candidate(network);
    }

    return next;
}

static void set_dest_info(message_info_t *info, address_cache_t *target)
{
    info->dest.netid = target->meshnetid;
    if (is_partial_function_sid(target->sid)) {
        info->dest2.addr.len = SHORT_ADDR_SIZE;
        info->dest2.addr.short_addr = target->sid;
        info->dir = DIR_UP;
        info->dest.addr.len = SHORT_ADDR_SIZE;
        info->dest.addr.short_addr = target->attach_sid;
    } else {
        info->dest.addr.len = SHORT_ADDR_SIZE;
        info->dest.addr.short_addr = target->sid;
    }
}

static void set_src_info(message_info_t *info)
{
    network_context_t *network;

    if (info->network == NULL) {
        network = get_network_context_by_meshnetid(info->dest.netid);
        if (network == NULL) {
            network = get_default_network_context();
        }
        info->network = network;
    }
    info->src.netid = mm_get_meshnetid(info->network);
    info->src.addr.len = SHORT_ADDR_SIZE;
    info->src.addr.short_addr = mm_get_local_sid();
    info->flags |= INSERT_LOWPAN_FLAG;
}

static void address_resolved_handler(network_context_t *network, address_cache_t *target,
                                     ur_error_t error)
{
    message_t      *message;
    message_info_t *info;
    hal_context_t  *hal;
    bool           matched = false;

    hal = network->hal;
    for_each_message(message, &hal->send_queue[PENDING_QUEUE]) {
        info = message->info;
        if (info->dest.addr.len == SHORT_ADDR_SIZE) {
            if (info->dest.addr.short_addr == target->sid &&
                info->dest.netid == target->meshnetid) {
                matched = true;
            }
        } else if (info->dest.addr.len == EXT_ADDR_SIZE) {
            if (memcmp(target->ueid, info->dest.addr.addr, sizeof(target->ueid)) == 0) {
                matched = true;
            }
        }

        if (matched == false) {
            continue;
        }
        message_queue_dequeue(message);
        if (error == UR_ERROR_NONE) {
            set_dest_info(info, target);
            set_src_info(info);
            network = info->network;
            message_queue_enqueue(&network->hal->send_queue[DATA_QUEUE], message);
            yos_schedule_call(send_datagram, network->hal);
        } else {
            message_free(message);
        }
    }
}

ur_error_t mf_resolve_dest(const ur_ip6_addr_t *dest, ur_addr_t *dest_addr)
{
    if (ur_is_mcast(dest)) {
        return UR_ERROR_FAIL;
    }

    dest_addr->addr.len = SHORT_ADDR_SIZE;
    dest_addr->addr.short_addr = ur_swap16(dest->m16[7]);
    dest_addr->netid = ur_swap16(dest->m16[3]) | ur_swap16(dest->m16[6]);
    return UR_ERROR_NONE;
}

static void set_dest_encrypt_flag(message_info_t *info)
{
    if (mm_get_seclevel() < SEC_LEVEL_1) {
        return;
    }

    if (info->type == MESH_FRAME_TYPE_DATA) {
        info->flags |= ENCRYPT_ENABLE_FLAG;
        info->key_index = MASTER_KEY_INDEX;
        return;
    }

    if (info->type == MESH_FRAME_TYPE_CMD) {
        if (info->command == COMMAND_ADVERTISEMENT ||
            info->command == COMMAND_DISCOVERY_REQUEST ||
            info->command == COMMAND_DISCOVERY_RESPONSE) {
            info->key_index = INVALID_KEY_INDEX;
            return;
        }
        info->flags |= ENCRYPT_ENABLE_FLAG;
        if (info->command == COMMAND_ATTACH_REQUEST ||
            info->command == COMMAND_ATTACH_RESPONSE) {
            info->key_index = MASTER_KEY_INDEX;
        } else {
            info->key_index = GROUP_KEY1_INDEX;
        }
    }
}

static neighbor_t *get_neighbor(uint8_t type, uint16_t meshnetid,
                                mac_address_t *addr)
{
    network_context_t *network;
    neighbor_t *nbr = NULL;

    if (addr->len == SHORT_ADDR_SIZE) {
        if (addr->short_addr == BCAST_NETID) {
            return NULL;
        }
        network = get_network_context_by_meshnetid(meshnetid);
        if (network == NULL) {
            network = get_default_network_context();
        }
        nbr = get_neighbor_by_sid(network->hal, addr->short_addr, meshnetid);
    } else if (addr->len == EXT_ADDR_SIZE) {
        if (type == MESH_FRAME_TYPE_DATA) {
            nbr = get_neighbor_by_ueid(addr->addr);
        } else {
            nbr = get_neighbor_by_mac_addr(addr);
        }
    }

    return nbr;
}

ur_error_t mf_send_message(message_t *message)
{
    ur_error_t        error = UR_ERROR_NONE;
    message_info_t    *info;
    ur_node_id_t      target;
    ur_node_id_t      attach;
    network_context_t *network = NULL;
    uint8_t           query_type = PF_ATTACH_QUERY;
    neighbor_t        *nbr = NULL;
    bool              is_mcast = false;
    bool              need_resolve = false;
    hal_context_t     *hal;

    info = message->info;
    if (is_local_ucast_address(info)) {
        network = get_default_network_context();
        info->src.netid = mm_get_meshnetid(network);
        info->src.addr.len = SHORT_ADDR_SIZE;
        info->src.addr.short_addr = mm_get_local_sid();
        yos_schedule_call(handle_datagram, message);
        return UR_ERROR_NONE;
    }

    info->flags |= INSERT_MESH_HEADER;
    set_dest_encrypt_flag(info);

    nbr = get_neighbor(info->type, info->dest.netid, &info->dest.addr);
    if (info->dest.addr.len == SHORT_ADDR_SIZE) {
        if (nbr == NULL && is_partial_function_sid(info->dest.addr.short_addr)) {
            need_resolve = true;
            target.sid = info->dest.addr.short_addr;
            target.meshnetid = info->dest.netid;
            query_type = PF_ATTACH_QUERY;
        } else if (info->dest.addr.short_addr == BCAST_SID) {
            is_mcast = true;
        }
    } else if (info->dest.addr.len == EXT_ADDR_SIZE &&
               info->type == MESH_FRAME_TYPE_DATA) {
        if (nbr == NULL) {
            need_resolve = true;
            memcpy(target.ueid, &info->dest.addr.addr, sizeof(target.ueid));
            query_type = TARGET_QUERY;
        } else {
            info->dest.netid = nbr->addr.netid;
        }
    }
    if ((info->dest.addr.len != SHORT_ADDR_SIZE && info->dest.addr.len != EXT_ADDR_SIZE) ||
        (info->dest.addr.len == EXT_ADDR_SIZE && nbr == NULL && need_resolve == false)) {
        message_free(message);
        return UR_ERROR_DROP;
    }

    if (need_resolve) {
        error = address_resolve(query_type, &target, &attach);
        if (error == UR_ERROR_ADDRESS_QUERY) {
            hal = get_default_hal_context();
            message_queue_enqueue(&hal->send_queue[PENDING_QUEUE], message);
            return error;
        } else if (error == UR_ERROR_NONE) {
            address_cache_t target_cache;
            target_cache.attach_sid = attach.sid;
            target_cache.sid = target.sid;
            target_cache.meshnetid = target.meshnetid;
            set_dest_info(info, &target_cache);
        } else {
            message_free(message);
            return error;
        }
    } else if (is_mcast && info->type == MESH_FRAME_TYPE_DATA) {
        info->flags |= INSERT_MCAST_FLAG;
    }

    if (nbr && is_mcast == false) {
        memcpy(&info->dest.addr, &nbr->mac, sizeof(info->dest.addr));
    }

    set_src_info(info);
    network = info->network;
    hal = network->hal;
    if (info->type == MESH_FRAME_TYPE_DATA) {
        message_queue_enqueue(&hal->send_queue[DATA_QUEUE], message);
    } else {
        message_queue_enqueue(&hal->send_queue[CMD_QUEUE], message);
    }
    yos_schedule_call(send_datagram, hal);
    return error;
}

static bool proxy_check(message_t *message)
{
    network_context_t *network;
    message_info_t    *info;

    info = message->info;
    network = get_default_network_context();

    if (is_bcast_sid(&info->src) ||
        is_same_mainnet(info->src.netid, mm_get_meshnetid(network)) == false ||
        (mm_get_seclevel() > SEC_LEVEL_0 && (info->flags & ENCRYPT_ENABLE_FLAG) == 0)) {
        if (info->type == MESH_FRAME_TYPE_DATA) {
            return false;
        }

        network = get_network_context_by_meshnetid(info->dest.netid);
        if (info->dest.netid != BCAST_NETID &&
            (network == NULL || info->dest.addr.short_addr != mm_get_local_sid())) {
            return false;
        }

        if (info->dest.netid != BCAST_NETID &&
            (info->dest2.addr.len != SHORT_ADDR_SIZE || is_bcast_sid(&info->dest2)) == false) {
            return false;
        }

        if (info->dest2.addr.len != SHORT_ADDR_SIZE ||
            info->dest.addr.short_addr == LEADER_SID) {
            info->dest2.addr.len = 0;
        } else {
            info->dest2.addr.short_addr = LEADER_SID;
            info->dest2.netid = get_main_netid(mm_get_meshnetid(network));
        }
    }

    return true;
}

static void message_handler(void *args)
{
    ur_error_t error = UR_ERROR_NONE;
    uint8_t *nexth;
    message_t *assemble_message;
    network_context_t *network = NULL;
    hal_context_t *hal = NULL;
    message_info_t *info;
    bool recv = false;
    bool forward = false;
    neighbor_t *nbr = NULL;
    received_frame_t *frame = NULL;

    frame = (received_frame_t *)args;
    hal = frame->hal;
    info = frame->message->info;
    network = get_default_network_context();

    if (proxy_check(frame->message) == false) {
        message_free(frame->message);
        ur_mem_free(frame, sizeof(received_frame_t));
        hal->link_stats.in_drops++;
        return;
    }

    if (memcmp(&info->dest.addr, mm_get_mac_address(), sizeof(info->dest.addr)) == 0) {
        recv = true;
    } else if (info->dest.addr.len == SHORT_ADDR_SIZE) {
        if (info->dest.netid != BCAST_NETID &&
            is_same_mainnet(info->dest.netid, mm_get_main_netid(network))) {
            network = get_network_context_by_meshnetid(info->dest.netid);
            if (info->dest.addr.short_addr == BCAST_SID ||
                (network && info->dest.addr.short_addr == mm_get_local_sid())) {
                recv = true;
            } else {
                forward = true;
            }
        } else if (info->type == MESH_FRAME_TYPE_CMD && info->dest.netid == BCAST_NETID) {
            recv = true;
        }
    }

    if (recv != true && forward != true) {
        hal->link_stats.in_filterings++;
        message_free(frame->message);
        ur_mem_free(frame, sizeof(received_frame_t));
        return;
    }

    if (recv && info->dest2.addr.len != 0) {
        memcpy(&info->dest.addr, &info->dest2.addr, sizeof(info->dest.addr));
        info->dest2.addr.len = 0;
        message_set_payload_offset(frame->message, -info->payload_offset);
        info->flags |= INSERT_MESH_HEADER;
        set_src_info(info);
        forward = true;
    }

    if (info->type == MESH_FRAME_TYPE_CMD) {
        handle_diags_command(frame->message, recv);
    }

    if (forward == true) {
        network = NULL;
        nbr = get_neighbor(info->type, info->dest.netid, &info->dest.addr);
        if (info->dest.addr.len == EXT_ADDR_SIZE) {
            info->dest.netid = BCAST_NETID;
            if (nbr) {
                hal = (hal_context_t *)nbr->hal;
                network = get_hal_default_network_context(hal);
            }
        } else if (info->dest.netid != BCAST_NETID) {
            network = get_network_context_by_meshnetid(info->dest.netid);
            if (network == NULL) {
                network = get_default_network_context();
            }
        }
        if (network == NULL) {
            hal->link_stats.in_drops++;
            message_free(frame->message);
            ur_mem_free(frame, sizeof(received_frame_t));
            return;
        }
        if (nbr) {
            memcpy(&info->dest.addr, &nbr->mac, sizeof(info->dest.addr));
        }
        info->network = network;
        info->payload_offset = 0;
        hal = network->hal;
        if (info->type == MESH_FRAME_TYPE_DATA) {
            message_queue_enqueue(&hal->send_queue[DATA_QUEUE], frame->message);
        } else {
            message_queue_enqueue(&hal->send_queue[CMD_QUEUE], frame->message);
        }
        yos_schedule_call(send_datagram, network->hal);
        return;
    }

    message_set_payload_offset(frame->message, -info->payload_offset);
    nexth = message_get_payload(frame->message);
    if (is_fragment_header(*nexth)) {
        error = lp_reassemble(frame->message, &assemble_message);
        if (error == UR_ERROR_NONE && assemble_message) {
            frame->message = assemble_message;
        } else {
            if (error != UR_ERROR_NONE) {
                message_free(frame->message);
                ur_mem_free(frame, sizeof(received_frame_t));
            }
            return;
        }
    }

    if (info->type == MESH_FRAME_TYPE_DATA) {
        hal->link_stats.in_data++;
    } else {
        hal->link_stats.in_command++;
    }

    yos_schedule_call(handle_datagram, frame->message);
    ur_mem_free(frame, sizeof(received_frame_t));
}

static void handle_received_frame(void *context, frame_t *frame,
                                  frame_info_t *frame_info,
                                  int error)
{
    message_t         *message;
    received_frame_t  *rx_frame = NULL;
    whitelist_entry_t *entry;
    hal_context_t *hal = (hal_context_t *)context;

    hal->link_stats.in_frames++;
    if (mm_get_device_state() == DEVICE_STATE_DISABLED) {
        hal->link_stats.in_drops++;
        return;
    }

    if (error != 0 || is_mesh_header(frame->data[0]) == false) {
        hal->link_stats.in_filterings++;
        return;
    }

    if (whitelist_is_enabled()) {
        entry = whitelist_find(&frame_info->peer);
        if (entry == NULL) {
            hal->link_stats.in_filterings++;
            return;
        }
    }

    message = message_alloc(frame->len);
    if (message == NULL) {
        hal->link_stats.in_drops++;
        return;
    }
    rx_frame = (received_frame_t *)ur_mem_alloc(sizeof(received_frame_t));
    if (rx_frame == NULL) {
        hal->link_stats.in_drops++;
        message_free(message);
        return;
    }
    message_copy_from(message, frame->data, frame->len);
    rx_frame->message = message;
    rx_frame->hal = hal;
    memcpy(&rx_frame->frame_info, frame_info, sizeof(rx_frame->frame_info));
    resolve_message_info(rx_frame);
    yos_schedule_call(message_handler, rx_frame);
}

static void send_datagram(void *args)
{
    ur_error_t error = UR_ERROR_NONE;
    hal_context_t *hal;
    message_t *message = NULL;
    uint8_t *lowpan_payload;
    message_info_t *info;
    int16_t offset = 0;
    uint8_t ip_hdr_len;
    uint8_t lowpan_hdr_len;
    uint8_t *ip_payload;

    hal = (hal_context_t *)args;
    message = hal->send_message;
    if (message && hal->frag_info.offset > message->frag_offset) {
        return;
    }
    if (message == NULL) {
        message = message_queue_get_head(&hal->send_queue[CMD_QUEUE]);
        if (message) {
            hal->link_stats.out_command++;
        } else {
            message = message_queue_get_head(&hal->send_queue[DATA_QUEUE]);
            if (message) {
                hal->link_stats.out_data++;
            }
        }
        if (message == NULL) {
            return;
        }
        hal->send_message = message;
    }
    info = message->info;
    if (info->flags & INSERT_LOWPAN_FLAG) {
        if (info->flags & ENABLE_COMPRESS_FLAG) {
            ip_payload = (uint8_t *)ur_mem_alloc((UR_IP6_HLEN + UR_UDP_HLEN) * 2);
            if (ip_payload == NULL) {
                hal->link_stats.out_errors++;
                return;
            }
            if (message_get_msglen(message) >= (UR_IP6_HLEN + UR_UDP_HLEN)) {
                message_copy_to(message, 1, ip_payload, UR_IP6_HLEN + UR_UDP_HLEN);
            } else if (message_get_msglen(message) >= UR_IP6_HLEN) {
                message_copy_to(message, 1, ip_payload, UR_IP6_HLEN);
            }
            lowpan_payload = ip_payload + UR_IP6_HLEN + UR_UDP_HLEN;
            lp_header_compress(ip_payload, lowpan_payload, &ip_hdr_len, &lowpan_hdr_len);
            offset = ip_hdr_len - lowpan_hdr_len;

            *(--lowpan_payload) = LOWPAN_IPHC_DISPATCH;
            message_set_payload_offset(message, -offset);
            message_copy_from(message, lowpan_payload, ip_hdr_len - offset);
            ur_mem_free(ip_payload, (UR_IP6_HLEN + UR_UDP_HLEN) * 2);
        }
        info->flags &= (~(INSERT_LOWPAN_FLAG | ENABLE_COMPRESS_FLAG));
    }

    if (info->flags & INSERT_MCAST_FLAG) {
        offset = sizeof(mcast_header_t) + 1;
        message_set_payload_offset(message, offset);
        lowpan_payload = message_get_payload(message);
        *lowpan_payload = MCAST_HEADER_DISPATCH;
        insert_mcast_header(info->network, lowpan_payload + 1);
        info->flags &= (~INSERT_MCAST_FLAG);
    }

    error = send_fragment(info->network, message);
    if (error == UR_ERROR_NONE) {
        hal->sending_timer = ur_start_timer(SENDING_TIMEOUT,
                                            handle_sending_timer, hal);
    } else if (error == UR_ERROR_DROP) {
        hal->last_sent = SENT_FAIL;
        yos_schedule_call(message_sent_task, hal);
    }
}

static void handle_datagram(void *args)
{
    ur_error_t        error = UR_ERROR_NONE;
    int16_t           offset;
    message_info_t    *info;
    message_t         *message;
    message_t         *relay_message;
    uint8_t           *nexth;
    slist_t           *hals;
    hal_context_t     *hal;
    network_context_t *network = NULL;

    message = (message_t *)args;
    info = message->info;

    if (info->dest.netid != BCAST_NETID) {
        info->network = get_network_context_by_meshnetid(info->dest.netid);
    } else if (info->src.netid != BCAST_NETID) {
        info->network = get_network_context_by_meshnetid(info->src.netid);
        if (info->network == NULL) {
            hal = get_hal_context(info->hal_type);
            info->network = get_hal_default_network_context(hal);
        }
    }
    if (info->network == NULL) {
        message_free(message);
        return;
    }

    if (info->type == MESH_FRAME_TYPE_DATA) {
        nexth = message_get_payload(message);
        if (is_mcast_header(*nexth)) {
            network = get_default_network_context();
            error = process_mcast_header(network, message_get_payload(message) + 1);
            if (error != UR_ERROR_NONE) {
                message_free(message);
                return;
            }
            if (info->flags & ENCRYPT_ENABLE_FLAG) {
                info->flags = ENCRYPT_ENABLE_FLAG;
            }
            info->flags |= INSERT_MESH_HEADER;
            hals = get_hal_contexts();
            slist_for_each_entry(hals, hal, hal_context_t, next) {
                relay_message = message_alloc(message_get_msglen(message));
                if (relay_message != NULL) {
                    relay_message->info->network = (void *)get_hal_default_network_context(hal);
                    message_copy(relay_message, message);
                    message_queue_enqueue(&hal->send_queue[DATA_QUEUE], relay_message);
                    yos_schedule_call(send_datagram, hal);
                }
            }
            offset = sizeof(mcast_header_t) + 1;
            message_set_payload_offset(message, -offset);
            nexth = message_get_payload(message);
        }
        if (is_uncompressed(*nexth)) {
            message_set_payload_offset(message, -1);
        } else if (is_lowpan_iphc(*nexth)) {
            message_set_payload_offset(message, -1);
            message = handle_lowpan_iphc(message);
        } else {
            message_free(message);
            return;
        }
        ur_mesh_input((umessage_t *)message);
    } else {
        mm_handle_frame_received(message);
        message_free(message);
    }
}

const ur_link_stats_t *mf_get_stats(hal_context_t *hal) {
    if (hal == NULL) {
        return NULL;
    }

    hal->link_stats.sending = hal->send_message? true: false;
    return &hal->link_stats;
}

ur_error_t mf_init(void)
{
    slist_t *hals;
    hal_context_t *hal;

    address_resolver_init(address_resolved_handler);

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        hal_ur_mesh_register_receiver(hal->module, handle_received_frame, hal);
        ur_stop_timer(&hal->sending_timer, hal);
        hal->frag_info.tag = 0;
        hal->frag_info.offset = 0;
    }
    return UR_ERROR_NONE;
}

ur_error_t mf_deinit(void)
{
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        ur_stop_timer(&hal->sending_timer, hal);
    }

    return UR_ERROR_NONE;
}
