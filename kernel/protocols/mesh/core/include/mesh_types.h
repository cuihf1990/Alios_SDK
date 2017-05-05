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

#ifndef UR_MESH_TYPES_H
#define UR_MESH_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#include <hal/mesh.h>

#ifndef NULL
#define NULL (void *)0
#endif

typedef enum ur_error_s {
   UR_ERROR_NONE          = 0,
   UR_ERROR_FAIL          = 1,
   UR_ERROR_BUSY          = 2,
   UR_ERROR_DROP          = 3,
   UR_ERROR_MEM           = 4,
   UR_ERROR_ROUTE         = 5,
   UR_ERROR_PARSE         = 6,
   UR_ERROR_ADDRESS_QUERY = 7,
} ur_error_t;

typedef struct mesh_ipv6_addr_s {
    uint8_t  addr[16];
} mesh_ipv6_addr_t;

typedef struct mesh_ipv6_prefix_s {
    uint8_t addr[16];
    uint8_t len;
} mesh_ipv6_prefix_t;

enum {
    TYPE_COMMAND_FLAG     = 1 << 0,
    TYPE_DATA_FLAG        = 1 << 1,

    INSERT_DESTNETID_FLAG = 1 << 4,
    INSERT_LOWPAN_FLAG    = 1 << 5,
    ENABLE_COMPRESS_FLAG  = 1 << 6,
    INSERT_MCAST_FLAG     = 1 << 7,
    RELAYERSID_FLAG       = 1 << 8,
    ENCRYPT_ENABLE_FLAG   = 1 << 9,

    INSERT_MESH_HEADER    = 1 << 15,
};

typedef struct mesh_dest_s {
    mac_address_t dest;
    uint16_t      meshnetid;
    uint16_t      relayersid;
    uint16_t      flags;
    uint8_t       hop_limit;
} mesh_dest_t;

typedef enum node_type_s {
    LEAF_NODE      = 0x01,
    ROUTER_NODE    = 0x02,
} sid_node_type_t;

typedef struct sid_s {
    uint16_t        meshnetid;
    uint16_t        sid;
    sid_node_type_t type;
} sid_t;

typedef struct ur_link_stats_s {
    uint32_t in_frames;
    uint32_t in_command;
    uint32_t in_data;
    uint32_t in_filterings;
    uint32_t in_drops;

    uint32_t out_frames;
    uint32_t out_command;
    uint32_t out_data;
    uint32_t out_errors;

    bool     bcast_sending;
    bool     ucast_sending;
    uint16_t bcast_sending_timeouts;
    uint16_t ucast_sending_timeouts;
} ur_link_stats_t;

typedef struct ur_node_id_s {
    uint8_t ueid[8];
    uint16_t meshnetid;
    uint16_t sid;
} ur_node_id_t;

enum {
    NETWORK_CONTEXT_NUM = 5,
};

#endif  /* UR_MESH_TYPES_H */
