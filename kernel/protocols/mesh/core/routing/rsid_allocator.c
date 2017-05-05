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
#include "memory.h"

enum {
    RSID_NUM = 2048,
};

typedef struct rsid_allocator_state_s {
    slist_t  node_list;
    uint16_t node_num;
    uint32_t free_bits[(RSID_NUM + 31) / 32];
} rsid_allocator_state_t;

static rsid_allocator_state_t g_ra_state;

void rsid_allocator_init(void)
{
    slist_init(&g_ra_state.node_list);
    g_ra_state.node_num = 0;
    memset(g_ra_state.free_bits, 0xff, sizeof(g_ra_state.free_bits));
    g_ra_state.free_bits[0] -= 1;
}

void rsid_allocator_deinit(void)
{
    sid_node_t *node;

    while (!slist_empty(&g_ra_state.node_list)) {
        node = slist_first_entry(&g_ra_state.node_list, sid_node_t, next);
        slist_del(&node->next, &g_ra_state.node_list);
        ur_mem_free(node, sizeof(sid_node_t));
    }
    g_ra_state.node_num = 0;
}

ur_error_t rsid_allocate_sid(uint8_t *ueid, uint8_t type, sid_t *sid)
{
    ur_error_t error = UR_ERROR_FAIL;
    sid_node_t *node;
    sid_node_t *new_node = NULL;
    uint16_t len;
    int32_t new_sid;

    slist_for_each_entry(&g_ra_state.node_list, node, sid_node_t, next) {
        if (memcmp(node->ueid, ueid, sizeof(node->ueid)) == 0) {
            new_node = node;
            break;
        }
    }
    if (new_node == NULL) {
        new_node = (sid_node_t *)ur_mem_alloc(sizeof(sid_node_t));
        if (!new_node) {
            return UR_ERROR_MEM;
        }
        g_ra_state.node_num++;
        new_node->sid = INVALID_SID;
        slist_add(&new_node->next, &g_ra_state.node_list);
    }
    if (type == SHORT_RANDOM_SID) {
        len = (1 << 8) -1;
    } else {
        len = (1 << 16) -1;
    }
    if (len > RSID_NUM) {
        len = RSID_NUM;
    }
    if (sid->sid != INVALID_SID) {
        if (sid->sid == new_node->sid) {
            error = UR_ERROR_NONE;
        } else {
            error = grab_free_bit(g_ra_state.free_bits, len, sid->sid);
        }
    }
    if (error != UR_ERROR_NONE) {
        new_sid = find_first_free_bit(g_ra_state.free_bits, len);
        if (new_sid >= 0) {
            sid->sid = new_sid;
            sid->type = ROUTER_NODE;
            error = UR_ERROR_NONE;
        }
    }
    if (error == UR_ERROR_NONE) {
        new_node->sid = sid->sid;
        new_node->attach_sid = INVALID_SID;
        memcpy(new_node->ueid, ueid, sizeof(new_node->ueid));
    } else {
        ur_mem_free(new_node, sizeof(sid_node_t));
        g_ra_state.node_num--;
    }
    return error;
}

ur_error_t rsid_free_sid(uint8_t *ueid, uint8_t type, uint16_t sid)
{
    sid_node_t *node = NULL;
    uint16_t len;

    slist_for_each_entry(&g_ra_state.node_list, node, sid_node_t, next) {
        if (memcmp(node->ueid, ueid, sizeof(node->ueid)) == 0) {
            break;
        }
    }
    if (node == NULL) {
        return UR_ERROR_NONE;
    }
    if (type == SHORT_RANDOM_SID) {
        len = (1 << 8) -1;
    } else {
        len = (1 << 16) -1;
    }
    if (len > RSID_NUM) {
        len = RSID_NUM;
    }
    release_bit(g_ra_state.free_bits, len, sid);
    slist_del(&node->next, &g_ra_state.node_list);
    ur_mem_free(node, sizeof(sid_node_t));
    g_ra_state.node_num--;
    return UR_ERROR_NONE;
}

uint16_t rsid_get_allocated_number(void)
{
    return g_ra_state.node_num;
}
