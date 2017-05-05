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

#include "mesh_mgmt.h"
#include "interface_context.h"

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

typedef struct sid_node_s {
    slist_t  next;
    uint16_t meshnetid;
    uint8_t  ueid[8];
    uint16_t sid;
    uint16_t attach_sid;
    uint16_t attach_netid;
    uint8_t timeout;
    media_type_t type;
} sid_node_t;

// structured sid
void allocator_init(const uint8_t *ueid, uint16_t mysid);
void allocator_deinit(void);
ur_error_t allocate_sid(uint8_t *ueid, uint16_t attach_node_sid, node_mode_t mode, sid_t *sid);
void free_sid(uint8_t *ueid, uint16_t attach_node_sid, uint16_t sid);
ur_error_t update_sid_mapping(uint8_t *ueid, uint16_t sid,
                              uint16_t attach_node_sid, bool to_add);

uint32_t   get_allocated_bitmap(void);
uint16_t   get_allocated_number(void);
uint16_t   get_allocated_pf_number(void);
uint16_t   get_free_number(void);
uint16_t   get_attach_sid(uint16_t sid);
slist_t    *get_ssid_nodes_list(void);
bool       is_partial_function_sid(uint16_t sid);
bool       is_direct_child(uint16_t sid);

// random sid
void rsid_allocator_init(void);
void rsid_allocator_deinit(void);
ur_error_t rsid_allocate_sid(uint8_t *ueid, uint8_t type, sid_t *sid);
ur_error_t rsid_free_sid(uint8_t *ueid, uint8_t type, uint16_t sid);
uint16_t rsid_get_allocated_number(void);

static inline int find_first_free_bit(uint32_t *bits, int len)
{
    int i;
    for (i=0;i<len;i++) {
        if (!(bits[i/32] & (1 << (i%32))))
            continue;

        bits[i/32] &= ~(1 << (i%32));
        return i;
    }

    return -1;
}

static inline ur_error_t grab_free_bit(uint32_t *bits, uint32_t len, uint16_t index)
{
    if (index >= len) {
        return UR_ERROR_FAIL;
    }
    if (bits[index/32] & (1 << (index%32))) {
        bits[index/32] &= ~(1 << (index%32));
        return UR_ERROR_NONE;
    }
    return UR_ERROR_FAIL;
}

static inline bool release_bit(uint32_t *bits, int len, int i)
{
    if (i >= len)
        return false;
    if (bits[i/32] & (1 << (i%32)))
        return false;
    bits[i/32] |= 1 << (i%32);
    return true;
}

#endif  /* UR_SID_ALLOCATOR_H */
