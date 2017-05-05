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


#ifndef UR_MESH_FORWARDER_H
#define UR_MESH_FORWARDER_H

#include "message.h"
#include "ip6.h"
#include "interface_context.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    UNCOMPRESSED_DISPATCH      = 0x41,
    LOWPAN_IPHC_DISPATCH       = 0x60,
    LOWPAN_IPHC_DISPATCH_MASK  = 0xe0,
    MM_HEADER_DISPATCH         = 0x46,
    MM_HEADER_DISPATCH_MASK    = 0xfe,
    MCAST_HEADER_DISPATCH      = 0x48,
    MCAST_HEADER_DISPATCH_MASK = 0xfe,
    MESH_HEADER_DISPATCH       = 0x80,
    MESH_HEADER_DISPATCH_MASK  = 0xc0,
    FRAG_HEADER_DISPATCH       = 0xc0,
    FRAG_N_FLAG                = 0x20,
    FRAG_HEADER_DISPATCH_MASK  = 0xc0,
};

enum {
    MESH_HOPS_LEFT_MASK        = 0x07,

    MESH_FRAME_TYPE_MASK       = 0x08,
    MESH_FRAME_TYPE_DATA       = 0x00,
    MESH_FRAME_TYPE_CMD        = 0x08,

    MESH_CONTROL_MASK          = 0x30,
    MESH_HEADER_RELAYER        = 0x10,
    MESH_HEADER_DESTNETID      = 0x20,

    MESH_CONTROL_EXT_MASK      = 0xf0,
    MESH_HEADER_BCASTDESTNETID = 0x10,
    MESH_HEADER_BCASTDESTSID   = 0x20,
    MESH_HEADER_ENABLE_ENCRYPT = 0x40,
};

enum {
    FORWARD_HOP_LIMIT = 6,
};

enum {
    MAIN_NETID_MASK = 0xff00,
    SUB_NETID_MASK = 0x00ff,
};

#define get_main_netid(nid) ((nid) & MAIN_NETID_MASK)
#define get_sub_netid(nid) ((nid) & SUB_NETID_MASK)
#define mk_sub_netid(m, s) ((uint16_t)(get_main_netid(m) | (s)))
#define is_subnet(nid) ((nid) & SUB_NETID_MASK)
#define get_leader_sid(nid) (((nid) & SUB_NETID_MASK) >> 2)
#define is_same_subnet(nid1, nid2) (get_sub_netid(nid1) == get_sub_netid(nid2))
#define is_same_mainnet(nid1, nid2) (get_main_netid(nid1) == get_main_netid(nid2))

typedef struct mesh_header_s {
    uint8_t  control;
    uint8_t control_ext;
    uint16_t meshnetid;
    uint16_t src;
} __attribute__((packed)) mesh_header_t;

typedef struct mesh_header_destsid_s {
    uint16_t dest;
} __attribute__((packed)) mesh_header_destsid_t;

typedef struct mesh_header_destnetid_s {
    uint8_t destsubnetid;
} __attribute__((packed)) mesh_header_destnetid_t;

typedef struct mesh_header_relayer_s {
    uint16_t relayer;
} __attribute__((packed)) mesh_header_relayer_t;

typedef struct mesh_header_frame_counter_s {
    uint32_t frame_counter;
} __attribute__((packed)) mesh_header_frame_counter_t;

typedef struct mesh_src_s {
    hal_context_t *hal;
    network_context_t *dest_network;
    mac_address_t src;
    uint16_t      meshnetid;
    uint16_t      sid;
    uint8_t       channel;
} mesh_src_t;

ur_error_t mf_init(void);
ur_error_t mf_deinit(void);
ur_error_t mf_send_ip6(message_t *message, const ur_ip6_addr_t *dest);
ur_error_t mf_send_command(uint8_t command, message_t *message);
ur_error_t mf_resolve_dest(const ur_ip6_addr_t *dest, sid_t *dest_sid);

const ur_link_stats_t *mf_get_stats(void);

#ifdef __cplusplus
}
#endif

#endif  /* UR_MESH_FORWARDER_H */
