/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <assert.h>
#include <string.h>

#include "core/address_mgmt.h"
#include "core/mesh_mgmt.h"
#include "core/mesh_forwarder.h"
#include "core/router_mgr.h"
#include "core/network_data.h"
#include "core/link_mgmt.h"
#include "hal/hals.h"
#include "hal/interfaces.h"
#include "umesh_utils.h"

typedef struct address_resolver_state_s {
    address_cache_t cache[UR_MESH_ADDRESS_CACHE_SIZE];
    ur_timer_t timer;
} address_resolver_state_t;

typedef struct address_cache_state_s {
    slist_t cache_list;
    uint16_t cache_num;
    ur_timer_t timer;
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

static ur_error_t get_target_by_ueid(ur_node_id_t *node_id, uint8_t *ueid)
{
    sid_node_t *node;

    if (memcmp(ueid, umesh_mm_get_local_ueid(), 8) == 0) {
        node_id->sid = umesh_mm_get_local_sid();
        node_id->meshnetid = umesh_mm_get_meshnetid(NULL);
        return UR_ERROR_NONE;
    }

    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        if (memcmp(node->node_id.ueid, ueid, sizeof(node_id->ueid)) == 0) {
            node_id->sid = node->node_id.sid;
            node_id->meshnetid = node->node_id.meshnetid;
            return UR_ERROR_NONE;
        }
    }
    return UR_ERROR_FAIL;
}

static ur_error_t get_attach_by_sid(ur_node_id_t *attach, uint16_t netid, uint16_t sid)
{
    sid_node_t *node = NULL;

    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        if (node->node_id.sid == sid && node->node_id.meshnetid == netid) {
            attach->sid = node->node_id.attach_sid;
            attach->meshnetid = node->node_id.meshnetid;
            return UR_ERROR_NONE;
        }
    }

    return UR_ERROR_FAIL;
}

static void set_dest_info(message_info_t *info, address_cache_t *target)
{
    if (is_partial_function_sid(target->sid)) {
        set_mesh_short_addr(&info->dest2, target->meshnetid, target->sid);
        set_mesh_short_addr(&info->dest, target->meshnetid, target->attach_sid);
    } else {
        set_mesh_short_addr(&info->dest, target->meshnetid, target->sid);
    }
}

static void address_resolved_handler(network_context_t *network,
                                     address_cache_t *target,
                                     ur_error_t error)
{
    message_t *message;
    message_info_t *info;
    hal_context_t *hal;
    bool matched = false;

    hal = network->hal;
    for_each_message(message, &hal->send_queue[PENDING_QUEUE]) {
        info = message->info;
        if (info->dest.addr.len == SHORT_ADDR_SIZE) {
            if (info->dest.addr.short_addr == target->sid &&
                info->dest.netid == target->meshnetid) {
                matched = true;
            }
        } else if (info->dest.addr.len == EXT_ADDR_SIZE) {
            if (memcmp(target->ueid, info->dest.addr.addr, sizeof(target->ueid)) == 0) {
                matched = true;
            }
        }

        if (matched == false) {
            continue;
        }
        message_queue_dequeue(message);
        if (error == UR_ERROR_NONE) {
            set_dest_info(info, target);
            mf_send_message(message);
        } else {
            message_free(message);
        }
    }
}

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
                network = get_default_network_context();
                address_resolved_handler(network, &g_ar_state.cache[index], UR_ERROR_DROP);
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

ur_error_t address_resolve(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
    uint8_t index = 0;
    address_cache_t *cache = NULL;
    network_context_t *network;
    ur_addr_t dest;
    neighbor_t *nbr = NULL;
    uint8_t query_type;
    ur_node_id_t target;
    message_info_t *info;
    hal_context_t *hal;

    info = message->info;
    if (info->dest.addr.len == SHORT_ADDR_SIZE &&
        info->dest.addr.short_addr == BCAST_SID) {
        if (info->type == MESH_FRAME_TYPE_DATA) {
            info->flags |= INSERT_MCAST_FLAG;
        }
        return UR_ERROR_NONE;
    }

    nbr = mf_get_neighbor(info->type, info->dest.netid, &info->dest.addr);
    if (nbr) {
        set_mesh_ext_addr(&info->dest, nbr->netid, nbr->mac);
        return UR_ERROR_NONE;
    } else if (info->dest.addr.len == SHORT_ADDR_SIZE &&
               is_partial_function_sid(info->dest.addr.short_addr) == false) {
        return UR_ERROR_NONE;
    }

    if (info->dest.addr.len == SHORT_ADDR_SIZE) {
        query_type = ATTACH_QUERY;
        target.sid = info->dest.addr.short_addr;
        target.meshnetid = info->dest.netid;
    } else {
        query_type = TARGET_QUERY;
        memcpy(target.ueid, info->dest.addr.addr, sizeof(target.ueid));
    }

    for (index = 0; index < UR_MESH_ADDRESS_CACHE_SIZE; index++) {
        if (g_ar_state.cache[index].state != AQ_STATE_INVALID) {
            if (query_type == ATTACH_QUERY &&
                g_ar_state.cache[index].meshnetid == target.meshnetid &&
                g_ar_state.cache[index].sid == target.sid) {
                cache = &g_ar_state.cache[index];
                break;
            } else if (query_type == TARGET_QUERY &&
                       memcmp(target.ueid, g_ar_state.cache[index].ueid,
                              sizeof(g_ar_state.cache[index].ueid)) == 0) {
                cache = &g_ar_state.cache[index];
                break;
            }
        } else if (cache == NULL) {
            cache = &g_ar_state.cache[index];
        }
    }

    if (cache == NULL) {
        return UR_ERROR_DROP;
    }

    network = get_default_network_context();
    get_leader_addr(&dest);
    switch (cache->state) {
        case AQ_STATE_INVALID:
            memcpy(cache->ueid, target.ueid, sizeof(cache->ueid));
            cache->sid = target.sid;
            cache->meshnetid = target.meshnetid;
            cache->attach_sid = BCAST_SID;
            cache->attach_netid = BCAST_NETID;
            cache->timeout = ADDRESS_QUERY_TIMEOUT;
            cache->retry_timeout = ADDRESS_QUERY_RETRY_TIMEOUT;
            cache->state = AQ_STATE_QUERY;
            send_address_query(network, &dest, query_type, &target);
            error = UR_ERROR_ADDRESS_QUERY;
            break;
        case AQ_STATE_QUERY:
            if (cache->timeout > 0) {
                error = UR_ERROR_ADDRESS_QUERY;
            } else if (cache->timeout == 0 && cache->retry_timeout == 0) {
                cache->timeout = ADDRESS_QUERY_TIMEOUT;
                send_address_query(network, &dest, query_type, &target);
                error = UR_ERROR_ADDRESS_QUERY;
            } else {
                error = UR_ERROR_DROP;
            }
            break;
        case AQ_STATE_CACHED:
            break;
        default:
            assert(0);
            break;
    }

    if (error == UR_ERROR_ADDRESS_QUERY) {
        hal = get_default_hal_context();
        message_queue_enqueue(&hal->send_queue[PENDING_QUEUE], message);
    } else if (error == UR_ERROR_NONE) {
        set_dest_info(info, cache);
    }

    return error;
}

static ur_error_t send_address_query(network_context_t *network,
                                     ur_addr_t *dest,
                                     uint8_t query_type, ur_node_id_t *target)
{
    ur_error_t error = UR_ERROR_FAIL;
    mm_addr_query_tv_t *addr_query;
    message_t *message;
    uint8_t *data;
    uint8_t *data_orig;
    uint16_t length;
    message_info_t *info;

    length = sizeof(mm_header_t) + sizeof(mm_addr_query_tv_t);
    if (query_type == ATTACH_QUERY) {
        length += sizeof(mm_node_id_tv_t);
    } else if (query_type == TARGET_QUERY) {
        length += sizeof(mm_ueid_tv_t);
    } else {
        return UR_ERROR_FAIL;
    }

    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);

    addr_query = (mm_addr_query_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)addr_query, TYPE_ADDR_QUERY);
    addr_query->query_type = query_type;
    data += sizeof(mm_addr_query_tv_t);

    switch (query_type) {
        case ATTACH_QUERY:
            data += set_mm_node_id_tv(data, TYPE_NODE_ID, target);
            break;
        case TARGET_QUERY:
            data += set_mm_ueid_tv(data, TYPE_TARGET_UEID, target->ueid);
            break;
        default:
            assert(0);
            break;
    }

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_ADDRESS_QUERY,
                               data_orig, length, ADDRESS_MGMT_1);
    if (message == NULL) {
        goto exit;
    }

    if (g_ar_state.timer == NULL) {
        g_ar_state.timer = ur_start_timer(ADDRESS_QUERY_STATE_UPDATE_PERIOD,
                                          timer_handler, NULL);
    }

    info = message->info;
    info->network = network;
    memcpy(&info->dest, dest, sizeof(info->dest));
    error = mf_send_message(message);
    MESH_LOG_DEBUG("send address query, len %d", length);

exit:
    ur_mem_free(data_orig, length);
    return error;
}

ur_error_t handle_address_query(message_t *message)
{
    ur_error_t error = UR_ERROR_FAIL;
    mm_addr_query_tv_t *addr_query;
    uint8_t *tlvs;
    ur_node_id_t target_node;
    ur_node_id_t attach_node;
    mm_node_id_tv_t *target_id;
    mm_ueid_tv_t *ueid;
    uint16_t tlvs_length;
    network_context_t *network;
    message_info_t *info;

    if (umesh_mm_get_device_state() < DEVICE_STATE_LEADER) {
        return UR_ERROR_NONE;
    }

    info = message->info;
    network = info->network;
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    tlvs = ur_mem_alloc(tlvs_length);
    if (tlvs == NULL) {
        return UR_ERROR_MEM;
    }
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);

    addr_query = (mm_addr_query_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                       TYPE_ADDR_QUERY);
    if (addr_query == NULL) {
        goto exit;
    }

    attach_node.sid = INVALID_SID;
    attach_node.meshnetid = INVALID_NETID;

    switch (addr_query->query_type) {
        case ATTACH_QUERY:
            target_id = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_NODE_ID);
            if (target_id) {
                target_node.meshnetid = target_id->meshnetid;
                target_node.sid = target_id->sid;
                error = get_attach_by_sid(&attach_node, target_id->meshnetid, target_id->sid);
            }
            break;
        case TARGET_QUERY:
            ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);
            if (ueid) {
                error = get_target_by_ueid(&target_node, ueid->ueid);
                if (error == UR_ERROR_NONE && is_partial_function_sid(target_node.sid) == true) {
                    error = get_attach_by_sid(&attach_node, target_node.meshnetid, target_node.sid);
                }
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

exit:
    ur_mem_free(tlvs, tlvs_length);
    MESH_LOG_DEBUG("handle address query");
    return error;
}

static ur_error_t send_address_query_response(network_context_t *network,
                                              ur_addr_t *dest,
                                              ur_node_id_t *attach_node,
                                              ur_node_id_t *target_node)
{
    ur_error_t error = UR_ERROR_MEM;
    message_t   *message;
    uint8_t     *data;
    uint8_t *data_orig;
    uint16_t    length;
    message_info_t *info;

    length = sizeof(mm_header_t) + sizeof(mm_node_id_tv_t) +
             sizeof(mm_ueid_tv_t);
    if (attach_node->sid != INVALID_SID &&
        attach_node->meshnetid != INVALID_NETID) {
        length += sizeof(mm_node_id_tv_t);
    }

    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);
    data += set_mm_node_id_tv(data, TYPE_NODE_ID, target_node);
    data += set_mm_ueid_tv(data, TYPE_TARGET_UEID, target_node->ueid);

    if (attach_node->sid != INVALID_SID &&
        attach_node->meshnetid != INVALID_NETID) {
        data += set_mm_node_id_tv(data, TYPE_ATTACH_NODE_ID, attach_node);
    }

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_ADDRESS_QUERY_RESPONSE,
                               data_orig, length, ADDRESS_MGMT_2);
    if (message) {
        info = message->info;
        info->network = network;
        memcpy(&info->dest, dest, sizeof(info->dest));
        error = address_resolve(message);
        if (error == UR_ERROR_NONE) {
            error = mf_send_message(message);
        } else if (error == UR_ERROR_DROP) {
            message_free(message);
        }
    }
    ur_mem_free(data_orig, length);

    MESH_LOG_DEBUG("send address query response, len %d", length);
    return error;
}

ur_error_t handle_address_query_response(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
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
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    tlvs = ur_mem_alloc(tlvs_length);
    if (tlvs == NULL) {
        return UR_ERROR_MEM;
    }
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);

    attach_id = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                   TYPE_ATTACH_NODE_ID);
    target_id = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_NODE_ID);
    target_ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                  TYPE_TARGET_UEID);

    if (target_id == NULL || target_ueid == NULL) {
        error = UR_ERROR_FAIL;
        goto exit;
    }

    if (attach_id && (attach_id->sid == INVALID_SID ||
                      attach_id->meshnetid == INVALID_NETID)) {
        error = UR_ERROR_DROP;
        goto exit;
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
                address_resolved_handler(network, &g_ar_state.cache[index], error);
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
                address_resolved_handler(network, &g_ar_state.cache[index], error);
            }
            break;
        }
    }

exit:
    ur_mem_free(tlvs, tlvs_length);
    MESH_LOG_DEBUG("handle address query response");
    return UR_ERROR_NONE;
}

ur_error_t send_address_notification(network_context_t *network,
                                     ur_addr_t *dest)
{
    ur_error_t error = UR_ERROR_MEM;
    mm_hal_type_tv_t *hal_type;
    message_t       *message;
    uint8_t         *data;
    uint8_t *data_orig;
    uint16_t        length;
    message_info_t *info;
    hal_context_t   *hal;
    ur_node_id_t node_id;

    length = sizeof(mm_header_t) + sizeof(mm_ueid_tv_t) +
             sizeof(mm_node_id_tv_t) + sizeof(mm_hal_type_tv_t);
    if (network->attach_node) {
        length += sizeof(mm_node_id_tv_t);
    }

    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);
    data += set_mm_ueid_tv(data, TYPE_TARGET_UEID, umesh_mm_get_local_ueid());

    node_id.sid = umesh_mm_get_local_sid();
    node_id.meshnetid = umesh_mm_get_meshnetid(network);
    data += set_mm_node_id_tv(data, TYPE_NODE_ID, &node_id);

    hal_type = (mm_hal_type_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)hal_type, TYPE_DEF_HAL_TYPE);
    hal = get_default_hal_context();
    hal_type->type = hal->module->type;
    data += sizeof(mm_hal_type_tv_t);

    if (network->attach_node) {
        node_id.sid = network->attach_node->sid;
        node_id.meshnetid = network->attach_node->netid;
        data += set_mm_node_id_tv(data, TYPE_ATTACH_NODE_ID, &node_id);
    }

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_ADDRESS_NOTIFICATION,
                               data_orig, length, ADDRESS_MGMT_3);
    if (message) {
        info = message->info;
        info->network = network;
        if (dest == NULL) {
            get_leader_addr(&info->dest);
        } else {
            memcpy(&info->dest, dest, sizeof(info->dest));
        }
        error = mf_send_message(message);
    }
    ur_mem_free(data_orig, length);

    MESH_LOG_DEBUG("send address notification, len %d", length);
    return error;
}

ur_error_t send_address_unreachable(network_context_t *network,
                                    ur_addr_t *dest, ur_addr_t *target)
{
    ur_error_t error = UR_ERROR_MEM;
    message_t *message;
    uint8_t *data;
    uint8_t *data_orig;
    uint16_t length;
    message_info_t *info;
    ur_node_id_t node_id;

    if (target == NULL || target->addr.len != SHORT_ADDR_SIZE) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_node_id_tv_t);
    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);

    node_id.sid = target->addr.short_addr;
    node_id.meshnetid = target->netid;
    data += set_mm_node_id_tv(data, TYPE_NODE_ID, &node_id);

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_ADDRESS_UNREACHABLE,
                               data_orig, length, ADDRESS_MGMT_4);
    if (message) {
        info = message->info;
        info->network = network;
        memcpy(&info->dest, dest, sizeof(info->dest));
        error = mf_send_message(message);
    }
    ur_mem_free(data_orig, length);

    MESH_LOG_DEBUG("send address unreachable, len %d", length);
    return error;
}

ur_error_t handle_address_notification(message_t *message)
{
    ur_error_t error = UR_ERROR_FAIL;
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

    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    tlvs = ur_mem_alloc(tlvs_length);
    if (tlvs == NULL) {
        return UR_ERROR_MEM;
    }
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);

    attach_node = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                     TYPE_ATTACH_NODE_ID);
    target_node = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                     TYPE_NODE_ID);
    target_ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                  TYPE_TARGET_UEID);
    hal_type = (mm_hal_type_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                   TYPE_DEF_HAL_TYPE);

    if (target_node == NULL || target_ueid == NULL || hal_type == NULL) {
        goto exit;
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

exit:
    ur_mem_free(tlvs, tlvs_length);
    MESH_LOG_DEBUG("handle address notification");
    return error;
}

ur_error_t handle_address_unreachable(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
    mm_node_id_tv_t *target_node;
    uint8_t *tlvs;
    uint16_t tlvs_length;
    uint8_t index;
    message_info_t *info;

    info = message->info;

    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    tlvs = ur_mem_alloc(tlvs_length);
    if (tlvs == NULL) {
        return UR_ERROR_MEM;
    }
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);

    target_node = (mm_node_id_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                     TYPE_NODE_ID);

    if (target_node == NULL) {
        error = UR_ERROR_FAIL;
        goto exit;
    }

    for (index = 0; index < UR_MESH_ADDRESS_CACHE_SIZE; index++) {
        if (g_ar_state.cache[index].state == AQ_STATE_CACHED &&
            g_ar_state.cache[index].sid == target_node->sid &&
            g_ar_state.cache[index].meshnetid == target_node->meshnetid &&
            g_ar_state.cache[index].attach_sid == info->src.addr.short_addr &&
            g_ar_state.cache[index].attach_netid == info->src.netid) {
            g_ar_state.cache[index].state = AQ_STATE_INVALID;
            break;
        }
    }

    MESH_LOG_DEBUG("handle address unreachable");

exit:
    ur_mem_free(tlvs, tlvs_length);
    return error;
}

void address_resolver_init(void)
{
    memset(g_ar_state.cache, 0, sizeof(g_ar_state.cache));
}

static void handle_addr_cache_timer(void *args)
{
    sid_node_t        *node;
    uint8_t           timeout;
    network_context_t *network = NULL;
    slist_t           *tmp;

    g_ac_state.timer = NULL;
    slist_for_each_entry_safe(&g_ac_state.cache_list, tmp, node, sid_node_t, next) {
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
            sid_allocator_free(network, &node->node_id);
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

    MESH_LOG_DEBUG("update_address_cache, ueid %x, sid %x, netid %x, attach_sid %x",
                   node->node_id.ueid[0], node->node_id.sid, node->node_id.meshnetid,
                   node->node_id.attach_sid);
    return UR_ERROR_NONE;
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
