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

#include "core/address_mgmt.h"
#include "core/mesh_mgmt.h"
#include "core/mesh_forwarder.h"
#include "core/sid_allocator.h"
#include "core/network_data.h"
#include "core/link_mgmt.h"
#include "utilities/timer.h"
#include "utilities/logging.h"
#include "hal/hals.h"
#include "hal/interfaces.h"

typedef struct address_resolver_state_s {
    address_cache_t            cache[UR_MESH_ADDRESS_CACHE_SIZE];
    ur_timer_t                 timer;
    address_resolved_handler_t handler;
} address_resolver_state_t;

typedef struct address_cache_state_s {
    slist_t                    cache_list;
    uint16_t                   cache_num;
    ur_timer_t                 timer;
    address_resolved_handler_t handler;
} address_cache_state_t;

static address_resolver_state_t g_ar_state;
static address_cache_state_t g_ac_state;

static ur_error_t send_address_query(network_context_t *network,
                                     ur_addr_t *dest,
                                     uint8_t query_type, ur_node_id_t *target);
static ur_error_t send_address_query_response(network_context_t *network,
                                              ur_addr_t *dest,
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
        g_ar_state.timer = ur_start_timer(ADDRESS_QUERY_STATE_UPDATE_PERIOD,
                                          timer_handler, NULL);
    }
}

ur_error_t address_resolve(uint8_t query_type, ur_node_id_t *target,
                           ur_node_id_t *attach)
{
    ur_error_t        error = UR_ERROR_NONE;
    uint8_t           index = 0;
    address_cache_t   *cache = NULL;
    network_context_t *network;
    ur_addr_t         dest;

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

    network = get_default_network_context();
    get_leader_addr(&dest);
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
            send_address_query(network, &dest, query_type, target);
            error = UR_ERROR_ADDRESS_QUERY;
            break;
        case AQ_STATE_QUERY:
            if (cache->timeout > 0) {
                error = UR_ERROR_ADDRESS_QUERY;
            } else if (cache->timeout == 0 && cache->retry_timeout == 0) {
                cache->timeout = ADDRESS_QUERY_TIMEOUT;
                send_address_query(network, &dest, query_type, target);
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

static ur_error_t send_address_query(network_context_t *network,
                                     ur_addr_t *dest,
                                     uint8_t query_type, ur_node_id_t *target)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    mm_addr_query_tv_t *addr_query;
    mm_node_id_tv_t *target_id;
    mm_ueid_tv_t *target_ueid;
    message_t *message;
    uint8_t *data;
    uint16_t length;
    message_info_t *info;

    length = sizeof(mm_header_t) + sizeof(mm_addr_query_tv_t);
    if (query_type == PF_ATTACH_QUERY) {
        length += sizeof(mm_node_id_tv_t);
    } else if (query_type == TARGET_QUERY) {
        length += sizeof(mm_ueid_tv_t);
    } else {
        return UR_ERROR_FAIL;
    }
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ADDRESS_QUERY;
    data += sizeof(mm_header_t);

    addr_query = (mm_addr_query_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)addr_query, TYPE_ADDR_QUERY);
    addr_query->query_type = query_type;
    data += sizeof(mm_addr_query_tv_t);

    switch (query_type) {
        case PF_ATTACH_QUERY:
            target_id = (mm_node_id_tv_t *)data;
            umesh_mm_init_tv_base((mm_tv_t *)target_id, TYPE_NODE_ID);
            target_id->sid = target->sid;
            target_id->meshnetid = target->meshnetid;
            data += sizeof(mm_node_id_tv_t);
            break;
        case TARGET_QUERY:
            target_ueid = (mm_ueid_tv_t *)data;
            umesh_mm_init_tv_base((mm_tv_t *)target_ueid, TYPE_TARGET_UEID);
            memcpy(target_ueid->ueid, target->ueid, sizeof(target_ueid->ueid));
            data += sizeof(mm_ueid_tv_t);
            break;
        default:
            assert(0);
            break;
    }

    if (g_ar_state.timer == NULL) {
        g_ar_state.timer = ur_start_timer(ADDRESS_QUERY_STATE_UPDATE_PERIOD,
                                          timer_handler, NULL);
    }

    info = message->info;
    info->network = network;
    // dest
    memcpy(&info->dest, dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send address query, len %d\r\n", length);
    return error;
}

ur_error_t handle_address_query(message_t *message)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_addr_query_tv_t *addr_query;
    mm_node_id_tv_t *target_id;
    mm_ueid_tv_t *ueid;
    uint8_t     *tlvs;
    ur_node_id_t target_node;
    ur_node_id_t attach_node;
    uint16_t    tlvs_length;
    network_context_t *network;
    message_info_t *info;

    if (umesh_mm_get_device_state() < DEVICE_STATE_LEADER) {
        return UR_ERROR_NONE;
    }

    info = message->info;
    network = info->network;
    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    addr_query = (mm_addr_query_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                       TYPE_ADDR_QUERY);
    target_id = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_NODE_ID);
    ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);

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
        network = get_network_context_by_meshnetid(info->src.netid);
        if (network == NULL) {
            network = get_default_network_context();
        }
        send_address_query_response(network, &info->src, &attach_node,
                                    &target_node);
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle address query\r\n");
    return error;
}

static ur_error_t send_address_query_response(network_context_t *network,
                                              ur_addr_t *dest,
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
    message_info_t *info;

    length = sizeof(mm_header_t) + sizeof(mm_node_id_tv_t) +
             sizeof(mm_ueid_tv_t);
    if (attach_node->sid != INVALID_SID &&
        attach_node->meshnetid != INVALID_NETID) {
        length += sizeof(mm_node_id_tv_t);
    }
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ADDRESS_QUERY_RESPONSE;
    data += sizeof(mm_header_t);

    target_id = (mm_node_id_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)target_id, TYPE_NODE_ID);
    target_id->sid = target_node->sid;
    target_id->meshnetid = target_node->meshnetid;
    data += sizeof(mm_node_id_tv_t);

    target_ueid = (mm_ueid_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)target_ueid, TYPE_TARGET_UEID);
    memcpy(target_ueid->ueid, target_node->ueid, sizeof(target_ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    if (attach_node->sid != INVALID_SID &&
        attach_node->meshnetid != INVALID_NETID) {
        attach_id = (mm_node_id_tv_t *)data;
        umesh_mm_init_tv_base((mm_tv_t *)attach_id, TYPE_ATTACH_NODE_ID);
        attach_id->sid = attach_node->sid;
        attach_id->meshnetid = attach_node->meshnetid;
        data += sizeof(mm_node_id_tv_t);
    }

    info = message->info;
    info->network = network;
    // dest
    memcpy(&info->dest, dest, sizeof(info->dest));
    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send address query response, len %d\r\n", length);

    return error;
}

ur_error_t handle_address_query_response(message_t *message)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_node_id_tv_t *target_id;
    mm_node_id_tv_t *attach_id;
    mm_ueid_tv_t *target_ueid;
    uint8_t     *tlvs;
    uint16_t    tlvs_length;
    uint8_t     index;
    network_context_t *network;
    message_info_t *info;

    info = message->info;
    network = info->network;
    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    attach_id = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                   TYPE_ATTACH_NODE_ID);
    target_id = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_NODE_ID);
    target_ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                  TYPE_TARGET_UEID);

    if (target_id == NULL || target_ueid == NULL) {
        return UR_ERROR_FAIL;
    }

    if (attach_id && (attach_id->sid == INVALID_SID ||
                      attach_id->meshnetid == INVALID_NETID)) {
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
                memcpy(g_ar_state.cache[index].ueid, target_ueid->ueid,
                       sizeof(g_ar_state.cache[index].ueid));
                g_ar_state.handler(network, &g_ar_state.cache[index], error);
            }
            break;
        } else if (target_ueid &&
                   memcmp(target_ueid->ueid, g_ar_state.cache[index].ueid,
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
                g_ar_state.handler(network, &g_ar_state.cache[index], error);
            }
            break;
        }
    }
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "handle address query response\r\n");
    return UR_ERROR_NONE;
}

ur_error_t send_address_notification(network_context_t *network,
                                     ur_addr_t *dest)
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
    message_info_t *info;
    hal_context_t   *hal;

    length = sizeof(mm_header_t) + sizeof(mm_ueid_tv_t) +
             sizeof(mm_node_id_tv_t) + sizeof(mm_hal_type_tv_t);
    if (network->attach_node) {
        length += sizeof(mm_node_id_tv_t);
    }
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ADDRESS_NOTIFICATION;
    data += sizeof(mm_header_t);

    target_ueid = (mm_ueid_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)target_ueid, TYPE_TARGET_UEID);
    memcpy(target_ueid->ueid, umesh_mm_get_local_ueid(), sizeof(target_ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    target_node = (mm_node_id_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)target_node, TYPE_NODE_ID);
    target_node->sid = umesh_mm_get_local_sid();
    target_node->meshnetid = umesh_mm_get_meshnetid(network);
    data += sizeof(mm_node_id_tv_t);

    hal_type = (mm_hal_type_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)hal_type, TYPE_DEF_HAL_TYPE);
    hal = get_default_hal_context();
    hal_type->type = hal->module->type;
    data += sizeof(mm_hal_type_tv_t);

    if (network->attach_node) {
        attach_node = (mm_node_id_tv_t *)data;
        umesh_mm_init_tv_base((mm_tv_t *)attach_node, TYPE_ATTACH_NODE_ID);
        attach_node->sid = network->attach_node->addr.addr.short_addr;
        attach_node->meshnetid = network->attach_node->addr.netid;
        data += sizeof(mm_node_id_tv_t);
    }

    info = message->info;
    info->network = network;
    // dest
    if (dest == NULL) {
        get_leader_addr(&info->dest);
    } else {
        memcpy(&info->dest, dest, sizeof(info->dest));
    }

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send address notification, len %d\r\n", length);
    return error;
}

ur_error_t handle_address_notification(message_t *message)
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

    if (umesh_mm_get_device_state() != DEVICE_STATE_LEADER &&
        umesh_mm_get_device_state() != DEVICE_STATE_SUPER_ROUTER) {
        return UR_ERROR_NONE;
    }

    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    attach_node = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                     TYPE_ATTACH_NODE_ID);
    target_node = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                     TYPE_NODE_ID);
    target_ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                  TYPE_TARGET_UEID);
    hal_type = (mm_hal_type_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                   TYPE_DEF_HAL_TYPE);

    if (target_node == NULL || target_ueid == NULL || hal_type == NULL) {
        return UR_ERROR_FAIL;
    }

    target.sid = target_node->sid;
    target.meshnetid = target_node->meshnetid;
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

void address_resolver_init(address_resolved_handler_t handler)
{
    memset(&g_ar_state.cache, 0, sizeof(g_ar_state.cache));
    g_ar_state.handler = handler;
}

static void handle_addr_cache_timer(void *args)
{
    sid_node_t        *node;
    uint8_t           timeout;
    network_context_t *network;

    g_ac_state.timer = NULL;
    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        switch (node->type) {
            case MEDIA_TYPE_WIFI:
                timeout = WIFI_ADDR_CACHE_ALIVE_TIMEOUT;
                break;
            case MEDIA_TYPE_BLE:
                timeout = BLE_ADDR_CACHE_ALIVE_TIMEOUT;
                break;
            case MEDIA_TYPE_15_4:
                timeout = IEEE154_ADDR_CACHE_ALIVE_TIMEOUT;
                break;
            default:
                timeout = 0;
                break;
        }

        node->node_id.timeout++;
        if (node->node_id.timeout > timeout) {
            if (is_partial_function_sid(node->node_id.sid)) {
                network = get_default_network_context();
                update_sid_mapping(network, &node->node_id, false);
            } else if ((network = get_network_context_by_meshnetid(node->node_id.meshnetid))
                       != NULL) {
                if (network->router->sid_type == SHORT_RANDOM_SID ||
                    network->router->sid_type == RANDOM_SID) {
                    rsid_free_sid(network, &node->node_id);
                }
            }
            slist_del(&node->next, &g_ac_state.cache_list);
            ur_mem_free(node, sizeof(sid_node_t));
            g_ac_state.cache_num--;
        }
    }

    nd_set_meshnetsize(NULL, g_ac_state.cache_num + 1);
    g_ac_state.timer = ur_start_timer(ADDR_CACHE_CHECK_INTERVAL,
                                      handle_addr_cache_timer, NULL);
}

ur_error_t update_address_cache(media_type_t type, ur_node_id_t *target,
                                ur_node_id_t *attach)
{
    sid_node_t *node = NULL;

    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        if (memcmp(node->node_id.ueid, target->ueid, sizeof(node->node_id.ueid)) == 0) {
            break;
        }
    }

    if (node == NULL) {
        node = (sid_node_t *)ur_mem_alloc(sizeof(sid_node_t));
        if (node == NULL) {
            return UR_ERROR_MEM;
        }
        slist_add(&node->next, &g_ac_state.cache_list);
        g_ac_state.cache_num++;
        nd_set_meshnetsize(NULL, g_ac_state.cache_num + 1);
        memcpy(node->node_id.ueid, target->ueid, sizeof(node->node_id.ueid));
    }

    node->node_id.sid = target->sid;
    node->node_id.meshnetid = target->meshnetid;
    node->node_id.attach_sid = attach->sid;
    node->node_id.timeout = 0;
    node->type = type;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "update_address_cache, ueid %x, sid %x, netid %x, attach_sid %x\r\n",
           node->node_id.ueid[0], node->node_id.sid, node->node_id.meshnetid,
           node->node_id.attach_sid);
    return UR_ERROR_NONE;
}

void get_attach_by_nodeid(ur_node_id_t *attach, ur_node_id_t *target)
{
    sid_node_t *node = NULL;

    if (attach == NULL || target == NULL) {
        return;
    }
    attach->sid = INVALID_SID;
    attach->meshnetid = INVALID_NETID;
    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        if (node->node_id.sid == target->sid &&
            node->node_id.meshnetid == target->meshnetid) {
            memcpy(target->ueid, node->node_id.ueid, sizeof(target->ueid));
            break;
        }
    }
    if (node) {
        attach->sid = node->node_id.attach_sid;
        attach->meshnetid = node->node_id.meshnetid;
    }
}

void get_target_by_ueid(ur_node_id_t *node_id, uint8_t *ueid)
{
    sid_node_t *node;
    network_context_t *network;

    if (memcmp(ueid, umesh_mm_get_local_ueid(), 8) == 0) {
        node_id->sid = umesh_mm_get_local_sid();
        network = get_default_network_context();
        node_id->meshnetid = umesh_mm_get_meshnetid(network);
        memcpy(node_id->ueid, umesh_mm_get_local_ueid(), sizeof(node_id->ueid));
        return;
    }

    node_id->sid = INVALID_SID;
    node_id->meshnetid = INVALID_NETID;
    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        if (memcmp(node->node_id.ueid, ueid, sizeof(node_id->ueid)) == 0) {
            node_id->sid = node->node_id.sid;
            node_id->meshnetid = node->node_id.meshnetid;
            memcpy(node_id->ueid, node->node_id.ueid, sizeof(node_id->ueid));
            break;
        }
    }
}

void start_addr_cache(void)
{
    slist_init(&g_ac_state.cache_list);
    g_ac_state.cache_num = 0;
    g_ac_state.timer = ur_start_timer(ADDR_CACHE_CHECK_INTERVAL,
                                      handle_addr_cache_timer, NULL);
}

void stop_addr_cache(void)
{
    sid_node_t *node;

    while (!slist_empty(&g_ac_state.cache_list)) {
        node = slist_first_entry(&g_ac_state.cache_list, sid_node_t, next);
        slist_del(&node->next, &g_ac_state.cache_list);
        ur_mem_free(node, sizeof(sid_node_t));
    }

    ur_stop_timer(&g_ac_state.timer, NULL);
    g_ac_state.cache_num = 0;
}
