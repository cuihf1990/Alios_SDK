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

#include "utilities/message.h"
#include "ipv6/ip6.h"
#include "hal/interface_context.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    UNCOMPRESSED_DISPATCH      = 0x41,
    LOWPAN_IPHC_DISPATCH       = 0x60,
    LOWPAN_IPHC_DISPATCH_MASK  = 0xe0,
    MCAST_HEADER_DISPATCH      = 0x48,
    MCAST_HEADER_DISPATCH_MASK = 0xfe,
    MESH_HEADER_DISPATCH       = 0x80,
    MESH_HEADER_DISPATCH_MASK  = 0xc0,
    FRAG_HEADER_DISPATCH       = 0xc0,
    FRAG_N_FLAG                = 0x20,
    FRAG_HEADER_DISPATCH_MASK  = 0xc0,
};

/* mesh header control field format */
/* 2bits | 1bit | 3bits | 2bits |    2bits   | 2bits | 1bit | 2bits | 1 bit */
/* flags | type | hops  |  src  | dest netid |  dest |  dir | dest2 |  sec  */

enum {
    MESH_FRAME_TYPE_MASK       = 0x20,
    MESH_FRAME_TYPE_OFFSET     = 5,

    MESH_HOPS_LEFT_MASK        = 0x1c,
    MESH_HOPS_LEFT_OFFSET      = 2,

    MESH_HEADER_SRC_MASK       = 0x03,
    MESH_HEADER_SRC_OFFSET     = 0,

    MESH_HEADER_DESTNETID_MASK = 0xc0,
    MESH_HEADER_DESTNETID_OFFSET = 6,
    MESH_HEADER_DEST_MASK      = 0x30,
    MESH_HEADER_DEST_OFFSET    = 4,
    MESH_HEADER_DIR_MASK       = 0x80,
    MESH_HEADER_DIR_OFFSET     = 3,
    MESH_HEADER_DEST2_MASK     = 0x06,
    MESH_HEADER_DEST2_OFFSET   = 1,
    MESH_HEADER_SEC_MASK       = 0x01,
    MESH_HEADER_SEC_OFFSET     = 0,
};

enum {
    MESH_FRAME_TYPE_DATA = 0x00,
    MESH_FRAME_TYPE_CMD  = 0x01,
};

enum {
    NO_ADDR_MODE    = 0,
    SHORT_ADDR_MODE = 1,
    EXT_ADDR_MODE   = 2,
    BCAST_ADDR_MODE = 3,
};

enum {
    DIR_UP   = 0,
    DIR_DOWN = 1,
};

enum {
    DISABLE_SEC = 0,
    ENABLE_SEC = 1,
};

enum {
    NO_DEST_NETID    = 0,
    BCAST_DEST_NETID = 1,
    SUB_DEST_NETID   = 2,
    DEST_NETID       = 3,
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

typedef struct mesh_header_control_s {
    uint8_t control[2];
} __attribute__((packed)) mesh_header_control_t;

typedef struct mesh_short_addr_s {
    uint16_t addr;
} __attribute__((packed)) mesh_short_addr_t;

typedef struct mesh_ext_addr_s {
    uint8_t addr[EXT_ADDR_SIZE];
} __attribute__((packed)) mesh_ext_addr_t;

typedef struct mesh_netid_s {
    uint16_t netid;
} __attribute__((packed)) mesh_netid_t;

typedef struct mesh_subnetid_s {
    uint8_t netid;
} __attribute__((packed)) mesh_subnetid_t;

typedef struct mesh_header_frame_counter_s {
    uint32_t frame_counter;
} __attribute__((packed)) mesh_header_frame_counter_t;

enum {
    INSERT_LOWPAN_FLAG   = 1 << 5,
    ENABLE_COMPRESS_FLAG = 1 << 6,
    INSERT_MCAST_FLAG    = 1 << 7,
    ENCRYPT_ENABLE_FLAG  = 1 << 9,

    INSERT_MESH_HEADER   = 1 << 15,
};

ur_error_t mf_init(void);
ur_error_t mf_deinit(void);
ur_error_t mf_send_message(message_t *message);
ur_error_t mf_resolve_dest(const ur_ip6_addr_t *dest, ur_addr_t *dest_addr);
const ur_link_stats_t *mf_get_stats(void);

#ifdef __cplusplus
}
#endif

#endif  /* UR_MESH_FORWARDER_H */
