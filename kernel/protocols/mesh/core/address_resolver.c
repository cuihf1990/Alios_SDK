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
#include <string.h>

#include "mesh_types.h"
#include "address_resolver.h"
#include "mesh_mgmt.h"
#include "mesh_forwarder.h"
#include "sid_allocator.h"
#include "timer.h"
#include "logging.h"
#include "interfaces.h"
#include "address_cache.h"
#include "link_mgmt.h"

typedef struct address_resolver_state_s {
    address_cache_t            cache[UR_MESH_ADDRESS_CACHE_SIZE];
    ur_timer_t                 timer;
    address_resolved_handler_t handler;
} address_resolver_state_t;

static address_resolver_state_t g_ar_state;

static ur_error_t send_address_query(network_context_t *network, uint8_t query_type,
                                     ur_node_id_t *target);
static ur_error_t send_address_query_response(network_context_t *network,
                                              ur_node_id_t *dest,
                                              ur_node_id_t *attach_node,
                                              ur_node_id_t *target_node);

static void timer_handler(void *args)
{
    uint8_t index;
    bool    continue_timer = false;
    network_context_t *network;

    g_ar_state.timer = NULL;
    for (index = 0; index < UR_MESH_ADDRESS_CACHE_SIZE; index++) {
        if (g_ar_state.cache[index].state != AQ_STATE_QUERY) {
            continue;
        }
        continue_timer = true;
        if (g_ar_state.cache[index].timeout > 0) {
            g_ar_state.cache[index].timeout--;
            if (g_ar_state.cache[index].timeout == 0) {
                g_ar_state.cache[index].retry_timeout = ADDRESS_QUERY_RETRY_TIMEOUT;
                network = get_network_context_by_meshnetid(g_ar_state.cache[index].meshnetid);
                if (network == NULL) {
                    network = get_default_network_context();
                }
                g_ar_state.handler(network, &g_ar_state.cache[index], UR_ERROR_DROP);
            }
        } else if (g_ar_state.cache[index].retry_timeout > 0) {
                g_ar_state.cache[index].retry_timeout--;
        }
    }

    if (continue_timer) {
        g_ar_state.timer = ur_start_timer(ADDRESS_QUERY_STATE_UPDATE_PERIOD, timer_handler, NULL);
    }
}

ur_error_t address_resolve(network_context_t *network, uint8_t query_type,
                           ur_node_id_t *target, ur_node_id_t *attach)
{
    ur_error_t      error = UR_ERROR_NONE;
    uint8_t         index = 0;
    address_cache_t *cache = NULL;

    for (index = 0; index < UR_MESH_ADDRESS_CACHE_SIZE; index++) {
        if (g_ar_state.cache[index].state != AQ_STATE_INVALID) {
            if (query_type == PF_ATTACH_QUERY &&
                g_ar_state.cache[index].meshnetid == target->meshnetid &&
                g_ar_state.cache[index].sid == target->sid) {
                cache = &g_ar_state.cache[index];
                break;
            } else if (query_type == TARGET_QUERY &&
                       memcmp(target->ueid, g_ar_state.cache[index].ueid,
                       sizeof(g_ar_state.cache[index].ueid)) == 0) {
                cache = &g_ar_state.cache[index];
                break;
            }
        } else if (cache == NULL) {
            cache = &g_ar_state.cache[index];
        }
    }

    if (cache == NULL) {
        return UR_ERROR_MEM;
    }

    switch (cache->state) {
        case AQ_STATE_INVALID:
            memcpy(cache->ueid, target->ueid, sizeof(cache->ueid));
            cache->sid = target->sid;
            cache->meshnetid = target->meshnetid;
            cache->attach_sid = attach->sid;
            cache->attach_netid = attach->meshnetid;
            cache->timeout = ADDRESS_QUERY_TIMEOUT;
            cache->retry_timeout = ADDRESS_QUERY_RETRY_TIMEOUT;
            cache->state = AQ_STATE_QUERY;
            send_address_query(network, query_type, target);
            error = UR_ERROR_ADDRESS_QUERY;
            break;
        case AQ_STATE_QUERY:
            if (cache->timeout > 0) {
                error = UR_ERROR_ADDRESS_QUERY;
            } else if (cache->timeout == 0 && cache->retry_timeout == 0) {
                cache->timeout = ADDRESS_QUERY_TIMEOUT;
                send_address_query(network, query_type, target);
                error = UR_ERROR_ADDRESS_QUERY;
            } else {
                error = UR_ERROR_DROP;
            }
            break;
        case AQ_STATE_CACHED:
            attach->sid = cache->attach_sid;
            attach->meshnetid = cache->attach_netid;
            target->sid = cache->sid;
            target->meshnetid = cache->meshnetid;
            break;
        default:
            assert(0);
            break;
    }
    return error;
}

static ur_error_t send_address_query(network_context_t *network, uint8_t query_type,
                                     ur_node_id_t *target)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    mm_addr_query_tv_t *addr_query;
    mm_node_id_tv_t *target_id;
    mm_ueid_tv_t *target_ueid;
    message_t   *message;
    uint8_t     *data;
    uint16_t    length;
    mesh_dest_t *dest;

    length = sizeof(mm_header_t) + sizeof(mm_addr_query_tv_t);
    if (query_type == PF_ATTACH_QUERY) {
        length += sizeof(mm_node_id_tv_t);
    } else if (query_type == TARGET_QUERY) {
        length += sizeof(mm_ueid_tv_t);
    } else {
        return UR_ERROR_FAIL;
    }
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ADDRESS_QUERY;
    data += sizeof(mm_header_t);

    addr_query = (mm_addr_query_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)addr_query, TYPE_ADDR_QUERY);
    addr_query->query_type = query_type;
    data += sizeof(mm_addr_query_tv_t);

    switch (query_type) {
        case PF_ATTACH_QUERY:
            target_id = (mm_node_id_tv_t *)data;
            mm_init_tv_base((mm_tv_t *)target_id, TYPE_NODE_ID);
            target_id->sid = target->sid;
            target_id->meshnetid = target->meshnetid;
            data += sizeof(mm_node_id_tv_t);
            break;
        case TARGET_QUERY:
            target_ueid = (mm_ueid_tv_t *)data;
            mm_init_tv_base((mm_tv_t *)target_ueid, TYPE_TARGET_UEID);
            memcpy(target_ueid->ueid, target->ueid, sizeof(target_ueid->ueid));
            data += sizeof(mm_ueid_tv_t);
            break;
        default:
            assert(0);
            break;
    }

    if (g_ar_state.timer == NULL) {
        g_ar_state.timer = ur_start_timer(ADDRESS_QUERY_STATE_UPDATE_PERIOD, timer_handler, NULL);
    }

    dest = message->dest;
    dest->dest.len = 2;
    dest->flags = INSERT_DESTNETID_FLAG;
    dest->meshnetid = get_main_netid(network->meshnetid);
    dest->dest.short_addr = LEADER_SID;
    message->network = network;
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send address query, len %d\r\n", length);
    return error;
}

ur_error_t handle_address_query(uint8_t *payload, uint16_t length,
                                const mesh_src_t *src,
                                const mac_address_t *dest)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_addr_query_tv_t *addr_query;
    mm_node_id_tv_t *target_id;
    mm_ueid_tv_t *ueid;
    uint8_t     *tlvs;
    ur_node_id_t dest_node;
    ur_node_id_t target_node;
    ur_node_id_t attach_node;
    uint16_t    tlvs_length;
    network_context_t *network;

    if (mm_get_device_state() < DEVICE_STATE_LEADER) {
        return UR_ERROR_NONE;
    }

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    addr_query = (mm_addr_query_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_ADDR_QUERY);
    target_id = (mm_node_id_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_NODE_ID);
    ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);

    attach_node.sid = INVALID_SID;
    attach_node.meshnetid = INVALID_NETID;

    if (addr_query == NULL) {
        return UR_ERROR_FAIL;
    }
    switch (addr_query->query_type) {
        case PF_ATTACH_QUERY:
            if (target_id == NULL) {
                return UR_ERROR_FAIL;
            }
            memset(&target_node, 0xff, sizeof(target_node));
            target_node.sid = target_id->sid;
            target_node.meshnetid = target_id->meshnetid;
            get_attach_by_nodeid(&attach_node, &target_node);
            if (attach_node.sid == INVALID_SID || attach_node.meshnetid == INVALID_NETID) {
                error = UR_ERROR_FAIL;
            }
            break;
        case TARGET_QUERY:
            if (ueid == NULL) {
                return UR_ERROR_FAIL;
            }
            get_target_by_ueid(&target_node, ueid->ueid);
            if (is_partial_function_sid(target_node.sid) == true) {
                get_attach_by_nodeid(&attach_node, &target_node);
            }
            if (target_node.sid == INVALID_SID ||
                target_node.meshnetid == INVALID_NETID) {
                error = UR_ERROR_FAIL;
            }
            break;
        default:
            break;
    }

    if (error == UR_ERROR_NONE) {
        dest_node.sid = src->sid;
        dest_node.meshnetid = src->meshnetid;
        network = get_network_context_by_meshnetid(dest_node.meshnetid);
        if (network == NULL) {
            network = get_default_network_context();
        }
        send_address_query_response(network, &dest_node, &attach_node,
                                    &target_node);
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle address query\r\n");
    return error;
}

static ur_error_t send_address_query_response(network_context_t *network,
                                              ur_node_id_t *dest_node,
                                              ur_node_id_t *attach_node,
                                              ur_node_id_t *target_node)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    mm_node_id_tv_t *attach_id;
    mm_node_id_tv_t *target_id;
    mm_ueid_tv_t *target_ueid;
    message_t   *message;
    uint8_t     *data;
    uint16_t    length;
    mesh_dest_t *mesh_dest;
    ur_node_id_t attach;

    length = sizeof(mm_header_t) + sizeof(mm_node_id_tv_t) +
             sizeof(mm_ueid_tv_t);
    if (attach_node->sid != INVALID_SID &&
        attach_node->meshnetid != INVALID_NETID) {
        length += sizeof(mm_node_id_tv_t);
    }
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ADDRESS_QUERY_RESPONSE;
    data += sizeof(mm_header_t);

    target_id = (mm_node_id_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)target_id, TYPE_NODE_ID);
    target_id->sid = target_node->sid;
    target_id->meshnetid = target_node->meshnetid;
    data += sizeof(mm_node_id_tv_t);

    target_ueid = (mm_ueid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)target_ueid, TYPE_TARGET_UEID);
    memcpy(target_ueid->ueid, target_node->ueid, sizeof(target_ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    if (attach_node->sid != INVALID_SID &&
        attach_node->meshnetid != INVALID_NETID) {
        attach_id = (mm_node_id_tv_t *)data;
        mm_init_tv_base((mm_tv_t *)attach_id, TYPE_ATTACH_NODE_ID);
        attach_id->sid = attach_node->sid;
        attach_id->meshnetid = attach_node->meshnetid;
        data += sizeof(mm_node_id_tv_t);
    }

    mesh_dest = message->dest;
    mesh_dest->dest.len = 2;
    mesh_dest->dest.short_addr = dest_node->sid;
    mesh_dest->flags = INSERT_DESTNETID_FLAG;
    mesh_dest->meshnetid = dest_node->meshnetid;
    if (is_partial_function_sid(dest_node->sid)) {
        mesh_dest->flags |= RELAYERSID_FLAG;
        get_attach_by_nodeid(&attach, dest_node);
        mesh_dest->relayersid = attach.sid;
        mesh_dest->meshnetid = attach.meshnetid;
    }
    if (mesh_dest->meshnetid != mm_get_meshnetid(network)) {
        mesh_dest->flags |= INSERT_DESTNETID_FLAG;
    }

    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send address query response, len %d\r\n", length);

    return error;
}

ur_error_t handle_address_query_response(uint8_t *payload, uint16_t length,
                                         const mesh_src_t *src,
                                         const mac_address_t *dest)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_node_id_tv_t *target_id;
    mm_node_id_tv_t *attach_id;
    mm_ueid_tv_t *target_ueid;
    uint8_t     *tlvs;
    uint16_t    tlvs_length;
    uint8_t     index;

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    attach_id = (mm_node_id_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_ATTACH_NODE_ID);
    target_id = (mm_node_id_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_NODE_ID);
    target_ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);

    if (target_id == NULL || target_ueid == NULL) {
        return UR_ERROR_FAIL;
    }

    if (attach_id && (attach_id->sid == INVALID_SID || attach_id->meshnetid == INVALID_NETID)) {
        return UR_ERROR_DROP;
    }

    for (index = 0; index < UR_MESH_ADDRESS_CACHE_SIZE; index++) {
        if (attach_id && g_ar_state.cache[index].sid == target_id->sid &&
            g_ar_state.cache[index].meshnetid == target_id->meshnetid) {
            if (g_ar_state.cache[index].state != AQ_STATE_CACHED) {
                g_ar_state.cache[index].state = AQ_STATE_CACHED;
                g_ar_state.cache[index].attach_sid = attach_id->sid;
                g_ar_state.cache[index].attach_netid = attach_id->meshnetid;
                g_ar_state.cache[index].timeout = 0;
                memcpy(g_ar_state.cache[index].ueid, target_ueid->ueid, sizeof(g_ar_state.cache[index].ueid));
                g_ar_state.handler(src->dest_network, &g_ar_state.cache[index], error);
            }
            break;
        } else if (target_ueid && memcmp(target_ueid->ueid, g_ar_state.cache[index].ueid,
                                         sizeof(target_ueid->ueid)) == 0) {
            if (g_ar_state.cache[index].state != AQ_STATE_CACHED) {
                g_ar_state.cache[index].state = AQ_STATE_CACHED;
                g_ar_state.cache[index].sid = target_id->sid;
                g_ar_state.cache[index].meshnetid = target_id->meshnetid;
                if (attach_id) {
                    g_ar_state.cache[index].attach_sid = attach_id->sid;
                    g_ar_state.cache[index].attach_netid = attach_id->meshnetid;
                } else {
                    g_ar_state.cache[index].attach_sid = INVALID_SID;
                    g_ar_state.cache[index].attach_netid = INVALID_NETID;
                }
                g_ar_state.cache[index].timeout = 0;
                g_ar_state.handler(src->dest_network, &g_ar_state.cache[index], error);
            }
            break;
        }
    }
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle address query response\r\n");
    return UR_ERROR_NONE;
}

ur_error_t send_address_notification(network_context_t *network, uint16_t dest)
{
    ur_error_t      error = UR_ERROR_NONE;
    mm_header_t     *mm_header;
    mm_ueid_tv_t    *target_ueid;
    mm_node_id_tv_t *target_node;
    mm_node_id_tv_t *attach_node;
    mm_hal_type_tv_t *hal_type;
    message_t       *message;
    uint8_t         *data;
    uint16_t        length;
    mesh_dest_t     *mesh_dest;
    hal_context_t   *hal;

    length = sizeof(mm_header_t) + sizeof(mm_ueid_tv_t) +
             sizeof(mm_node_id_tv_t) + sizeof(mm_hal_type_tv_t);
    if (network->attach_node) {
        length += sizeof(mm_node_id_tv_t);
    }
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ADDRESS_NOTIFICATION;
    data += sizeof(mm_header_t);

    target_ueid = (mm_ueid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)target_ueid, TYPE_TARGET_UEID);
    memcpy(target_ueid->ueid, mm_get_local_ueid(), sizeof(target_ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    target_node = (mm_node_id_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)target_node, TYPE_NODE_ID);
    target_node->sid = mm_get_local_sid();
    target_node->meshnetid = mm_get_meshnetid(network);
    data += sizeof(mm_node_id_tv_t);

    hal_type = (mm_hal_type_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)hal_type, TYPE_DEF_HAL_TYPE);
    hal = get_default_hal_context();
    hal_type->type = hal->module->type;
    data += sizeof(mm_hal_type_tv_t);

    if (network->attach_node) {
        attach_node = (mm_node_id_tv_t *)data;
        mm_init_tv_base((mm_tv_t *)attach_node, TYPE_ATTACH_NODE_ID);
        attach_node->sid = network->attach_node->sid;
        attach_node->meshnetid = network->attach_node->sub_netid;
        data += sizeof(mm_node_id_tv_t);
    }

    mesh_dest = message->dest;
    mesh_dest->dest.len = 2;
    mesh_dest->dest.short_addr = dest;
    mesh_dest->flags = INSERT_DESTNETID_FLAG;
    if (dest == LEADER_SID) {
        mesh_dest->meshnetid = mm_get_main_netid(network);
    } else {
        mesh_dest->meshnetid = mm_get_meshnetid(network);
    }
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send address notification, len %d\r\n", length);
    return error;
}

ur_error_t handle_address_notification(uint8_t *payload, uint16_t length,
                                       const mesh_src_t *src,
                                       const mac_address_t *dest)
{
    ur_error_t   error = UR_ERROR_NONE;
    mm_ueid_tv_t *target_ueid;
    mm_node_id_tv_t *target_node;
    mm_node_id_tv_t *attach_node;
    mm_hal_type_tv_t *hal_type;
    uint8_t      *tlvs;
    uint16_t     tlvs_length;
    ur_node_id_t target;
    ur_node_id_t attach;
    neighbor_t   *nbr = NULL;
    hal_context_t *hal;

    if (mm_get_device_state() != DEVICE_STATE_LEADER &&
        mm_get_device_state() != DEVICE_STATE_SUPER_ROUTER) {
        return UR_ERROR_NONE;
    }

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    attach_node = (mm_node_id_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_ATTACH_NODE_ID);
    target_node = (mm_node_id_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_NODE_ID);
    target_ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);
    hal_type = (mm_hal_type_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_DEF_HAL_TYPE);

    if (target_node == NULL || target_ueid == NULL || hal_type == NULL) {
        return UR_ERROR_FAIL;
    }

    target.sid = target_node->sid;
    target.meshnetid = target_node->meshnetid;
    hal = src->dest_network->hal;
    nbr = get_neighbor_by_sid(hal, target.sid, target.meshnetid);
    if (nbr) {
        memcpy(nbr->ueid, target_ueid->ueid, sizeof(nbr->ueid));
    }
    if (attach_node) {
        attach.sid = attach_node->sid;
        attach.meshnetid = attach_node->meshnetid;
    } else {
        attach.sid = INVALID_SID;
        attach.meshnetid = INVALID_NETID;
    }
    memcpy(&target.ueid, target_ueid->ueid, sizeof(target.ueid));
    error = update_address_cache(hal_type->type, &target, &attach);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle address notification\r\n");

    return error;
}

ur_error_t send_dest_unreachable(network_context_t *network, sid_t *dest,
                                 sid_t *target)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    mm_node_id_tv_t *target_node_id;
    message_t   *message;
    uint8_t     *data;
    uint16_t    length;
    mesh_dest_t *mesh_dest;

    length = sizeof(mm_header_t) + sizeof(mm_node_id_tv_t);
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_DEST_UNREACHABLE;
    data += sizeof(mm_header_t);

    target_node_id = (mm_node_id_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)target_node_id, TYPE_NODE_ID);
    target_node_id->meshnetid = target->meshnetid;
    target_node_id->sid = target->sid;
    data += sizeof(mm_node_id_tv_t);

    mesh_dest = message->dest;
    mesh_dest->dest.len = 2;
    mesh_dest->dest.short_addr = dest->sid;
    mesh_dest->meshnetid = dest->meshnetid;
    mesh_dest->flags = INSERT_DESTNETID_FLAG;
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send dest unreachable, len %d\r\n", length);
    return error;
}

ur_error_t handle_dest_unreachable(uint8_t *payload, uint16_t length,
                                   const mesh_src_t *src,
                                   const mac_address_t *dest)
{
    ur_error_t      error = UR_ERROR_NONE;
    mm_node_id_tv_t *target_node_id;
    uint8_t         *tlvs;
    uint16_t        tlvs_length;
    uint8_t         index;

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    if ((target_node_id = (mm_node_id_tv_t *)mm_get_tv(tlvs, tlvs_length,
                                                       TYPE_NODE_ID)) == NULL) {
        return UR_ERROR_FAIL;
    }

    for (index = 0; index < UR_MESH_ADDRESS_CACHE_SIZE; index++) {
        if (g_ar_state.cache[index].sid == target_node_id->sid &&
            g_ar_state.cache[index].meshnetid == target_node_id->meshnetid &&
            g_ar_state.cache[index].state != AQ_STATE_INVALID) {
            g_ar_state.cache[index].state = AQ_STATE_INVALID;
        }
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle dest unreachable\r\n");
    return error;
}

ur_error_t send_address_error(network_context_t *network, sid_t *dest)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    message_t   *message;
    uint8_t     *data;
    uint16_t    length;
    mesh_dest_t *mesh_dest;

    length = sizeof(mm_header_t);
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ADDRESS_ERROR;
    data += sizeof(mm_header_t);

    mesh_dest = message->dest;
    mesh_dest->dest.len = 2;
    mesh_dest->dest.short_addr = dest->sid;
    mesh_dest->meshnetid = dest->meshnetid;
    mesh_dest->flags |= INSERT_DESTNETID_FLAG;

    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send address error, len %d\r\n", length);
    return error;
}

ur_error_t handle_address_error(uint8_t *payload, uint16_t length,
                                const mesh_src_t *src,
                                const mac_address_t *dest)
{
    ur_error_t error = UR_ERROR_NONE;

    if (mm_get_local_sid() == src->sid) {
        mm_set_local_sid(src->sid);
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle address error\r\n");
    return error;

}

void address_resolver_init(address_resolved_handler_t handler)
{
    memset(&g_ar_state.cache, 0, sizeof(g_ar_state.cache));
    g_ar_state.handler = handler;
}
