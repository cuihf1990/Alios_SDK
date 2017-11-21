/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>

#include "core/mesh_mgmt.h"
#include "core/mesh_forwarder.h"
#include "core/link_mgmt.h"
#include "core/network_data.h"
#include "core/sid_allocator.h"
#include "core/address_mgmt.h"
#include "core/mesh_mgmt_tlvs.h"
#include "umesh_utils.h"
#include "hal/interfaces.h"

static neighbor_updated_t g_neighbor_updater_head;

static void handle_link_request_timer(void *args)
{
    neighbor_t *attach_node;
    hal_context_t *hal = (hal_context_t *)args;
    network_context_t *network;
    uint8_t tlv_type[1] = {TYPE_UCAST_CHANNEL};
    ur_addr_t addr;

    MESH_LOG_DEBUG("handle link request timer");

    hal->link_request_timer = NULL;
    network = get_default_network_context();
    attach_node = umesh_mm_get_attach_node();
    if (attach_node == NULL) {
        return;
    }

    set_mesh_short_addr(&addr, attach_node->netid, attach_node->sid);
    send_link_request(network, &addr, tlv_type, sizeof(tlv_type));
    if (attach_node->stats.link_request < LINK_ESTIMATE_SENT_THRESHOLD) {
        hal->link_request_timer = ur_start_timer(hal->link_request_interval,
                                                 handle_link_request_timer, hal);
    }
}

static ur_error_t update_link_cost(link_nbr_stats_t *stats)
{
    uint32_t new;
    uint32_t old;

    if (stats->link_request < LINK_ESTIMATE_SENT_THRESHOLD) {
        return UR_ERROR_FAIL;
    }

    if (stats->link_accept) {
        new = ((uint32_t)LINK_ESTIMATE_COEF * stats->link_request) / stats->link_accept;
    } else {
        new = LINK_COST_MAX;
    }

    if (stats->link_cost == 0xffff) {
        stats->link_cost = new;
    } else {
        old = stats->link_cost;
        stats->link_cost = ((uint32_t)((LINK_ESTIMATE_COEF - LINK_ESTIMATE_UPDATE_ALPHA)
                                       * old) +
                            ((uint32_t)(LINK_ESTIMATE_UPDATE_ALPHA * new))) / LINK_ESTIMATE_COEF;
        if ((stats->link_cost == old) && (!stats->link_accept)) {
            stats->link_cost += LINK_ESTIMATE_COEF;
        }
    }
    stats->link_request = 0;
    stats->link_accept = 0;
    return UR_ERROR_NONE;
}

static ur_error_t remove_neighbor(hal_context_t *hal, neighbor_t *neighbor)
{
    network_context_t *network;
    ur_node_id_t node_id;

    if (neighbor == NULL) {
        return UR_ERROR_NONE;
    }

    network = get_network_context_by_meshnetid(neighbor->netid);
    if (network && network->router->sid_type == STRUCTURED_SID &&
        is_allocated_child(network->sid_base, neighbor)) {
        node_id.sid = neighbor->sid;
        memcpy(node_id.ueid, neighbor->mac, sizeof(node_id.ueid));
        update_sid_mapping(network->sid_base, &node_id, false);
    }

    slist_del(&neighbor->next, &hal->neighbors_list);
    ur_mem_free(neighbor->one_time_key, KEY_SIZE);
    ur_mem_free(neighbor, sizeof(neighbor_t));
    hal->neighbors_num--;
    return UR_ERROR_NONE;
}

static void handle_link_quality_update_timer(void *args)
{
    ur_error_t error;
    neighbor_t *nbr;
    hal_context_t *hal = (hal_context_t *)args;

    MESH_LOG_DEBUG("handle link quality update timer");

    hal->link_quality_update_timer = ur_start_timer(hal->link_request_interval * LINK_ESTIMATE_TIMES,
                                                    handle_link_quality_update_timer, hal);

    slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
        if ((umesh_now_ms() - nbr->last_heard) > (hal->neighbor_alive_interval / 2)) {
            nbr->flags &= (~NBR_LINK_ESTIMATED);
        }
        error = update_link_cost(&nbr->stats);
        if (nbr == umesh_mm_get_attach_node()) {
            if (error == UR_ERROR_NONE && nbr->stats.reverse_rssi >= RSSI_THRESHOLD) {
                nbr->flags |= NBR_LINK_ESTIMATED;
            } else if ((nbr->flags & NBR_LINK_ESTIMATED) == 0 && hal->link_request_timer == NULL) {
                hal->link_request_timer = ur_start_timer(hal->link_request_interval,
                                                         handle_link_request_timer, hal);
            }
        }
        if (nbr->stats.link_cost >= LINK_COST_THRESHOLD) {
            nbr->state = STATE_INVALID;
            g_neighbor_updater_head(nbr);
            remove_neighbor(hal, nbr);
        }
    }
}

static neighbor_t *new_neighbor(hal_context_t *hal, const mac_address_t *addr,
                                uint8_t *tlvs, uint16_t length, bool is_attach)
{
    neighbor_t *nbr;

    slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
        if (nbr->state == STATE_INVALID) {
            goto get_nbr;
        }
    }

    if (hal->neighbors_num < MAX_NEIGHBORS_NUM) {
        nbr = (neighbor_t *)ur_mem_alloc(sizeof(neighbor_t));
        memset(nbr, 0, sizeof(neighbor_t));
        if (nbr == NULL) {
            return NULL;
        }

        slist_add(&nbr->next, &hal->neighbors_list);
        hal->neighbors_num++;
        goto get_nbr;
    }

    if (!is_attach) {
        return NULL;
    }

    slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
        if (nbr->state == STATE_PARENT) {
            continue;
        }
        if (nbr->state == STATE_CHILD) {
            continue;
        }

        MESH_LOG_INFO("sid %04x mac " EXT_ADDR_FMT " is replaced",
                      nbr->sid, EXT_ADDR_DATA(nbr->mac));
        goto get_nbr;
    }
    return NULL;

get_nbr:
    memcpy(nbr->mac, addr->addr, sizeof(nbr->mac));
    nbr->netid = BCAST_NETID;
    nbr->sid = BCAST_SID;
    nbr->path_cost          = INFINITY_PATH_COST;
    nbr->mode               = 0;
    nbr->stats.link_cost    = 256;
    nbr->stats.link_request = 0;
    nbr->stats.link_accept  = 0;
    nbr->state              = STATE_INVALID;
    nbr->flags              = 0;
    nbr->last_heard         = umesh_now_ms();
    return nbr;
}

static void handle_update_nbr_timer(void *args)
{
    neighbor_t    *node;
    hal_context_t *hal = (hal_context_t *)args;
    uint16_t sid = umesh_mm_get_local_sid();
    network_context_t *network = NULL;
    ur_node_id_t node_id;

    hal->update_nbr_timer = ur_start_timer(hal->advertisement_interval,
                                           handle_update_nbr_timer, hal);
    slist_for_each_entry(&hal->neighbors_list, node, neighbor_t, next) {
        if (node->state < STATE_NEIGHBOR || (node->mode & MODE_RX_ON) == 0) {
            continue;
        }

        if (node->attach_candidate_timeout > 0) {
            node->attach_candidate_timeout--;
        }

        if ((umesh_now_ms() - node->last_heard) < hal->neighbor_alive_interval) {
            continue;
        }

        MESH_LOG_INFO("%x neighbor " EXT_ADDR_FMT " become inactive",
                      sid, EXT_ADDR_DATA(node->mac));
        network = get_network_context_by_meshnetid(node->netid);
        if (network && network->router->sid_type == STRUCTURED_SID &&
            node->state == STATE_CHILD) {
            node_id.sid = node->sid;
            memcpy(node_id.ueid, node->mac, sizeof(node_id.ueid));
            update_sid_mapping(network->sid_base, &node_id, false);
        }
        node->state = STATE_INVALID;
        g_neighbor_updater_head(node);
        remove_neighbor(hal, node);
    }
}

void neighbors_init(void)
{
    neighbor_t *node;
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        while (!slist_empty(&hal->neighbors_list)) {
            node = slist_first_entry(&hal->neighbors_list, neighbor_t, next);
            remove_neighbor(hal, node);
        }
        slist_init(&hal->neighbors_list);
        hal->neighbors_num  = 0;
    }
}

neighbor_t *update_neighbor(const message_info_t *info,
                            uint8_t *tlvs, uint16_t length, bool is_attach)
{
    neighbor_t        *nbr = NULL;
    mm_cost_tv_t      *path_cost = NULL;
    mm_ssid_info_tv_t *ssid_info = NULL;
    mm_channel_tv_t   *channel;
    hal_context_t     *hal;
    network_context_t *network;
    uint8_t channel_orig;
    ur_node_id_t node_id;

    MESH_LOG_DEBUG("update neighbor");

    hal = get_hal_context(info->hal_type);
    nbr = get_neighbor_by_mac_addr(info->src_mac.addr.addr);

    if (length == 0) {
        goto exit;
    }

    path_cost = (mm_cost_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_PATH_COST);
    ssid_info = (mm_ssid_info_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_SSID_INFO);
    channel = (mm_channel_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_UCAST_CHANNEL);

    // remove nbr, if mode changed
    if (nbr && nbr->mode != info->mode) {
        remove_neighbor(hal, nbr);
        nbr = NULL;
    }

    if (nbr == NULL) {
        nbr = new_neighbor(hal, &info->src_mac.addr, tlvs, length, is_attach);
    } else if (is_attach) {
        /* move attaching neighbor to the head of list */
        slist_del(&nbr->next, &hal->neighbors_list);
        slist_add_tail(&nbr->next, &hal->neighbors_list);
    }
    if (nbr == NULL) {
        return NULL;
    }

    nbr->mode = (node_mode_t)info->mode;
    if (nbr->state < STATE_CANDIDATE) {
        nbr->state = STATE_NEIGHBOR;
        nbr->stats.link_cost = 256;
    }
    if (path_cost != NULL) {
        nbr->path_cost = path_cost->cost;
    }

    if (nbr->sid != info->src.addr.short_addr) {
        nbr->flags |= NBR_SID_CHANGED;
    }

    if (nbr->netid != info->src.netid) {
        nbr->flags |= NBR_NETID_CHANGED;
    }

    channel_orig = nbr->channel;
    if (channel) {
        nbr->channel = channel->channel;
    } else {
        nbr->channel = info->src_channel;
    }
    if (channel_orig && nbr->channel != channel_orig) {
        nbr->flags |= NBR_CHANNEL_CHANGED;
    }

    network = info->network;
    if (network->router->sid_type == STRUCTURED_SID) {
        if (ssid_info != NULL) {
            nbr->ssid_info.child_num = ssid_info->child_num;
            nbr->ssid_info.free_slots = ssid_info->free_slots;
        }

        if (network->sid_base && (nbr->flags & (NBR_SID_CHANGED | NBR_NETID_CHANGED))) {
            if (nbr->state == STATE_CHILD ||
                (!is_direct_child(network->sid_base, info->src.addr.short_addr) &&
                 is_allocated_child(network->sid_base, nbr) &&
                 is_partial_function_sid(info->src.addr.short_addr) == false)) {
                node_id.sid = nbr->sid;
                memcpy(node_id.ueid, nbr->mac, sizeof(node_id.ueid));
                update_sid_mapping(network->sid_base, &node_id, false);
                nbr->state = STATE_NEIGHBOR;
            }
        }

        network = get_network_context_by_meshnetid(info->src.netid);
        if (network && is_direct_child(network->sid_base, info->src.addr.short_addr) &&
            is_allocated_child(network->sid_base, nbr)) {
            nbr->state = STATE_CHILD;
        }
    }

    if (hal->update_nbr_timer == NULL && (umesh_mm_get_mode() & MODE_RX_ON)) {
        hal->update_nbr_timer = ur_start_timer(hal->advertisement_interval,
                                               handle_update_nbr_timer, hal);
    }
    nbr->sid = info->src.addr.short_addr;
    nbr->netid = info->src.netid;
    g_neighbor_updater_head(nbr);

exit:
    if (nbr) {
        nbr->stats.reverse_rssi = info->reverse_rssi;
        if (nbr->stats.reverse_rssi < RSSI_THRESHOLD) {
            nbr->flags &= (~NBR_LINK_ESTIMATED);
        }
        if (info->forward_rssi != 127) {
            nbr->stats.forward_rssi = info->forward_rssi;
        }
        nbr->last_heard = umesh_now_ms();
    }
    return nbr;
}

void set_state_to_neighbor(void)
{
    neighbor_t *node;
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        slist_for_each_entry(&hal->neighbors_list, node, neighbor_t, next) {
            if (node->state == STATE_PARENT) {
                node->state = STATE_NEIGHBOR;
            }
            if (node->state == STATE_CHILD) {
                node->state = STATE_NEIGHBOR;
            }
        }
    }
}

neighbor_t *get_neighbor_by_mac_addr(const uint8_t *addr)
{
    neighbor_t *nbr = NULL;
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
            if (memcmp(addr, nbr->mac, sizeof(nbr->mac)) == 0 &&
                nbr->state > STATE_INVALID) {
                return nbr;
            }
        }
    }
    return nbr;
}

neighbor_t *get_neighbor_by_sid(hal_context_t *hal, uint16_t sid,
                                uint16_t meshnetid)
{
    slist_t *hals;
    neighbor_t *node;

    if (hal == NULL) {
        hals = get_hal_contexts();
        hal = slist_first_entry(hals, hal_context_t, next);
    }
    if (hal == NULL || sid == INVALID_SID || sid == BCAST_SID) {
        return NULL;
    }

    slist_for_each_entry(&hal->neighbors_list, node, neighbor_t, next) {
        if (node->sid == sid && node->netid == meshnetid &&
            node->state > STATE_INVALID) {
            return node;
        }
    }
    return NULL;
}

neighbor_t *get_neighbors(uint16_t *num)
{
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    hal = slist_first_entry(hals, hal_context_t, next);
    if (num != NULL) {
        *num = hal->neighbors_num;
    }
    return slist_first_entry(&hal->neighbors_list, neighbor_t, next);
}

ur_error_t send_link_request(network_context_t *network, ur_addr_t *dest,
                             uint8_t *tlvs, uint8_t tlvs_length)
{
    ur_error_t error = UR_ERROR_MEM;
    mm_tlv_request_tlv_t *request_tlvs;
    message_t            *message;
    uint8_t              *data;
    uint8_t *data_orig;
    uint16_t             length;
    message_info_t *info;
    neighbor_t *nbr;

    nbr = get_neighbor_by_sid(network->hal, dest->addr.short_addr,
                              dest->netid);
    if (nbr == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t);
    if (tlvs_length) {
        length += (tlvs_length + sizeof(mm_tlv_request_tlv_t));
    }

    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);

    if (tlvs_length) {
        request_tlvs = (mm_tlv_request_tlv_t *)data;
        umesh_mm_init_tlv_base((mm_tlv_t *)request_tlvs, TYPE_TLV_REQUEST, tlvs_length);
        data += sizeof(mm_tlv_request_tlv_t);
        memcpy(data, tlvs, tlvs_length);
        data += tlvs_length;
    }

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_LINK_REQUEST,
                               data_orig, length, LINK_MGMT_1);
    if (message) {
        info = message->info;
        info->network = network;
        memcpy(&info->dest, dest, sizeof(info->dest));
        error = mf_send_message(message);
    }
    ur_mem_free(data_orig, length);

    nbr->stats.link_request++;

    MESH_LOG_DEBUG("send link request, len %d", length);
    return error;
}

static ur_error_t send_link_accept_and_request(network_context_t *network,
                                               ur_addr_t *dest,
                                               uint8_t *tlvs,
                                               uint8_t tlvs_length)
{
    ur_error_t error = UR_ERROR_MEM;
    message_t   *message;
    uint8_t     *data;
    uint8_t *data_orig;
    int16_t     length;
    neighbor_t  *node;
    message_info_t *info;

    node = get_neighbor_by_mac_addr(dest->addr.addr);
    if (node == NULL) {
        return UR_ERROR_FAIL;
    }

    length = tlvs_calc_length(tlvs, tlvs_length);
    if (length < 0) {
        return UR_ERROR_FAIL;
    }
    length += sizeof(mm_header_t);

    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);
    data += tlvs_set_value(network, data, tlvs, tlvs_length);

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_LINK_ACCEPT_AND_REQUEST,
                               data_orig, length, LINK_MGMT_2);
    if (message) {
        info = message->info;
        info->network = network;
        memcpy(&info->dest, dest, sizeof(info->dest));
        error = mf_send_message(message);
    }
    ur_mem_free(data_orig, length);

    node->stats.link_request++;

    MESH_LOG_DEBUG("send link accept and request, len %d", length);
    return error;
}

static ur_error_t send_link_accept(network_context_t *network,
                                   ur_addr_t *dest,
                                   uint8_t *tlvs, uint8_t tlvs_length)
{
    ur_error_t error = UR_ERROR_MEM;
    message_t   *message;
    uint8_t     *data;
    uint8_t *data_orig;
    int16_t     length;
    neighbor_t  *node;
    message_info_t *info;

    node = get_neighbor_by_mac_addr(dest->addr.addr);
    if (node == NULL) {
        return UR_ERROR_FAIL;
    }

    length = tlvs_calc_length(tlvs, tlvs_length);
    if (length < 0) {
        return UR_ERROR_FAIL;
    }
    length += sizeof(mm_header_t);

    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);
    data += tlvs_set_value(network, data, tlvs, tlvs_length);

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_LINK_ACCEPT,
                               data_orig, length, LINK_MGMT_3);
    if (message) {
        info = message->info;
        info->network = network;
        memcpy(&info->dest, dest, sizeof(info->dest));
        error = mf_send_message(message);
    }
    ur_mem_free(data_orig, length);

    MESH_LOG_DEBUG("send link accept, len %d", length);
    return error;
}

uint8_t insert_mesh_header_ies(network_context_t *network,
                               message_info_t *info, int16_t hdr_ies_limit)
{
    hal_context_t *hal;
    mesh_header_control_t *control;
    uint8_t offset = 0;
    mm_tv_t *tv;
    mm_rssi_tv_t *rssi;
    neighbor_t *nbr;

    if (hdr_ies_limit < (sizeof(mm_mode_tv_t) + sizeof(mm_tv_t))) {
        return 0;
    }

    hal = network->hal;
    control = (mesh_header_control_t *)hal->frame.data;
    control->control[1] |= (1 << MESH_HEADER_IES_OFFSET);

    offset += set_mm_mode_tv(hal->frame.data + info->header_ies_offset);
    if (hdr_ies_limit < (offset + sizeof(mm_rssi_tv_t) + sizeof(mm_tv_t))) {
        goto exit;
    }

    nbr = get_neighbor_by_sid(hal, info->dest.addr.short_addr,
                              info->dest.netid);
    if (nbr) {
        rssi = (mm_rssi_tv_t *)(hal->frame.data + info->header_ies_offset +
                                offset);
        umesh_mm_init_tv_base((mm_tv_t *)rssi, TYPE_REVERSE_RSSI);
        if ((nbr->flags & NBR_LINK_ESTIMATED) == 0) {
            if (nbr->stats.reverse_rssi > RSSI_THRESHOLD) {
                nbr->flags |= NBR_LINK_ESTIMATED;
            }
            rssi->rssi = 127;
            nbr->stats.link_request++;
        } else {
            rssi->rssi = nbr->stats.reverse_rssi;
        }
        offset += sizeof(mm_rssi_tv_t);
    }

exit:
    tv = (mm_tv_t *)(hal->frame.data + info->header_ies_offset + offset);
    tv->type = TYPE_HEADER_IES_TERMINATOR;
    offset += sizeof(mm_tv_t);

    info->payload_offset += offset;

    return offset;
}

ur_error_t handle_mesh_header_ies(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
    message_info_t *info;
    uint8_t offset;
    uint8_t len;
    mm_tv_t tv;
    mm_rssi_tv_t rssi;
    mm_mode_tv_t mode;

    info = message->info;
    offset = info->header_ies_offset;
    message_copy_to(message, offset, (uint8_t *)&tv, sizeof(tv));

    while (tv.type != TYPE_HEADER_IES_TERMINATOR) {
        switch (tv.type) {
            case TYPE_REVERSE_RSSI:
                message_copy_to(message, offset, (uint8_t *)&rssi, sizeof(rssi));
                if (rssi.rssi == 127) {
                    send_link_accept(info->network, &info->src_mac, NULL, 0);
                }
                info->forward_rssi = rssi.rssi;
                len = sizeof(mm_rssi_tv_t);
                break;
            case TYPE_MODE:
                message_copy_to(message, offset, (uint8_t *)&mode, sizeof(mode));
                info->mode = mode.mode;
                len = sizeof(mm_mode_tv_t);
                break;
            default:
                error = UR_ERROR_PARSE;
        }

        if (error != UR_ERROR_NONE) {
            break;
        }

        offset += len;
        message_copy_to(message, offset, (uint8_t *)&tv, sizeof(tv));
    }

    offset += sizeof(mm_tv_t);
    info->payload_offset = offset;
    return error;
}

ur_error_t handle_link_request(message_t *message)
{
    mm_tlv_request_tlv_t *tlvs_request;
    uint8_t              *tlvs;
    uint16_t             tlvs_length;
    message_info_t       *info;
    network_context_t    *network;
    uint8_t *data;
    uint16_t length;

    MESH_LOG_DEBUG("handle link request");

    info = message->info;
    network = info->network;
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    data = ur_mem_alloc(tlvs_length);
    length = tlvs_length;
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    tlvs = data;
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);
    tlvs_request = (mm_tlv_request_tlv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                           TYPE_TLV_REQUEST);
    if (tlvs_request) {
        tlvs = (uint8_t *)tlvs_request + sizeof(mm_tlv_t);
        tlvs_length = tlvs_request->base.length;
    } else {
        tlvs = NULL;
        tlvs_length = 0;
    }
    send_link_accept_and_request(network, &info->src_mac, tlvs, tlvs_length);
    ur_mem_free(data, length);
    return UR_ERROR_NONE;
}

ur_error_t handle_link_accept_and_request(message_t *message)
{
    uint8_t *tlvs;
    uint16_t tlvs_length;
    mm_tlv_request_tlv_t *tlvs_request;
    mm_channel_tv_t *channel;
    neighbor_t *node;
    message_info_t *info;
    network_context_t *network;
    uint8_t local_channel;
    uint8_t *data;
    uint16_t length;

    MESH_LOG_DEBUG("handle link accept and resquest");

    info = message->info;
    network = info->network;
    node = get_neighbor_by_mac_addr(info->src_mac.addr.addr);
    if (node == NULL) {
        return UR_ERROR_NONE;
    }

    node->stats.link_accept++;

    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    data = ur_mem_alloc(tlvs_length);
    length = tlvs_length;
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    tlvs = data;
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);

    channel = (mm_channel_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                 TYPE_UCAST_CHANNEL);
    if (channel) {
        local_channel = umesh_mm_get_channel(network);
        if (local_channel != channel->channel) {
            umesh_mm_set_channel(network, channel->channel);
        }
    }

    tlvs_request = (mm_tlv_request_tlv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                           TYPE_TLV_REQUEST);
    if (tlvs_request) {
        tlvs = (uint8_t *)tlvs_request + sizeof(mm_tlv_t);
        tlvs_length = tlvs_request->base.length;
    } else {
        tlvs = NULL;
        tlvs_length = 0;
    }
    send_link_accept(network, &info->src_mac, tlvs, tlvs_length);
    ur_mem_free(data, length);
    return UR_ERROR_NONE;
}

ur_error_t handle_link_accept(message_t *message)
{
    neighbor_t *node;
    message_info_t *info;

    MESH_LOG_DEBUG("handle link accept");
    info = message->info;
    node = get_neighbor_by_mac_addr(info->src_mac.addr.addr);
    if (node == NULL) {
        return UR_ERROR_NONE;
    }

    node->stats.link_accept++;
    return UR_ERROR_NONE;
}

void start_neighbor_updater(void)
{
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        hal->link_quality_update_timer =
             ur_start_timer(hal->link_request_interval * LINK_ESTIMATE_TIMES,
                            handle_link_quality_update_timer, hal);
    }
}

void stop_neighbor_updater(void)
{
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        ur_stop_timer(&hal->link_quality_update_timer, hal);
        ur_stop_timer(&hal->update_nbr_timer, hal);
        ur_stop_timer(&hal->link_request_timer, hal);
    }
}

ur_error_t register_neighbor_updater(neighbor_updated_t updater)
{
    g_neighbor_updater_head = updater;
    return UR_ERROR_NONE;
}

