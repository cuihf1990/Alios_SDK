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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mesh_types.h"
#include "sid_allocator.h"
#include "link_mgmt.h"
#include "topology.h"
#include "logging.h"
#include "memory.h"
#include "interfaces.h"

#define LEADER_DEF_BITMAP 0x0ffe
#define ROUTER_DEF_BITMAP 0xfffe

typedef struct ssid_allocator_state_s {
    slist_t  node_list;
    uint16_t node_num;
    uint16_t pf_node_num;
    uint16_t sid_shift;
    uint16_t sid_prefix;
    uint32_t attach_free_bits;
    uint32_t mobile_free_bits[(PF_NODE_NUM + 31) / 32];
} ssid_allocator_state_t;

static ssid_allocator_state_t g_sa_state;

void allocator_init(const uint8_t *ueid, uint16_t mysid)
{
    int i;

    slist_init(&g_sa_state.node_list);
    set_state_to_neighbor();

    g_sa_state.node_num = 0;
    g_sa_state.pf_node_num = 0;

    if (mysid == LEADER_SID)
        g_sa_state.attach_free_bits = LEADER_DEF_BITMAP;
    else
        g_sa_state.attach_free_bits = ROUTER_DEF_BITMAP;

    memset(g_sa_state.mobile_free_bits, 0xff, sizeof(g_sa_state.mobile_free_bits));
    g_sa_state.mobile_free_bits[0] -= 1;

    for (i=0;i<SID_LEN;i+=SID_MASK_LEN) {
        uint16_t mask = (1 << SID_MASK_LEN) - 1;
        mask <<= i;
        if (mysid & mask)
            break;
    }

    g_sa_state.sid_shift = i - SID_MASK_LEN;
    g_sa_state.sid_prefix = mysid;
}

void allocator_deinit(void)
{
    sid_node_t *node;

    while (!slist_empty(&g_sa_state.node_list)) {
        node = slist_first_entry(&g_sa_state.node_list, sid_node_t, next);
        slist_del(&node->next, &g_sa_state.node_list);
        ur_mem_free(node, sizeof(sid_node_t));
    }
    g_sa_state.node_num = 0;
    g_sa_state.pf_node_num = 0;
}

bool is_direct_child(uint16_t sid)
{
    uint16_t sidmask = 0;
    uint8_t  index;

    if (sid == LEADER_SID || sid == INVALID_SID)
        return false;

    index = SID_LEN - SID_MASK_LEN;
    while (index > g_sa_state.sid_shift) {
        sidmask |= (SID_MASK << index);
        index -= SID_MASK_LEN;
    }
    if (g_sa_state.sid_prefix != (sid & sidmask ) ||
        g_sa_state.sid_prefix == sid) {
        return false;
    }
    sidmask = (1 << g_sa_state.sid_shift) - 1;
    if (sid & sidmask) {
        return false;
    }

    return true;
}

static bool is_valid_sid(neighbor_t *node)
{
    uint16_t sid = node->sid;
    if (node->state == STATE_INVALID)
        return false;
    if (node->sid == INVALID_SID)
        return false;
    if (node->netid != mm_get_meshnetid(NULL)) {
        return false;
    }
    if (is_partial_function_sid(node->sid)) {
        return true;
    }

    return is_direct_child(sid);
}

ur_error_t update_sid_mapping(uint8_t *ueid, uint16_t sid,
                              uint16_t attach_node_sid, bool to_add)
{
    sid_node_t *node;
    sid_node_t *new_node = NULL;

    slist_for_each_entry(&g_sa_state.node_list, node, sid_node_t, next) {
        if (memcmp(node->ueid, ueid, sizeof(node->ueid)) == 0) {
            new_node = node;
            break;
        }
    }

    if (to_add == false) {
        if (node) {
            free_sid(ueid, attach_node_sid, sid);
            slist_del(&node->next, &g_sa_state.node_list);
        }
        return UR_ERROR_NONE;
    }

    if (new_node) {
        if (sid != INVALID_SID && sid != new_node->sid) {
            free_sid(ueid, new_node->attach_sid, new_node->sid);
        }
    } else {
        new_node = (sid_node_t *)ur_mem_alloc(sizeof(sid_node_t));
        if (!new_node)
            return UR_ERROR_MEM;
        slist_add(&new_node->next, &g_sa_state.node_list);
    }

    new_node->sid = sid;
    new_node->attach_sid = attach_node_sid;
    memcpy(new_node->ueid, ueid, sizeof(new_node->ueid));

    return UR_ERROR_NONE;
}

void free_sid(uint8_t *ueid, uint16_t attach_node_sid, uint16_t sid)
{
    uint8_t len;

    if (is_partial_function_sid(sid)) {
        uint16_t idx = sid - (MOBILE_PREFIX << PF_SID_PREFIX_OFFSET);
        if (release_bit(g_sa_state.mobile_free_bits, PF_NODE_NUM, idx))
            g_sa_state.pf_node_num --;
        return;
    }

    if (!is_direct_child(sid)) {
        return;
    }

    sid -= g_sa_state.sid_prefix;
    sid >>= g_sa_state.sid_shift;
    len = 1 << SID_MASK_LEN;
    if (g_sa_state.sid_prefix == LEADER_SID) {
        len = 12;
    }
    if (release_bit(&g_sa_state.attach_free_bits, len, sid)) {
        g_sa_state.node_num --;
    }
}

static ur_error_t allocate_expected_sid(uint8_t *ueid, uint16_t sid,
                                        uint16_t attach_sid, node_mode_t mode)
{
    uint8_t index;
    uint8_t len;

    if (is_direct_child(sid)) {
        index = (sid - g_sa_state.sid_prefix) >> g_sa_state.sid_shift;
        len = 16;
        if (g_sa_state.sid_prefix == LEADER_SID) {
            len = 12;
        }
        if ((index > len) ||
            (grab_free_bit(&g_sa_state.attach_free_bits, len, index) == UR_ERROR_FAIL)) {
            return UR_ERROR_FAIL;
        }
        g_sa_state.node_num++;
    } else if (is_partial_function_sid(sid)) {
        index = sid - (MOBILE_PREFIX << PF_SID_PREFIX_OFFSET);
        if (index > PF_NODE_NUM) {
            return UR_ERROR_FAIL;
        }
        if (grab_free_bit(g_sa_state.mobile_free_bits, PF_NODE_NUM, index) == UR_ERROR_FAIL) {
            return UR_ERROR_FAIL;
        }
        if (update_sid_mapping(ueid, sid, attach_sid, true) != UR_ERROR_NONE) {
            release_bit(g_sa_state.mobile_free_bits, PF_NODE_NUM, sid);
            return UR_ERROR_FAIL;
        }
        g_sa_state.pf_node_num++;
    } else {
        return UR_ERROR_FAIL;
    }
    return UR_ERROR_NONE;
}

ur_error_t allocate_sid(uint8_t *ueid, uint16_t attach_node_sid,
                        node_mode_t mode, sid_t *sid)
{
    neighbor_t *node = NULL;
    sid_node_t *sid_node = NULL;
    int        newsid = -1;

    node = get_neighbor_by_ueid(ueid);
    if (!node) {
        goto new_sid;
    }
    if (!is_valid_sid(node)) {
        goto new_sid;
    }

    if (sid->sid != INVALID_SID &&
        allocate_expected_sid(ueid, sid->sid, attach_node_sid, mode) == UR_ERROR_FAIL) {
        goto new_sid;
    }

    if (sid->sid == INVALID_SID || sid->sid == node->sid) {
        sid->sid = node->sid;
    }

    sid->type = g_sa_state.sid_shift == 0 ? LEAF_NODE : ROUTER_NODE;
    return UR_ERROR_NONE;

new_sid:
    if (mode & MODE_MOBILE) {
        slist_for_each_entry(&g_sa_state.node_list, sid_node, sid_node_t, next) {
            if (memcmp(sid_node->ueid, ueid, sizeof(sid_node->ueid)) == 0) {
                break;
            }
        }
        sid->type = LEAF_NODE;
        if (sid_node == NULL) {
            newsid = find_first_free_bit(g_sa_state.mobile_free_bits, PF_NODE_NUM);
            sid->sid = ((uint16_t)newsid) | (MOBILE_PREFIX << PF_SID_PREFIX_OFFSET);
        } else {
            sid->sid = sid_node->sid;
        }
    } else {
        if (g_sa_state.sid_prefix == LEADER_SID) {
            newsid = find_first_free_bit(&g_sa_state.attach_free_bits, 12);
        } else {
            newsid = find_first_free_bit(&g_sa_state.attach_free_bits, 16);
        }
        sid->sid = g_sa_state.sid_prefix | (((uint16_t)newsid) << g_sa_state.sid_shift);
        sid->type = g_sa_state.sid_shift == 0 ? LEAF_NODE : ROUTER_NODE;
    }

    if (newsid < 0) {
        return UR_ERROR_MEM;
    }
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
            "allocate 0x%04x\r\n", sid->sid);

    if (!(mode & MODE_MOBILE)) {
        g_sa_state.node_num++;
        goto out;
    }

    if (UR_ERROR_NONE != update_sid_mapping(ueid, sid->sid, attach_node_sid, true)) {
        release_bit(g_sa_state.mobile_free_bits, PF_NODE_NUM, newsid);
        return UR_ERROR_FAIL;
    }
    g_sa_state.pf_node_num++;
out:
    return UR_ERROR_NONE;
}

uint32_t get_allocated_bitmap(void)
{
    if (g_sa_state.sid_prefix == LEADER_SID)
        return LEADER_DEF_BITMAP & ~g_sa_state.attach_free_bits;

    return ROUTER_DEF_BITMAP & ~g_sa_state.attach_free_bits;
}

uint16_t get_allocated_number(void)
{
    return g_sa_state.node_num;
}

uint16_t get_allocated_pf_number(void)
{
    return g_sa_state.pf_node_num;
}

uint16_t get_free_number(void)
{
    if (g_sa_state.sid_prefix == LEADER_SID)
        return 11 - g_sa_state.node_num;
    return 15 - g_sa_state.node_num;
}

uint16_t get_attach_sid(uint16_t sid)
{
    sid_node_t *node;

    if (is_direct_child(sid)) {
        return g_sa_state.sid_prefix;
    }

    slist_for_each_entry(&g_sa_state.node_list, node, sid_node_t, next) {
        if (node->sid == sid) {
            return node->attach_sid;
        }
    }

    return INVALID_SID;
}

slist_t *get_ssid_nodes_list(void)
{
    return &g_sa_state.node_list;
}

bool is_partial_function_sid(uint16_t sid)
{
    if (sid != INVALID_SID &&
        ((sid >> PF_SID_PREFIX_OFFSET) & PF_SID_PREFIX_MASK) >=
          MOBILE_PREFIX) {
        return true;
    }
    return false;
}
