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

#include "hal/mesh.h"
#include "mesh_types.h"
#include "urmesh.h"
#include "mesh_mgmt.h"
#include "mesh_forwarder.h"
#include "router_mgr.h"
#include "lowpan6.h"
#include "logging.h"
#include "message.h"
#include "encoding.h"
#include "ip6.h"
#include "ip6_address.h"
#include "network_data.h"
#include "mcast.h"
#include "sid_allocator.h"
#include "address_resolver.h"
#include "mac_whitelist.h"
#include "link_mgmt.h"
#include "timer.h"
#include "interface_context.h"
#include "interfaces.h"
#include "diags.h"
#include "keys_mgr.h"

typedef void (*yoc_call_t)(void *);
extern int yoc_schedule_call(yoc_call_t f, void *arg);

typedef struct received_frame_s {
    hal_context_t *hal;
    message_t    *message;
    frame_info_t frame_info;
} received_frame_t;

typedef struct loopback_frame_s {
    network_context_t *network;
    message_t *message;
} loopback_frame_t;

typedef struct frag_info_s {
    uint16_t tag;
    uint16_t offset;
} frag_info_t;
static frag_info_t g_frag_info;

enum {
    SENDING_TIMEOUT = 5000,
};

enum {
    SENT_SUCCESS = 0,
    SENT_FAIL = -1,
};

static ur_error_t send_mesh(network_context_t *network,
                            message_t *message, uint16_t offset,
                            uint16_t length, uint8_t *append,
                            uint16_t append_length);
static ur_error_t forward_mesh(network_context_t *network,
                               frame_t *frame, mesh_dest_t *dest);
static void send_datagram(void *args);
static ur_error_t handle_datagram(network_context_t *network,
                                  message_t *message, mesh_header_t *mesh_header);
static neighbor_t *get_next_node(network_context_t *network, mesh_dest_t *dest);

static inline bool is_lowpan_iphc(uint8_t control)
{
    return (control & LOWPAN_IPHC_DISPATCH_MASK) == LOWPAN_IPHC_DISPATCH;
}

static inline bool is_uncompressed(uint8_t control)
{
    return control == UNCOMPRESSED_DISPATCH;
}

static inline bool is_mm_header(uint8_t control)
{
    return (control & MM_HEADER_DISPATCH_MASK) == MM_HEADER_DISPATCH;
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

static inline bool is_local_ucast_address(network_context_t *network,
                                          mesh_dest_t *dest)
{
    mac_address_t       *addr;
    const mac_address_t *mac;
    bool                matched = false;

    addr = &dest->dest;
    switch (addr->len) {
        case 2:
            if (network == NULL) {
                return matched;
            }
            if (network->meshnetid == dest->meshnetid &&
                addr->short_addr == mm_get_local_sid()) {
                matched = true;
            }
            break;
        case 8:
            if (memcmp(addr->addr, mm_get_local_ueid(), sizeof(addr->addr)) == 0) {
                matched = true;
            }
            mac = mm_get_mac_address();
            if (memcmp(addr->addr, mac->addr, sizeof(mac->addr)) == 0) {
                matched = true;
            }
            break;
        default:
            matched = false;
    }

    return matched;
}

static inline bool is_bcast_address(const mac_address_t *addr)
{
    bool    bcast = false;
    uint8_t index;

    switch (addr->len) {
        case 2:
            if (addr->short_addr == BCAST_SID) {
                bcast = true;
            }
            break;
        case 8:
            for (index = 0; index < 8; index++) {
                if (addr->addr[index] != 0xff) {
                    break;
                }
            }
            if (index == 8) {
                bcast = true;
            }
            break;
        default:
            break;
    }
    return bcast;
}

static inline uint8_t get_mesh_hops_left(uint8_t control)
{
    return control & MESH_HOPS_LEFT_MASK;
}

static inline void set_mesh_hops_left(uint8_t *control, uint8_t hops)
{
    *control = (*control & ~MESH_HOPS_LEFT_MASK) | hops;
}

static uint16_t get_sid(network_context_t *network, const mac_address_t *addr)
{
    uint16_t   sid = INVALID_SID;
    neighbor_t *node;

    switch (addr->len) {
        case 2:
            sid = addr->short_addr;
            break;
        case 8:
            node = get_neighbor_by_mac_addr(network->hal, addr->addr);
            if (node) {
                sid = node->sid;
            }
            if (sid == INVALID_SID) {
                sid = BCAST_SID;
            }
            break;
        default:
            sid = INVALID_SID;
            break;
    }
    return sid;
}

static ur_error_t check_reachability(network_context_t *network, mesh_src_t *src,
                                     mesh_dest_t *dest)
{
    ur_error_t error = UR_ERROR_NONE;
    neighbor_t *next_node;
    sid_t      dest_node;
    sid_t      target_node;

    if (is_bcast_address(&dest->dest)) {
        return UR_ERROR_NONE;
    }

    if (dest->dest.len == 2) {
        next_node = get_next_node(network, dest);
        if (next_node == NULL) {
            network = get_network_context_by_meshnetid(src->meshnetid);
            if (network == NULL) {
                network = get_default_network_context();
            }
            dest_node.sid = src->sid;
            dest_node.meshnetid = src->meshnetid;
            target_node.sid = dest->dest.short_addr;
            target_node.meshnetid = dest->meshnetid;
            send_dest_unreachable(network, &dest_node, &target_node);
            error = UR_ERROR_DROP;
        }
    }

    return error;
}

void message_sent_task(void *args)
{
    hal_context_t *hal = (hal_context_t *)args;
    message_t     *message;
    uint16_t      msg_length = message_get_msglen(hal->send_message);

    message = hal->send_message;
    if (message == NULL) {
        return;
    }

    if (hal->sending_flags & SENDING_UCAST) {
        hal->sending_flags &= (~SENDING_UCAST);
    } else if (hal->sending_flags & SENDING_BCAST) {
        hal->sending_flags &= (~SENDING_BCAST);
    }
    if ((hal->sending_flags & SENDING_DATA) && g_frag_info.offset < msg_length &&
        hal->last_sent == SENT_SUCCESS) {
        message->frag_offset = g_frag_info.offset;
    } else {
        if (hal->sending_flags & SENDING_CMD) {
            hal->sending_flags &= (~SENDING_CMD);
        } else {
            hal->sending_flags &= (~SENDING_DATA);
        }
        message_queue_dequeue(message);
        message_free(message);
        hal->send_message = NULL;
    }
    send_datagram(NULL);
}

static void handle_sent_ucast(void *context,
                              frame_t *frame, int error)
{
    hal_context_t *hal = (hal_context_t *)context;

    ur_stop_timer(&hal->ucast_sending_timer, hal);
    if (error != SENT_SUCCESS) {
        error = SENT_FAIL;
    }
    hal->last_sent = error;
    yoc_schedule_call(message_sent_task, hal);
}

static void handle_sent_bcast(void *context,
                              frame_t *frame, int error)
{
    hal_context_t *hal = (hal_context_t *)context;

    ur_stop_timer(&hal->bcast_sending_timer, hal);
    if (error != SENT_SUCCESS) {
        error = SENT_FAIL;
    }
    hal->last_sent = error;
    yoc_schedule_call(message_sent_task, hal);
}

static message_t *handle_lowpan_iphc(message_t *message, mesh_header_t* mesh_header)
{
    return lp_header_decompress(message, mesh_header);
}

static ur_error_t handle_mm(message_t *message, const mesh_src_t *src,
                            const mac_address_t *dest)
{
    ur_error_t error = UR_ERROR_NONE;
    uint8_t    *payload;
    uint16_t   length;

    payload = message_get_payload(message);
    length = message_get_msglen(message);
    error = mm_handle_frame_received(payload, length, src, dest);
    return error;
}

static ur_error_t handle_one_frame(received_frame_t *frame, mesh_src_t *src,
                                   uint8_t offset)
{
    ur_error_t    error = UR_ERROR_NONE;
    mesh_header_t *mesh_header;
    uint8_t       *nexth;
    message_t     *assemble_message = NULL;
    uint8_t       frame_type;
    slist_t       *networks;
    network_context_t *dest_network = NULL;
    uint16_t dest_meshnetid;
    uint16_t dest_sid;

    mesh_header = (mesh_header_t *)message_get_payload(frame->message);
    frame_type = mesh_header->control & MESH_FRAME_TYPE_MASK;
    dest_meshnetid = frame->message->dest->meshnetid;
    dest_sid = frame->message->dest->dest.short_addr;

    if (mesh_header->control & MESH_HEADER_RELAYER) {
        offset += sizeof(mesh_header_relayer_t);
    }
    message_set_payload_offset(frame->message, -offset);

    switch(frame_type) {
        case MESH_FRAME_TYPE_DATA:
            if (dest_meshnetid == BCAST_NETID) {
                error = UR_ERROR_DROP;
            }
            if (error == UR_ERROR_NONE) {
                dest_network = get_network_context_by_meshnetid(dest_meshnetid);
            }
            if (dest_network == NULL) {
                // mcast
                if (dest_sid == BCAST_SID &&
                    dest_meshnetid == mm_get_main_netid(dest_network)) {
                    dest_network = get_default_network_context();
                // relay ucast
                } else if (dest_sid != BCAST_SID) {
                    error = UR_ERROR_FAIL;
                } else {
                    error = UR_ERROR_DROP;
                }
            } else if (dest_sid != BCAST_SID && dest_sid != mm_get_local_sid()) {
                error = UR_ERROR_FAIL;
            }
            if (error == UR_ERROR_NONE && dest_network) {
                nexth = message_get_payload(frame->message);
                if (is_fragment_header(*nexth)) {
                    error = lp_reassemble(frame->message, mesh_header, &assemble_message);
                    if (error == UR_ERROR_NONE && assemble_message) {
                        frame->message = assemble_message;
                    } else {
                        if (error != UR_ERROR_NONE) {
                            error = UR_ERROR_DROP;
                        } else {
                            return error;
                        }
                    }
                }
                if (error == UR_ERROR_NONE) {
                    error = handle_datagram(dest_network, frame->message, mesh_header);
                }
            }
            if (error == UR_ERROR_NONE) {
                frame->hal->link_stats.in_data++;
            }
            break;
        case MESH_FRAME_TYPE_CMD:
            if (dest_meshnetid != BCAST_NETID) {
                dest_network = get_network_context_by_meshnetid(dest_meshnetid);
                if (dest_network) {
                    if (dest_sid != BCAST_SID && dest_sid != mm_get_local_sid()) {
                        error = UR_ERROR_FAIL;
                    }
                } else if (mm_get_device_state() >= DEVICE_STATE_LEAF &&
                           (mm_get_attach_state() == ATTACH_IDLE ||
                            mm_get_attach_state() == ATTACH_DONE)) {
                    error = UR_ERROR_FAIL;
                }
            }
            handle_diags_command(frame->message, src, NULL, (error == UR_ERROR_FAIL)? false: true);
            if (error == UR_ERROR_NONE) {
                networks = get_network_contexts();
                slist_for_each_entry(networks, dest_network, network_context_t, next) {
                    if ((dest_meshnetid != BCAST_NETID &&
                         dest_meshnetid != mm_get_meshnetid(dest_network)) ||
                        (dest_meshnetid == BCAST_NETID && dest_network->hal != src->hal)) {
                        continue;
                    }
                    src->dest_network = dest_network;
                    error = handle_mm(frame->message, src, NULL);
                    if (error == UR_ERROR_NONE) {
                        frame->hal->link_stats.in_command++;
                    } else {
                        error = UR_ERROR_DROP;
                    }
                }
            }
            if (error == UR_ERROR_NONE) {
                message_set_payload_offset(frame->message, offset);
                message_free(frame->message);
            }
            break;
        default:
            error = UR_ERROR_DROP;
            break;
    }

    if (error == UR_ERROR_FAIL || error == UR_ERROR_DROP) {
        message_set_payload_offset(frame->message, offset);
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
            "frame %d %d from %x %x -> %x %x, error %d\r\n",
            frame->hal->module->type, frame_type,
            mesh_header->src, mm_get_meshnetid(dest_network),
            dest_sid, dest_meshnetid, error);

    return error;
}

static ur_error_t relay_one_frame(received_frame_t *frame)
{
    ur_error_t error = UR_ERROR_NONE;
    network_context_t *network;

    network = frame->message->network;
    message_queue_enqueue(&network->hal->send_queue[CMD_QUEUE], frame->message);
    network->hal->last_tx_time = ur_get_now();
    yoc_schedule_call(send_datagram, NULL);

    return error;
}

static ur_error_t handle_mesh(received_frame_t *frame)
{
    ur_error_t    error = UR_ERROR_DROP;
    uint8_t       hops_left;
    mesh_header_t *mesh_header;
    mesh_header_destnetid_t *header_destnetid;
    mesh_header_destsid_t *header_destsid;
    mesh_header_relayer_t *relayer = NULL;
    sid_t dest_sid;
    uint8_t offset;
    mesh_src_t src;
    mesh_dest_t *dest;
    message_t *message;
    network_context_t *network;

    message = frame->message;
    offset = sizeof(mesh_header_t);
    mesh_header = (mesh_header_t *)message_get_payload(message);
    if (mesh_header->control_ext & MESH_HEADER_BCASTDESTSID) {
        dest_sid.sid = BCAST_SID;
    } else {
        header_destsid = (mesh_header_destsid_t *)(message_get_payload(message) +
                                                   offset);
        dest_sid.sid = header_destsid->dest;
        offset += sizeof(mesh_header_destsid_t);
    }
    if (mesh_header->control & MESH_HEADER_DESTNETID) {
        header_destnetid = (mesh_header_destnetid_t *)(message_get_payload(message) +
                                                       offset);
        if (mesh_header->meshnetid != BCAST_NETID &&
            mesh_header->meshnetid != INVALID_NETID &&
            header_destnetid->destsubnetid != BCAST_SUB_NETID) {
            dest_sid.meshnetid = mk_sub_netid(mesh_header->meshnetid, header_destnetid->destsubnetid);
        } else {
            dest_sid.meshnetid = BCAST_NETID;
        }
        offset += sizeof(mesh_header_destnetid_t);
    } else if (mesh_header->control_ext & MESH_HEADER_BCASTDESTSID) {
        dest_sid.meshnetid = BCAST_NETID;
    } else {
        dest_sid.meshnetid = mesh_header->meshnetid;
    }

    src.hal = frame->hal;
    src.sid = mesh_header->src;
    src.meshnetid = mesh_header->meshnetid;
    src.channel = frame->frame_info.channel;
    memcpy(&src.src, &frame->frame_info.peer, sizeof(src.src));

    dest = message->dest;
    dest->meshnetid = dest_sid.meshnetid;
    dest->dest.len = 2;
    dest->dest.short_addr = dest_sid.sid;

    error = handle_one_frame(frame, &src, offset);
    if (error == UR_ERROR_FAIL) {
        error = UR_ERROR_DROP;
        hops_left = get_mesh_hops_left(mesh_header->control);
        if (hops_left > 0) {
            set_mesh_hops_left(&(mesh_header->control), hops_left - 1);
            mesh_header = (mesh_header_t *)message_get_payload(message);
            network = get_network_context_by_meshnetid(dest->meshnetid);
            if (network == NULL) {
                network = get_default_network_context();
            }
            message->network = network;
            if (mesh_header->control & MESH_HEADER_RELAYER) {
                dest->flags |= RELAYERSID_FLAG;
                relayer = (mesh_header_relayer_t *)((uint8_t *)mesh_header + offset);
                dest->relayersid = relayer->relayer;
            }
            error = check_reachability(network, &src, frame->message->dest);
        } else {
            network = get_hal_default_network_context(frame->hal);
            dest_sid.meshnetid = src.meshnetid;
            dest_sid.sid = src.sid;
            send_address_error(network, &dest_sid);
            error = UR_ERROR_DROP;
        }
        if (error == UR_ERROR_NONE) {
            error = relay_one_frame(frame);
        }
    }

    if (error != UR_ERROR_NONE) {
        ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
               "frame from %x %x -> %x dropped\r\n",
               mesh_header->src, mesh_header->meshnetid, dest_sid);
        frame->hal->link_stats.in_drops++;
        message_free(frame->message);
    }

    return error;
}

static uint8_t calc_header_length(mesh_dest_t *dest)
{
    uint8_t length;

    length = sizeof(mesh_header_t);

    if (dest->flags & INSERT_DESTNETID_FLAG) {
        length += sizeof(mesh_header_destnetid_t);
    }
    if (dest->flags & RELAYERSID_FLAG) {
        length += sizeof(mesh_header_relayer_t);
    }
    if (dest->dest.short_addr != BCAST_SID) {
        length += sizeof(mesh_header_destsid_t);
    }

    return length;
}

static ur_error_t send_fragment(network_context_t *network, message_t *message)
{
    ur_error_t    error = UR_ERROR_NONE;
    uint16_t      frag_length;
    uint16_t      msg_length;
    frag_header_t frag_header;
    uint16_t      mtu;
    mesh_dest_t   *dest;
    uint8_t       header_length = 0;

    dest = message->dest;
    if (is_bcast_address(&dest->dest)) {
        mtu = hal_ur_mesh_get_bcast_mtu(network->hal->module);
    } else {
        mtu = hal_ur_mesh_get_ucast_mtu(network->hal->module);
    }
    header_length = calc_header_length(dest);
    msg_length = message_get_msglen(message);
    memset(&frag_header, 0, sizeof(frag_header));

    if (message->frag_offset == 0) {
        frag_length = msg_length;
        if (frag_length > (mtu - header_length)) {
            frag_header.dispatch = FRAG_1_DISPATCH;
            frag_header.size     = msg_length;
            *((uint16_t *)&frag_header) = ur_swap16(*((uint16_t *)&frag_header));
            g_frag_info.tag++;
            frag_header.tag = ur_swap16(g_frag_info.tag);
            frag_length = (mtu + 1 - sizeof(frag_header_t) - header_length) & 0xff8;
            error = send_mesh(network, message, 0, frag_length,
                              (uint8_t *)&frag_header, sizeof(frag_header) - 1);
        } else {
            error = send_mesh(network, message, 0, frag_length, NULL, 0);
        }
    } else {
        frag_header.dispatch = FRAG_N_DISPATCH;
        frag_header.size     = msg_length;
        *((uint16_t *)&frag_header) = ur_swap16(*((uint16_t *)&frag_header));
        frag_header.offset = (uint8_t)(message->frag_offset >> 3);
        frag_header.tag = ur_swap16(g_frag_info.tag);
        frag_length = (mtu - sizeof(frag_header_t) - header_length) & 0xff8;
        if (frag_length > (msg_length - message->frag_offset)) {
            frag_length = msg_length - message->frag_offset;
        }
        error = send_mesh(network, message, message->frag_offset, frag_length,
                          (uint8_t *)&frag_header, sizeof(frag_header));
    }
    g_frag_info.offset += frag_length;

    return error;
}

static void handle_bcast_sending_timer(void *args)
{
    hal_context_t *hal = args;

    hal->last_sent = SENT_FAIL;
    message_sent_task(hal);
    hal->bcast_sending_timer = NULL;
    hal->link_stats.bcast_sending_timeouts++;
}

static void handle_ucast_sending_timer(void *args)
{
    hal_context_t *hal = args;

    hal->last_sent = SENT_FAIL;
    message_sent_task(hal);
    hal->ucast_sending_timer = NULL;
    hal->link_stats.ucast_sending_timeouts++;
}

static neighbor_t *get_next_node(network_context_t *network, mesh_dest_t *dest)
{
    uint16_t      local_sid;
    uint16_t      next_hop;
    neighbor_t    *next = NULL;
    bool          same_subnet = false;
    bool          same_net = true;
    mac_address_t dest_addr;
    hal_context_t *hal;
    bool          to_relayer = false;
    bool          reach_relayer = false;

    hal = network->hal;
    local_sid = mm_get_local_sid();

    if (dest->dest.len == 8) {
        dest_addr.len = 8;
        memcpy(&dest_addr.addr, &dest->dest.addr, sizeof(dest_addr.addr));
        next = get_neighbor_by_mac_addr(hal, dest_addr.addr);
        return next;
    }

    if (local_sid == INVALID_SID) {
        next = mm_get_attach_candidate(network);
        return next;
    }

    if (is_partial_function_sid(local_sid)) {
        next = mm_get_attach_node(network);
        return next;
    }

    if (dest->flags & RELAYERSID_FLAG) {
        to_relayer = true;
    }
    if (to_relayer &&
        is_same_mainnet(dest->meshnetid, mm_get_meshnetid(network)) &&
        dest->relayersid == mm_get_local_sid()) {
        reach_relayer = true;
    }

    if (reach_relayer) {
        next = get_neighbor_by_sid(hal, dest->dest.short_addr, dest->meshnetid);
        return next;
    }

    dest_addr.len = 2;
    if (to_relayer) {
        dest_addr.short_addr = dest->relayersid;
    } else {
        dest_addr.short_addr = dest->dest.short_addr;
    }

    if (mm_get_meshnetid(network) == dest->meshnetid) {
        same_subnet = true;
    } else if ((dest->meshnetid & MAIN_NETID_MASK) !=
        (network->meshnetid & MAIN_NETID_MASK)) {
        same_net = false;
    }

    if (same_net == true) {
        if (same_subnet == true) {
            next_hop = ur_router_get_next_hop(network, dest_addr.short_addr);
            next = get_neighbor_by_sid(network->hal, next_hop, dest->meshnetid);
        } else if (is_subnet(network->meshnetid)) {
            next = mm_get_attach_node(network);
        } else {
            next_hop = ur_router_get_next_hop(network, get_leader_sid(dest->meshnetid));
            next = get_neighbor_by_sid(network->hal, next_hop, get_main_netid(dest->meshnetid));
        }
    } else {
        next = mm_get_attach_candidate(network);
    }

    return next;
}

static ur_error_t forward_mesh(network_context_t *network,
                               frame_t *frame, mesh_dest_t *dest)
{
    ur_error_t    error = UR_ERROR_NONE;
    neighbor_t    *next_node = NULL;
    hal_context_t *hal = network->hal;
    mac_address_t dest_addr;

    if (is_bcast_address(&dest->dest)) {
        hal->sending_flags |= SENDING_BCAST;
        hal->bcast_sending_timer = ur_start_timer(SENDING_TIMEOUT,
                                                  handle_bcast_sending_timer, hal);
        error = hal_ur_mesh_send_bcast_request(network->hal->module, frame,
                                               handle_sent_bcast, hal);
        if (error != UR_ERROR_NONE) {
            ur_stop_timer(&hal->bcast_sending_timer, hal);
            hal->last_sent = SENT_FAIL;
            yoc_schedule_call(message_sent_task, hal);
        }

        return error;
    }

    next_node = get_next_node(network, dest);
    if (next_node == NULL) {
        hal->last_sent = SENT_FAIL;
        yoc_schedule_call(message_sent_task, hal);
        return UR_ERROR_FAIL;
    }

    dest_addr.len = 8;
    memcpy(&dest_addr.addr, next_node->addr.addr, sizeof(dest_addr.addr));

    if (error == UR_ERROR_NONE) {
        hal->sending_flags |= SENDING_UCAST;
        hal->ucast_sending_timer = ur_start_timer(SENDING_TIMEOUT,
                                                  handle_ucast_sending_timer, hal);
        error = hal_ur_mesh_send_ucast_request(hal->module, frame, &dest_addr,
                                               handle_sent_ucast, hal);
    }
    if (error != UR_ERROR_NONE) {
        ur_stop_timer(&hal->ucast_sending_timer, hal);
        hal->last_sent = SENT_FAIL;
        yoc_schedule_call(message_sent_task, hal);
    }

    return error;
}

static uint8_t insert_mesh_header(network_context_t *network,
                                  mesh_dest_t *dest)
{
    mesh_header_t           *mesh_header;
    mesh_header_relayer_t   *mesh_relayer;
    mesh_header_destnetid_t *mesh_destnetid;
    mesh_header_destsid_t   *mesh_destsid;
    uint8_t                 length;
    hal_context_t           *hal;
    uint16_t                dest_sid;
    uint8_t                 hop_limit;

    dest_sid = get_sid(network, &dest->dest);
    if (dest_sid == INVALID_SID) {
        return 0;
    }

    hal = network->hal;
    mesh_header = (mesh_header_t *)hal->frame.data;
    mesh_header->control = MESH_HEADER_DISPATCH;
    mesh_header->control_ext = 0;
    length = sizeof(mesh_header_t);
    if (dest->flags & TYPE_COMMAND_FLAG) {
        mesh_header->control |= MESH_FRAME_TYPE_CMD;
    }
    if (dest->hop_limit) {
        hop_limit = dest->hop_limit;
    } else {
        hop_limit = FORWARD_HOP_LIMIT;
    }
    set_mesh_hops_left(&(mesh_header->control), hop_limit);
    mesh_header->meshnetid = mm_get_meshnetid(network);
    if (mesh_header->meshnetid == INVALID_NETID) {
        mesh_header->meshnetid = BCAST_NETID;
    }
    mesh_header->src = mm_get_local_sid();
    if (dest_sid == BCAST_SID) {
        mesh_header->control_ext |= MESH_HEADER_BCASTDESTSID;
    } else {
        mesh_destsid = (mesh_header_destsid_t *)(hal->frame.data + length);
        mesh_destsid->dest = dest_sid;
        length += sizeof(mesh_header_destsid_t);
    }
    if (dest->flags & INSERT_DESTNETID_FLAG) {
        if (dest->meshnetid == BCAST_NETID) {
            mesh_header->control_ext |= MESH_HEADER_BCASTDESTNETID;
        } else {
            mesh_header->control |= MESH_HEADER_DESTNETID;
            mesh_destnetid = (mesh_header_destnetid_t *)(hal->frame.data + length);
            mesh_destnetid->destsubnetid = get_sub_netid(dest->meshnetid);
            length += sizeof(mesh_header_destnetid_t);
        }
    }
    if (dest->flags & RELAYERSID_FLAG) {
        mesh_header->control |= MESH_HEADER_RELAYER;
        mesh_relayer = (mesh_header_relayer_t *)(hal->frame.data + length);
        mesh_relayer->relayer = dest->relayersid;
        length += sizeof(mesh_header_relayer_t);
    }
    if ((mm_get_seclevel() > SEC_LEVEL_0) &&
        (dest->flags & ENCRYPT_ENABLE_FLAG)) {
        mesh_header->control_ext |= MESH_HEADER_ENABLE_ENCRYPT;
    }

    return length;
}

static ur_error_t send_mesh(network_context_t *network, message_t *message,
                            uint16_t offset, uint16_t length,
                            uint8_t *append, uint16_t append_length)
{
    ur_error_t    error = UR_ERROR_NONE;
    uint8_t       *payload;
    uint8_t       header_length = 0;
    mesh_dest_t   *dest;
    hal_context_t *hal = network->hal;

    dest = message->dest;
    if (dest->flags & INSERT_MESH_HEADER) {
        header_length = insert_mesh_header(network, dest);
        if (header_length == 0) {
            return UR_ERROR_DROP;
        }
    }
    if (append && append_length > 0) {
        memcpy(hal->frame.data + header_length, append, append_length);
    }
    payload = hal->frame.data + header_length + append_length;
    message_copy_to(message, offset, payload, length);
    hal->frame.len = header_length + append_length + length;

    error = forward_mesh(network, &hal->frame, dest);
    return error;
}

static void address_resolved_handler(network_context_t *network, address_cache_t *target,
                                     ur_error_t error)
{
    message_t       *message;
    ur_ip6_header_t *ip6_header;
    mesh_dest_t     *mesh_dest;
    uint16_t        dest;
    uint16_t        meshnetid;

    for_each_message(message, &network->hal->send_queue[PENDING_QUEUE]) {
        ip6_header = (ur_ip6_header_t *)message_get_payload(message);
        dest = ur_swap16(ip6_header->dest.m16[7]);
        meshnetid = ur_swap16(ip6_header->dest.m16[3]) | ur_swap16(ip6_header->dest.m16[6]);

        if ((dest == target->sid && meshnetid == target->meshnetid) ||
             (memcmp(target->ueid, &ip6_header->dest.m8[8], 8) == 0)) {
            message_queue_dequeue(message);
            if (error == UR_ERROR_NONE) {
                mesh_dest = message->dest;
                mesh_dest->dest.len = 2;
                mesh_dest->dest.short_addr = target->sid;
                mesh_dest->flags = INSERT_MESH_HEADER | TYPE_DATA_FLAG | INSERT_DESTNETID_FLAG;
                mesh_dest->meshnetid = target->meshnetid;
                if (target->attach_sid != INVALID_SID &&
                    target->attach_netid != INVALID_NETID) {
                    mesh_dest->flags |= RELAYERSID_FLAG;
                    mesh_dest->relayersid = target->attach_sid;
                    mesh_dest->meshnetid = target->attach_netid;
                }
                mesh_dest->flags |= (INSERT_LOWPAN_FLAG | ENABLE_COMPRESS_FLAG);
                message->network = get_network_context_by_meshnetid(mesh_dest->meshnetid);
                if (message->network == NULL) {
                    message->network = get_default_network_context();
                }
                message_queue_enqueue(&message->network->hal->send_queue[DATA_QUEUE], message);
                message->network->hal->last_tx_time = ur_get_now();
                yoc_schedule_call(send_datagram, NULL);
            } else {
                message_free(message);
            }
        }
    }
}

static void loopback_data(void *message)
{
    ur_error_t error = UR_ERROR_NONE;
    mesh_dest_t *dest;
    network_context_t *network;

    error = ur_mesh_input((message_t *)message);
    dest = ((message_t *)message)->dest;
    network = get_network_context_by_meshnetid(dest->meshnetid);
    if (error != UR_ERROR_NONE) {
        network->hal->link_stats.in_drops++;
        message_free((message_t *)message);
    }
}

ur_error_t mf_resolve_dest(const ur_ip6_addr_t *dest, sid_t *dest_sid)
{
    if (ur_is_mcast(dest)) {
        return UR_ERROR_FAIL;
    }
    if (is_sid_address(&dest->m8[8]) == false) {
        return UR_ERROR_FAIL;
    }

    dest_sid->sid = ur_swap16(dest->m16[7]);
    dest_sid->meshnetid = ur_swap16(dest->m16[3]) | ur_swap16(dest->m16[6]);
    return UR_ERROR_NONE;
}

static void set_dest_encrypt_flag(mm_command_t command, mesh_dest_t *dest)
{
    if (mm_get_seclevel() > SEC_LEVEL_0 &&
        command != COMMAND_ADVERTISEMENT && command != COMMAND_DISCOVERY_REQUEST &&
        command != COMMAND_DISCOVERY_RESPONSE) {
        dest->flags |= ENCRYPT_ENABLE_FLAG;
    }
}

ur_error_t mf_send_ip6(message_t *message, const ur_ip6_addr_t *dest)
{
    ur_error_t        error = UR_ERROR_NONE;
    mesh_dest_t       *mesh_dest;
    ur_node_id_t      target;
    ur_node_id_t      attach;
    network_context_t *network = NULL;
    uint8_t           query_type;
    neighbor_t        *nbr = NULL;
    bool              is_mcast = false;
    bool              is_ueid = false;
    bool              is_pf = false;
    hal_context_t     *hal;
    message_t         *tx_message = NULL;

    mesh_dest = message->dest;
    mesh_dest->flags = INSERT_MESH_HEADER | TYPE_DATA_FLAG | INSERT_LOWPAN_FLAG;
    set_dest_encrypt_flag(0, mesh_dest);

    if ((ur_is_mcast(dest)) && (nd_is_subscribed_mcast(dest))) {
        is_mcast = true;
    } else if (ur_is_unique_local(dest)) {
        if (is_sid_address(&dest->m8[8]) == false) {
            is_ueid = true;
        }
        mesh_dest->flags |= ENABLE_COMPRESS_FLAG;
    } else {
        message_free(message);
        return UR_ERROR_DROP;  /* TODO:support all kinds of IPv6 addresses */
    }

    if (is_mcast) {
        mesh_dest->dest.len = 2;
        mesh_dest->dest.short_addr = BCAST_SID;
        mesh_dest->meshnetid = mm_get_main_netid(network);
        mesh_dest->relayersid = INVALID_SID;
        mesh_dest->flags |= (INSERT_MCAST_FLAG | INSERT_DESTNETID_FLAG);
    } else if (is_ueid) {
        mesh_dest->dest.len = 8;
        memcpy(mesh_dest->dest.addr, &dest->m8[8], sizeof(mesh_dest->dest.addr));
        mesh_dest->meshnetid = INVALID_NETID;
    } else {
        mesh_dest->dest.len = 2;
        mesh_dest->dest.short_addr = ur_swap16(dest->m16[7]);
        mesh_dest->meshnetid = ur_swap16(dest->m16[3]) | ur_swap16(dest->m16[6]);
        network = get_network_context_by_meshnetid(mesh_dest->meshnetid);
        if (network == NULL) {
            network = get_default_network_context();
        }
        if (is_partial_function_sid(mesh_dest->dest.short_addr)) {
           is_pf = true;
        }
    }

    // local address
    if (is_mcast == false &&
        is_local_ucast_address(network, mesh_dest)) {
        yoc_schedule_call(loopback_data, (void *)message);
        return UR_ERROR_NONE;
    }

    // neighbor
    if (is_mcast == false) {
        if (is_ueid) {
            nbr = get_neighbor_by_ueid(&dest->m8[8]);
        } else {
            if (network == NULL) {
                network = get_default_network_context();
            }
            nbr = get_neighbor_by_sid(network->hal, mesh_dest->dest.short_addr,
                                      mesh_dest->meshnetid);
        }
    }

    if (nbr && nbr->sid != INVALID_SID) {
        if (get_network_context_by_meshnetid(nbr->netid) == NULL &&
            get_network_context_by_meshnetid(nbr->sub_netid) == NULL) {
            nbr = NULL;
        } else {
            mesh_dest->dest.len = 8;
            memcpy(mesh_dest->dest.addr, nbr->addr.addr, sizeof(mesh_dest->dest.addr));
            if ((mm_get_mode() & MODE_SUPER) == 0 || (nbr->mode & MODE_SUPER) == 0) {
                mesh_dest->meshnetid = nbr->sub_netid;
            } else {
                mesh_dest->meshnetid = nbr->netid;
            }
            network = get_network_context_by_meshnetid(mesh_dest->meshnetid);
            if (network == NULL) {
                network = get_default_network_context();
            }
        }
    } else if (is_pf || is_ueid) {
        network = get_default_network_context();
        target.sid = INVALID_SID;
        target.meshnetid = INVALID_NETID;
        memset(target.ueid, 0xff, sizeof(target.ueid));
        attach.sid = INVALID_SID;
        attach.meshnetid = INVALID_NETID;
        memset(attach.ueid, 0xff, sizeof(attach.ueid));
        if (is_pf) {
           target.sid = mesh_dest->dest.short_addr;
           target.meshnetid = mesh_dest->meshnetid;
           query_type = PF_ATTACH_QUERY;
        } else {
           memcpy(target.ueid, &dest->m8[8], sizeof(target.ueid));
           query_type = TARGET_QUERY;
        }
        error = address_resolve(network, query_type,
                                &target, &attach);
        if (error == UR_ERROR_ADDRESS_QUERY) {
            message_queue_enqueue(&network->hal->send_queue[PENDING_QUEUE], message);
            return error;
        } else if (error == UR_ERROR_NONE) {
            mesh_dest->dest.len = 2;
            mesh_dest->dest.short_addr = target.sid;
            mesh_dest->meshnetid = target.meshnetid;
            if (attach.sid != INVALID_SID) {
                mesh_dest->flags |= RELAYERSID_FLAG;
                mesh_dest->relayersid = attach.sid;
            }
            network = get_network_context_by_meshnetid(mesh_dest->meshnetid);
            if (network == NULL) {
                network = get_default_network_context();
            }
        } else {
            message_free(message);
            network->hal->link_stats.out_errors++;
            return error;
        }
    }

    mesh_dest->flags |= INSERT_DESTNETID_FLAG;
    if (is_mcast) {
        uint16_t append_length;

        hal = get_default_hal_context();
        slist_for_each_entry(&hal->next, hal, hal_context_t, next) {
            network = get_sub_network_context(hal);
            append_length = sizeof(mcast_header_t) + 2;
            tx_message = message_alloc(network, message_get_msglen(message) + append_length);
            message_set_payload_offset(tx_message, -append_length);
            message_copy(tx_message, message);
            tx_message->network = network;
            message_queue_enqueue(&hal->send_queue[DATA_QUEUE], tx_message);
            hal->last_tx_time = ur_get_now();
        }
        network = get_default_network_context();
    }
    message->network = network;
    message_queue_enqueue(&network->hal->send_queue[DATA_QUEUE], message);
    network->hal->last_tx_time = ur_get_now();
    yoc_schedule_call(send_datagram, NULL);
    return error;
}

static void loopback_command(void *data)
{
    mesh_src_t src;
    message_t *message = (message_t *)data;
    network_context_t *network = message->network;

    src.sid = mm_get_local_sid();
    src.dest_network = network;
    src.meshnetid = network->meshnetid;
    memcpy(&src.src, mm_get_mac_address(), sizeof(src.src));
    handle_mm(message, &src, NULL);
    message_free(message);
}

ur_error_t mf_send_command(uint8_t command, message_t *message)
{
    ur_error_t  error = UR_ERROR_NONE;
    mesh_dest_t *dest;
    network_context_t *network = message->network;

    dest = message->dest;
    dest->flags |= (INSERT_MESH_HEADER | TYPE_COMMAND_FLAG);
    set_dest_encrypt_flag(command, dest);
    if (is_local_ucast_address(network, dest)) {
        yoc_schedule_call(loopback_command, message);
    } else {
        message_queue_enqueue(&network->hal->send_queue[CMD_QUEUE], message);
        network->hal->last_tx_time = ur_get_now();
        yoc_schedule_call(send_datagram, NULL);
        network->hal->link_stats.out_frames++;
    }

    return error;
}

static void message_handler(void *args)
{
    received_frame_t *rx_frame = (received_frame_t *)args;
    hal_context_t *hal;

    assert(rx_frame);
    if (is_mesh_header(*message_get_payload((message_t *)rx_frame->message))) {
        handle_mesh(rx_frame);
    } else {
        hal = rx_frame->hal;
        hal->link_stats.in_filterings++;
        message_free((message_t *)rx_frame->message);
    }
    ur_mem_free(rx_frame, sizeof(received_frame_t));
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

    if (error == 0 && ((message = message_alloc(NULL, frame->len)) != NULL)) {
        rx_frame = (received_frame_t *)ur_mem_alloc(sizeof(received_frame_t));
        if (rx_frame == NULL) {
            hal->link_stats.in_filterings++;
            message_free(message);
            return;
        }
        message_copy_from(message, frame->data, frame->len);
        rx_frame->message = message;
        memcpy(&rx_frame->frame_info, frame_info, sizeof(rx_frame->frame_info));
        rx_frame->hal = hal;
        yoc_schedule_call(message_handler, rx_frame);
    }
}

static ur_error_t send_data(hal_context_t *hal) {
    int16_t           offset = 0;
    uint8_t           ip_hdr_len;
    uint8_t           lowpan_hdr_len;
    uint8_t           *ip_payload;
    uint8_t           *lowpan_payload;
    mesh_dest_t       *dest;
    network_context_t *network;
    ur_error_t        error;
    message_t         *message;

    hal->sending_flags |= SENDING_DATA;
    message = hal->send_message;
    dest = message->dest;
    network = message->network;

    if (dest->flags & INSERT_LOWPAN_FLAG) {
        if (dest->flags & ENABLE_COMPRESS_FLAG) {
            ip_payload = (uint8_t *)ur_mem_alloc((UR_IP6_HLEN + UR_UDP_HLEN) * 2);
            if (ip_payload == NULL) {
                hal->link_stats.out_errors++;
                return UR_ERROR_FAIL;
            }
            if (message_get_msglen(message) >= (UR_IP6_HLEN + UR_UDP_HLEN)) {
                message_copy_to(message, 0, ip_payload, UR_IP6_HLEN + UR_UDP_HLEN);
            } else if (message_get_msglen(message) >= UR_IP6_HLEN) {
                message_copy_to(message, 0, ip_payload, UR_IP6_HLEN);
            }
            lowpan_payload = ip_payload + UR_IP6_HLEN + UR_UDP_HLEN;
            lp_header_compress(ip_payload, lowpan_payload, &ip_hdr_len, &lowpan_hdr_len);
            offset = ip_hdr_len - lowpan_hdr_len;

            offset -= 1;
            *(--lowpan_payload) = LOWPAN_IPHC_DISPATCH;
            message_set_payload_offset(message, -offset);
            message_copy_from(message, lowpan_payload, ip_hdr_len - offset);
            ur_mem_free(ip_payload, (UR_IP6_HLEN + UR_UDP_HLEN) * 2);
        } else {
            offset = 1;
            message_set_payload_offset(message, offset);
            lowpan_payload = message_get_payload(message);
            *lowpan_payload = UNCOMPRESSED_DISPATCH;
        }
        dest->flags &= (~(INSERT_LOWPAN_FLAG | ENABLE_COMPRESS_FLAG));
    }

    if (dest->flags & INSERT_MCAST_FLAG) {
        offset = sizeof(mcast_header_t) + 1;
        message_set_payload_offset(message, offset);
        lowpan_payload = message_get_payload(message);
        *lowpan_payload = MCAST_HEADER_DISPATCH;
        insert_mcast_header(network, lowpan_payload + 1);
        dest->flags &= (~INSERT_MCAST_FLAG);
    }

    error = send_fragment(network, message);
    if (error == UR_ERROR_NONE) {
        hal->link_stats.out_data++;
    }
    return error;
}

static ur_error_t send_cmd(hal_context_t *hal) {
    network_context_t *network;
    ur_error_t        error = UR_ERROR_FAIL;
    message_t         *message;

    hal->sending_flags |= SENDING_CMD;
    message = hal->send_message;
    network = message->network;
    if (network == NULL) {
        return error;
    }
    error = send_mesh(network, message, 0, message_get_msglen(message), NULL, 0);
    if (error == UR_ERROR_NONE) {
        hal->link_stats.out_command++;
    }
    return error;
}

static bool is_sending_available(hal_context_t *hal, message_t *message)
{
    bool ucast = true;

    if (is_bcast_address(&message->dest->dest)) {
        ucast = false;
    }
    if ((ucast && (hal->sending_flags & SENDING_UCAST)) ||
        (ucast == false && (hal->sending_flags & SENDING_BCAST))) {
        return false;
    }

    return true;
}

static hal_context_t *get_send_message(void)
{
    slist_t       *hals;
    hal_context_t *hal;
    hal_context_t *hal_to_send = NULL;
    message_t     *message = NULL;
    message_t     *message_to_send = NULL;
    uint32_t      last_tx_time = 0;
    bool          is_reset_offset = false;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        message = hal->send_message;
        if (message) {
            hal_to_send = NULL;
            is_reset_offset = false;
            if (is_sending_available(hal, message)) {
                hal_to_send = hal;
            }
            break;
        }
        message = message_queue_get_head(&hal->send_queue[CMD_QUEUE]);
        if (message == NULL) {
            message = message_queue_get_head(&hal->send_queue[DATA_QUEUE]);
        }
        if (message == NULL || is_sending_available(hal, message) == false) {
            continue;
        }
        if (last_tx_time == 0 || last_tx_time < hal->last_tx_time) {
            last_tx_time = hal->last_tx_time;
            hal_to_send = hal;
            message_to_send = message;
            is_reset_offset = true;
        }
    }

    if (hal_to_send == NULL) {
        return NULL;
    }
    if (is_reset_offset) {
        g_frag_info.offset = 0;
        hal_to_send->send_message = message_to_send;
    }

    return hal_to_send;
}

static void send_datagram(void *args)
{
    hal_context_t *hal = NULL;
    message_t     *message = NULL;
    ur_error_t    error;
    network_context_t *network;
    mesh_src_t    src;

    hal = get_send_message();
    if (hal == NULL) {
        return;
    }
    message = hal->send_message;
    network = message->network;
    src.sid = mm_get_local_sid();
    src.meshnetid = mm_get_meshnetid(network);
    error = check_reachability(network, &src, message->dest);
    if (error == UR_ERROR_NONE) {
        if (message->dest->flags & TYPE_DATA_FLAG) {
            error = send_data(hal);
        } else {
            error = send_cmd(hal);
        }
    }

    if (error != UR_ERROR_NONE) {
        hal->last_sent = SENT_FAIL;
        yoc_schedule_call(message_sent_task, hal);
    }
}

static ur_error_t handle_datagram(network_context_t *network,
                                  message_t *message, mesh_header_t *mesh_header)
{
    ur_error_t        error = UR_ERROR_NONE;
    int16_t           offset;
    mesh_dest_t       *dest;
    message_t         *relay_message;
    message_t         *input_message = NULL;
    uint8_t           *nexth;
    slist_t           *hals;
    hal_context_t     *hal;
    network_context_t *relay_network;

    nexth = message_get_payload(message);
    message_set_payload_offset(message, -1);
    if (is_mcast_header(*nexth)) {
        error = process_mcast_header(network, message_get_payload(message));
        if (error != UR_ERROR_NONE) {
            return UR_ERROR_DROP;
        }
        dest = message->dest;
        message_set_payload_offset(message, 1);
        if (dest != NULL) {
            dest->dest.len = 2;
            dest->dest.short_addr = BCAST_SID;
            dest->meshnetid = mm_get_meshnetid(network);
            dest->flags = TYPE_DATA_FLAG | INSERT_MESH_HEADER | INSERT_DESTNETID_FLAG;

            hals = get_hal_contexts();
            slist_for_each_entry(hals, hal, hal_context_t, next) {
                if (hal == network->hal) {
                    relay_network = network;
                } else {
                    relay_network = get_hal_default_network_context(hal);
                }
                relay_message = message_alloc(relay_network, message_get_msglen(message));
                if (relay_message != NULL) {
                    message_copy(relay_message, message);
                    relay_message->network = relay_network;
                    dest = relay_message->dest;
                    dest->meshnetid = mm_get_meshnetid(relay_network);
                    message_queue_enqueue(&hal->send_queue[DATA_QUEUE], relay_message);
                    hal->last_tx_time = ur_get_now();
                    yoc_schedule_call(send_datagram, NULL);
                }
            }
        }
        offset = sizeof(mcast_header_t) + 1;
        message_set_payload_offset(message, -offset);
        nexth = message_get_payload(message);
        message_set_payload_offset(message, -1);
    }

    if (is_uncompressed(*nexth)) {
        input_message = message;
    } else if (is_lowpan_iphc(*nexth)) {
        input_message = handle_lowpan_iphc(message, mesh_header);
    } else {
        error = UR_ERROR_DROP;
    }

    if (error == UR_ERROR_NONE && input_message) {
        error = ur_mesh_input(input_message);
    } else {
        error = UR_ERROR_DROP;
    }
    return error;
}

const ur_link_stats_t *mf_get_stats(void) {
    hal_context_t *hal;

    hal = get_default_hal_context();
    if (hal == NULL) {
        return NULL;
    }

    hal->link_stats.bcast_sending = hal->sending_flags & SENDING_BCAST;
    hal->link_stats.ucast_sending = hal->sending_flags & SENDING_UCAST;
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
        ur_stop_timer(&hal->bcast_sending_timer, hal);
        ur_stop_timer(&hal->ucast_sending_timer, hal);
    }

    g_frag_info.tag = 0;
    g_frag_info.offset = 0;

    return UR_ERROR_NONE;
}

ur_error_t mf_deinit(void)
{
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        ur_stop_timer(&hal->bcast_sending_timer, hal);
        ur_stop_timer(&hal->ucast_sending_timer, hal);
    }

    return UR_ERROR_NONE;
}
