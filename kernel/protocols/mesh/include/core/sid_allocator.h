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

#ifndef UR_SID_ALLOCATOR_H
#define UR_SID_ALLOCATOR_H

#include "core/mesh_mgmt.h"
#include "hal/interface_context.h"
#include "core/router_mgr.h"

enum {
    STRUCTURED_SID = 0,
    SHORT_RANDOM_SID = 1,
    RANDOM_SID = 2,
};

enum {
    SID_LEN      = 16,
    SID_MASK     = 0xf,
    SID_MASK_LEN = 4,
};

enum {
    MOBILE_PREFIX   = 0xc,
    LOWPOWER_PREFIX = 0xd,
};

enum {
    PF_SID_PREFIX_MASK   = 0xf,
    PF_SID_PREFIX_OFFSET = 12,
};

enum {
    PF_NODE_NUM = 128,
    RSID_NUM = 2048,
};

typedef struct sid_node_s {
    slist_t      next;
    ur_node_id_t node_id;
    media_type_t type;
} sid_node_t;

typedef struct ssid_allocator_s {
    sid_base_t base;
    uint16_t sid_shift;
    uint16_t sid_prefix;
    uint32_t attach_free_bits;
    uint16_t pf_node_num;
    uint32_t mobile_free_bits[(PF_NODE_NUM + 31) / 32];
} ssid_allocator_t;

typedef struct rsid_allocator_s {
    sid_base_t base;
    uint32_t free_bits[(RSID_NUM + 31) / 32];
} rsid_allocator_t;

// structured sid
void allocator_init(network_context_t *network);
void allocator_deinit(network_context_t *network);
ur_error_t allocate_sid(network_context_t *network, ur_node_id_t *node_id);
void free_sid(network_context_t *network, uint16_t sid);
ur_error_t update_sid_mapping(network_context_t *network,
                              ur_node_id_t *node_id, bool to_add);

uint16_t   get_allocated_number(network_context_t *network);
uint32_t   get_allocated_bitmap(network_context_t *network);
uint16_t   get_allocated_pf_number(network_context_t *network);
uint16_t   get_free_number(network_context_t *network);
slist_t    *get_ssid_nodes_list(network_context_t *network);
bool       is_direct_child(network_context_t *network, uint16_t sid);
bool       is_allocated_child(network_context_t *network, neighbor_t *nbr);
bool       is_partial_function_sid(uint16_t sid);

// random sid
void rsid_allocator_init(network_context_t *network);
void rsid_allocator_deinit(network_context_t *network);
ur_error_t rsid_allocate_sid(network_context_t *network, ur_node_id_t *node_id);
ur_error_t rsid_free_sid(network_context_t *network, ur_node_id_t *node_id);
uint16_t rsid_get_allocated_number(network_context_t *network);

static inline int find_first_free_bit(uint32_t *bits, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        if (!(bits[i / 32] & (1 << (i % 32)))) {
            continue;
        }

        bits[i / 32] &= ~(1 << (i % 32));
        return i;
    }

    return -1;
}

static inline ur_error_t grab_free_bit(uint32_t *bits, uint32_t len,
                                       uint16_t index)
{
    if (index >= len) {
        return UR_ERROR_FAIL;
    }
    if (bits[index / 32] & (1 << (index % 32))) {
        bits[index / 32] &= ~(1 << (index % 32));
        return UR_ERROR_NONE;
    }
    return UR_ERROR_FAIL;
}

static inline bool release_bit(uint32_t *bits, int len, int i)
{
    if (i >= len) {
        return false;
    }
    if (bits[i / 32] & (1 << (i % 32))) {
        return false;
    }
    bits[i / 32] |= 1 << (i % 32);
    return true;
}

#endif  /* UR_SID_ALLOCATOR_H */
