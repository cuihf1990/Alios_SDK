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

#include "core/sid_allocator.h"
#include "utilities/memory.h"

void rsid_allocator_init(network_context_t *network)
{
    rsid_allocator_t *allocator = NULL;

    rsid_allocator_deinit(network);
    allocator = (rsid_allocator_t *)ur_mem_alloc(sizeof(rsid_allocator_t));
    if (allocator == NULL) {
        return;
    }
    network->sid_base = (sid_base_t *)allocator;

    slist_init(&allocator->base.node_list);
    allocator->base.node_num = 0;
    memset(allocator->free_bits, 0xff, sizeof(allocator->free_bits));
    allocator->free_bits[0] -= 1;
}

void rsid_allocator_deinit(network_context_t *network)
{
    rsid_allocator_t *allocator;
    sid_node_t *node;

    allocator = (rsid_allocator_t *)network->sid_base;
    if (allocator == NULL) {
        return;
    }

    while (!slist_empty(&allocator->base.node_list)) {
        node = slist_first_entry(&allocator->base.node_list, sid_node_t, next);
        slist_del(&node->next, &allocator->base.node_list);
        ur_mem_free(node, sizeof(sid_node_t));
    }
    allocator->base.node_num = 0;
    ur_mem_free(allocator, sizeof(rsid_allocator_t));
    network->sid_base = NULL;
}

ur_error_t rsid_allocate_sid(network_context_t *network, ur_node_id_t *node_id)
{
    ur_error_t error = UR_ERROR_FAIL;
    sid_node_t *node;
    sid_node_t *new_node = NULL;
    uint16_t len;
    int32_t new_sid;
    rsid_allocator_t *allocator;

    allocator = (rsid_allocator_t *)network->sid_base;
    slist_for_each_entry(&allocator->base.node_list, node, sid_node_t, next) {
        if (memcmp(node->node_id.ueid, node_id->ueid,
                   sizeof(node->node_id.ueid)) == 0) {
            new_node = node;
            break;
        }
    }
    if (new_node == NULL) {
        new_node = (sid_node_t *)ur_mem_alloc(sizeof(sid_node_t));
        if (!new_node) {
            return UR_ERROR_MEM;
        }
        allocator->base.node_num++;
        new_node->node_id.sid = INVALID_SID;
        slist_add(&new_node->next, &allocator->base.node_list);
    }
    if (network->router->sid_type == SHORT_RANDOM_SID) {
        len = (1 << 8) - 1;
    } else {
        len = (1 << 16) - 1;
    }
    if (len > RSID_NUM) {
        len = RSID_NUM;
    }
    if (node_id->sid != INVALID_SID) {
        if (node_id->sid == new_node->node_id.sid) {
            error = UR_ERROR_NONE;
        } else {
            error = grab_free_bit(allocator->free_bits, len, node_id->sid);
        }
    }
    if (error != UR_ERROR_NONE) {
        new_sid = find_first_free_bit(allocator->free_bits, len);
        if (new_sid >= 0) {
            node_id->sid = new_sid;
            node_id->type = ROUTER_NODE;
            error = UR_ERROR_NONE;
        }
    }
    if (error == UR_ERROR_NONE) {
        new_node->node_id.sid = node_id->sid;
        node_id->type = ROUTER_NODE;
        new_node->node_id.type = node_id->type;
        new_node->node_id.attach_sid = INVALID_SID;
        memcpy(new_node->node_id.ueid, node_id->ueid, sizeof(new_node->node_id.ueid));
    } else {
        ur_mem_free(new_node, sizeof(sid_node_t));
        allocator->base.node_num--;
    }
    return error;
}

ur_error_t rsid_free_sid(network_context_t *network, ur_node_id_t *node_id)
{
    sid_node_t *node = NULL;
    uint16_t len;
    rsid_allocator_t *allocator;

    allocator = (rsid_allocator_t *)network->sid_base;
    slist_for_each_entry(&allocator->base.node_list, node, sid_node_t, next) {
        if (memcmp(node->node_id.ueid, node_id->ueid,
                   sizeof(node->node_id.ueid)) == 0) {
            break;
        }
    }
    if (node == NULL) {
        return UR_ERROR_NONE;
    }
    if (network->router->sid_type == SHORT_RANDOM_SID) {
        len = (1 << 8) - 1;
    } else {
        len = (1 << 16) - 1;
    }
    if (len > RSID_NUM) {
        len = RSID_NUM;
    }
    release_bit(allocator->free_bits, len, node_id->sid);
    slist_del(&node->next, &allocator->base.node_list);
    ur_mem_free(node, sizeof(sid_node_t));
    allocator->base.node_num--;
    return UR_ERROR_NONE;
}

uint16_t rsid_get_allocated_number(network_context_t *network)
{
    rsid_allocator_t *allocator;

    allocator = (rsid_allocator_t *)network->sid_base;
    return allocator->base.node_num;
}
