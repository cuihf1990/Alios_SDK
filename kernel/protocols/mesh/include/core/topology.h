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

#ifndef UR_TOPOLOGY_H
#define UR_TOPOLOGY_H

#include "umesh_types.h"

enum {
    MAX_NEIGHBORS_NUM = 32,
};

#define INVALID_UEID   "\xff\xff\xff\xff\xff\xff\xff\xff"

enum {
    BCAST_NETID   = 0xffff,
    BCAST_SUB_NETID = 0xff,
    INVALID_NETID = 0xfffe,
};

#define is_bcast_netid(nid) ((nid) == BCAST_NETID)
#define is_bcast_subnetid(nid) ((nid) == BCAST_SUB_NETID)
#define is_invalid_netid(nid) ((nid) == INVALID_NETID)

enum {
    INFINITY_PATH_COST = 0xffff,
};

typedef struct link_nbr_stats_s {
    uint32_t last_heard;
    uint16_t link_request;
    uint16_t link_accept;
    uint16_t link_cost;
    int8_t   forward_rssi;
    int8_t   reverse_rssi;
} link_nbr_stats_t;

typedef enum neighbor_state_s {
    STATE_INVALID,
    STATE_CANDIDATE,
    STATE_NEIGHBOR,
    STATE_PARENT,
    STATE_CHILD,
} neighbor_state_t;

enum {
    NBR_SID_CHANGED       = 1 << 0,
    NBR_DISCOVERY_REQUEST = 1 << 1,
    NBR_NETID_CHANGED     = 1 << 2,
};

typedef enum node_type_s {
    LEAF_NODE      = 0x01,
    ROUTER_NODE    = 0x02,
} sid_node_type_t;

typedef struct ur_node_id_s {
    uint8_t         ueid[EXT_ADDR_SIZE];
    node_mode_t     mode;
    uint16_t        meshnetid;
    uint16_t        sid;
    uint16_t        attach_sid;
    sid_node_type_t type;
    uint8_t         timeout;
} ur_node_id_t;

typedef struct ssid_info_s {
    uint16_t child_num;
    uint8_t  free_slots;
} ssid_info_t;

typedef struct neighbor_s {
    slist_t next;
    void *hal;
    uint8_t ueid[EXT_ADDR_SIZE];
    mac_address_t mac;
    ur_addr_t addr;
    uint16_t path_cost;
    neighbor_state_t state: 4;
    node_mode_t mode;
    link_nbr_stats_t stats;
    uint8_t flags;
    uint8_t attach_candidate_timeout;
    ssid_info_t ssid_info;
    uint8_t channel;
    uint32_t last_heard;
} neighbor_t;

#define ueid64(ueid) ({ \
    uint64_t id64 = ueid[7];     \
    id64 <<= 8; id64 |= ueid[6]; \
    id64 <<= 8; id64 |= ueid[5]; \
    id64 <<= 8; id64 |= ueid[4]; \
    id64 <<= 8; id64 |= ueid[3]; \
    id64 <<= 8; id64 |= ueid[2]; \
    id64 <<= 8; id64 |= ueid[1]; \
    id64 <<= 8; id64 |= ueid[0]; \
    id64; \
})
#endif  /* UR_TOPOLOGY_H */
