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

#include "core/mesh_mgmt.h"
#include "core/address_mgmt.h"
#include "core/router_mgr.h"
#include "hal/interfaces.h"
#include "umesh_utils.h"

#ifndef DEFAULT_ROUTER
#define DEFAULT_ROUTER SID_ROUTER
#endif

typedef struct router_mgmr_state_s {
    slist_t  router_list;
    router_t *default_router;
} router_mgmr_state_t;

static router_mgmr_state_t g_rm_state;

extern neighbor_t *get_neighbor_by_sid(hal_context_t *hal, uint16_t sid,
                                       uint16_t meshnetid);
extern void sid_router_register(void);
#ifdef CONFIG_YOS_MESH_SUPER
extern void vector_router_register(void);
#endif

void ur_router_register_module(void)
{
    router_t *router;

    slist_init(&g_rm_state.router_list);
    sid_router_register();
#ifdef CONFIG_YOS_MESH_SUPER
    vector_router_register();
#endif

    /* enable the default router */
    slist_for_each_entry(&g_rm_state.router_list, router, router_t, next) {
        if (router->id == DEFAULT_ROUTER) {
            g_rm_state.default_router = router;
            g_rm_state.default_router->cb.start();
            break;
        }
    }
}

ur_error_t ur_router_start(network_context_t *network)
{
    if (network && network->router) {
        network->router->cb.start();
    } else if (g_rm_state.default_router != NULL) {
        g_rm_state.default_router->cb.start();
    }
    return UR_ERROR_NONE;
}

void ur_router_stop(void)
{
    router_t *router;

    slist_for_each_entry(&g_rm_state.router_list, router, router_t, next) {
        if (router->cb.stop) {
            router->cb.stop();
        }
    }
}

uint16_t ur_router_get_next_hop(network_context_t *network, uint16_t dest_sid)
{
    uint16_t  next_hop = INVALID_SID;

    if (network == NULL) {
        network = g_rm_state.default_router->network;
    }
    if (network == NULL) {
        return INVALID_SID;
    }

    if (get_neighbor_by_sid(network->hal, dest_sid,
                            umesh_mm_get_meshnetid(network)) != NULL) {
        return dest_sid;
    }

    if (network->router != NULL) {
        next_hop = network->router->cb.get_next_hop_sid(dest_sid);
    } else if (g_rm_state.default_router != NULL) {
        next_hop = g_rm_state.default_router->cb.get_next_hop_sid(dest_sid);
    }

    return next_hop;
}

void ur_router_sid_updated(network_context_t *network, uint16_t sid)
{
    router_t *router;
    netids_t netids;

    router = network->router;
    if (router != NULL) {
        uint8_t i = 0, call = 0;
        for (i = 0; i < router->events.num; i++) {
            if (router->events.events[i] == EVENT_SID_UPDATED) {
                call = 1;
                break;
            }
        }

        if (call > 0 && router->cb.handle_subscribe_event != NULL) {
            netids.meshnetid = network->meshnetid;
            netids.sid = sid;
            router->cb.handle_subscribe_event(EVENT_SID_UPDATED, (uint8_t *)&netids,
                                              sizeof(netids_t));
        }
    }
}

void ur_router_neighbor_updated(neighbor_t *neighbor)
{
    if (g_rm_state.default_router != NULL &&
        g_rm_state.default_router->cb.handle_neighbor_updated != NULL) {
        g_rm_state.default_router->cb.handle_neighbor_updated(neighbor);
    }
}

router_t *ur_get_router_by_id(uint8_t id)
{
    router_t *router;
    slist_for_each_entry(&g_rm_state.router_list, router, router_t, next) {
        if (router->id == id) {
            return router;
        }
    }
    return NULL;
}

uint8_t ur_router_get_default_router(void)
{
    if (g_rm_state.default_router != NULL) {
        return g_rm_state.default_router->id;
    }
    return 0;
}

ur_error_t ur_router_set_default_router(uint8_t id)
{
    router_t *router;

    router = ur_get_router_by_id(id);
    if (router == NULL) {
        return UR_ERROR_FAIL;
    }

    if (router == g_rm_state.default_router) {
        return UR_ERROR_NONE;
    }

    if (g_rm_state.default_router->cb.stop != NULL) {
        g_rm_state.default_router->cb.stop();
    }

    g_rm_state.default_router = router;
    if (g_rm_state.default_router->cb.start != NULL) {
        g_rm_state.default_router->cb.start();
    }

    return UR_ERROR_NONE;
}

ur_error_t ur_router_send_message(router_t *router, uint16_t dst,
                                  uint8_t *payload, uint16_t length)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    uint16_t    msg_length;
    message_t   *message;
    uint8_t     *data;
    message_info_t *info;

    if (router == NULL || payload == NULL) {
        return UR_ERROR_FAIL;
    }

    if (ur_get_router_by_id(router->id) == NULL) {
        return UR_ERROR_FAIL;
    }

    /* not in use, ignore */
    if (!router->network) {
        return UR_ERROR_NONE;
    }

    msg_length = sizeof(mm_header_t) + sizeof(router->id) + length;
    data = (uint8_t *)ur_mem_alloc(msg_length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }

    msg_length = 0;
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ROUTING_INFO_UPDATE;
    msg_length += sizeof(mm_header_t);

    *(data + msg_length) = router->id;
    msg_length += sizeof(router->id);
    memcpy((data + msg_length), payload, length);
    msg_length += length;

    message = message_alloc(msg_length, ROUTER_MGR_1);
    if (message == NULL) {
        ur_mem_free(data, msg_length);
        return UR_ERROR_MEM;
    }

    message_copy_from(message, data, msg_length);

    info = message->info;
    info->network = router->network;
    // dest
    info->dest.addr.len = SHORT_ADDR_SIZE;
    info->dest.addr.short_addr = dst;
    info->dest.netid = umesh_mm_get_meshnetid(NULL);

    set_command_type(info, mm_header->command);
    ur_mem_free(data, msg_length);
    error = address_resolve(message);
    if (error == UR_ERROR_NONE) {
        error = mf_send_message(message);
    } else if(error == UR_ERROR_DROP) {
        message_free(message);
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_ROUTE,
           "router %d send routing info to %04x, len %d\r\n", router->id, dst, msg_length);
    return error;
}

ur_error_t handle_router_message_received(message_t *message)
{
    router_t *router;
    uint8_t *payload;
    uint16_t length;

    payload = message_get_payload(message);
    length = message_get_msglen(message);
    router = ur_get_router_by_id(payload[sizeof(mm_header_t)]);
    if (router == NULL) {
        return UR_ERROR_FAIL;
    }

    if (router->cb.handle_message_received == NULL) {
        return UR_ERROR_FAIL;
    }
    payload += sizeof(mm_header_t) + sizeof(router->id);
    length  -= sizeof(mm_header_t) + sizeof(router->id);
    router->cb.handle_message_received(payload, length);

    return UR_ERROR_NONE;
}

ur_error_t register_router(router_t *router)
{
    if (router == NULL || ur_get_router_by_id(router->id) != NULL) {
        return UR_ERROR_FAIL;
    }

    slist_add(&router->next, &g_rm_state.router_list);
    return UR_ERROR_NONE;
}

void sid_allocator_init(network_context_t *network)
{
    int sid_type = network->router->sid_type;
    if (sid_type == STRUCTURED_SID) {
        uint16_t sid = umesh_mm_get_local_sid();
        if (umesh_mm_get_device_state() == DEVICE_STATE_SUPER_ROUTER) {
            sid = SUPER_ROUTER_SID;
        }
        network->sid_base = allocator_init(sid, sid_type);
#ifdef CONFIG_YOS_MESH_SUPER
    } else {
        network->sid_base = rsid_allocator_init(sid_type);
#endif
    }
}

void sid_allocator_deinit(network_context_t *network)
{
    if (network->router->sid_type == STRUCTURED_SID) {
        allocator_deinit(network->sid_base);
#ifdef CONFIG_YOS_MESH_SUPER
    } else {
        rsid_allocator_deinit(network->sid_base);
#endif
    }
    network->sid_base = 0;
}

ur_error_t sid_allocator_alloc(network_context_t *network, ur_node_id_t *node)
{
    ur_error_t error;

    switch (network->router->sid_type) {
        case STRUCTURED_SID:
            error = allocate_sid(network->sid_base, node);
            break;
#ifdef CONFIG_YOS_MESH_SUPER
        case SHORT_RANDOM_SID:
        case RANDOM_SID:
            error = rsid_allocate_sid(network->sid_base, node);
            break;
#endif
        default:
            error = UR_ERROR_PARSE;
    }

    return error;
}

ur_error_t sid_allocator_free(network_context_t *network, ur_node_id_t *node)
{
    if (is_partial_function_sid(node->sid)) {
        network = get_default_network_context();
        update_sid_mapping(network->sid_base, node, false);
#ifdef CONFIG_YOS_MESH_SUPER
    } else {
        network = get_network_context_by_meshnetid(node->meshnetid);
        if (network == NULL) {
            return UR_ERROR_NONE;
        }

        if (network->router->sid_type == SHORT_RANDOM_SID ||
            network->router->sid_type == RANDOM_SID) {
            rsid_free_sid(network->sid_base, node);
        }
#endif
    }

    return UR_ERROR_NONE;
}

static uint16_t calc_ssid_child_num(network_context_t *network)
{
    uint16_t num = 1;
    neighbor_t *nbr;
    neighbor_t *next_nbr;
    bool dup = false;
    slist_t *nbrs;

    nbrs = &network->hal->neighbors_list;
    slist_for_each_entry(nbrs, nbr, neighbor_t, next) {
        if (nbr->state != STATE_CHILD || network->meshnetid != nbr->addr.netid) {
            continue;
        }
        dup = false;
        slist_for_each_entry(nbrs, next_nbr, neighbor_t, next) {
            if (nbr == next_nbr) {
                continue;
            }
            if (next_nbr->addr.netid == umesh_mm_get_meshnetid(network) &&
                nbr->addr.addr.short_addr != INVALID_SID &&
                nbr->addr.addr.short_addr == next_nbr->addr.addr.short_addr) {
                dup = true;
            }
        }
        if (dup == false) {
            num += nbr->ssid_info.child_num;
        }
    }

    if (umesh_mm_get_device_state() == DEVICE_STATE_LEADER) {
        num += get_allocated_pf_number(network->sid_base);
    }
    return num;
}

uint16_t sid_allocator_get_num(network_context_t *network)
{
    uint16_t num = 0;

#ifdef CONFIG_YOS_MESH_SUPER
    if (network->router->sid_type == SHORT_RANDOM_SID ||
        network->router->sid_type == RANDOM_SID) {
        num = rsid_get_allocated_number(network->sid_base) + 1;
    }
#endif
    if (network->router->sid_type == STRUCTURED_SID) {
        num = calc_ssid_child_num(network);
    }

    return num;
}
