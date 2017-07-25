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
    slist_t *networks;
    network_context_t *network;
    uint32_t interval;
    uint8_t tlv_type[1] = {TYPE_UCAST_CHANNEL};

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link update timer\r\n");

    if (umesh_mm_get_mode() & MODE_MOBILE) {
        interval = hal->link_request_mobile_interval;
    } else {
        interval = hal->link_request_interval;
    }
    hal->link_request_timer = ur_start_timer(interval, handle_link_request_timer,
                                             hal);

    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        if (network->hal != hal) {
            continue;
        }

        attach_node = umesh_mm_get_attach_node(network);
        if (attach_node == NULL) {
            continue;
        }

        send_link_request(network, &attach_node->addr,
                          tlv_type, sizeof(tlv_type));
    }
}

static ur_error_t update_link_cost(link_nbr_stats_t *stats)
{
    uint32_t new;
    uint32_t old;

    if (stats->link_request < LINK_ESTIMATE_SENT_THRESHOLD) {
        return -1;
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
    return 0;
}

static void handle_link_quality_update_timer(void *args)
{
    neighbor_t *attach_node;
    hal_context_t *hal = (hal_context_t *)args;
    slist_t *networks;
    network_context_t *network;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "handle link quality update timer\r\n");

    hal->link_quality_update_timer = ur_start_timer(
                                         hal->link_request_interval * LINK_ESTIMATE_TIMES,
                                         handle_link_quality_update_timer, hal);
    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        if (network->hal != hal) {
            continue;
        }

        attach_node = umesh_mm_get_attach_node(network);
        if (attach_node == NULL) {
            continue;
        }

        update_link_cost(&attach_node->stats);
        if (attach_node->stats.link_cost >= LINK_COST_MAX) {
            attach_node->state = STATE_INVALID;
            g_neighbor_updater_head(attach_node);
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

        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
               "sid %04x ueid " EXT_ADDR_FMT " is replaced\n",
               nbr->addr.addr.short_addr, EXT_ADDR_DATA(nbr->ueid));
        goto get_nbr;
    }
    return NULL;

get_nbr:
    nbr->hal                = (void *)hal;
    memset(nbr->ueid, 0xff, sizeof(nbr->ueid));
    memcpy(&nbr->mac, addr, sizeof(nbr->mac));
    nbr->addr.netid         = BCAST_NETID;
    nbr->addr.addr.len        = SHORT_ADDR_SIZE;
    nbr->addr.addr.short_addr = BCAST_SID;
    nbr->path_cost          = INFINITY_PATH_COST;
    nbr->mode               = 0;
    nbr->stats.link_cost    = 256;
    nbr->stats.link_request = 0;
    nbr->stats.link_accept  = 0;
    nbr->state              = STATE_INVALID;
    nbr->flags              = 0;
    nbr->last_heard         = ur_get_now();

    return nbr;
}

static ur_error_t remove_neighbor(hal_context_t *hal, neighbor_t *neighbor)
{
    network_context_t *network;

    if (neighbor == NULL) {
        return UR_ERROR_NONE;
    }

    network = get_network_context_by_meshnetid(neighbor->addr.netid);
    if (network && network->router->sid_type == STRUCTURED_SID &&
        is_allocated_child(network->sid_base, neighbor)) {
        free_sid(network->sid_base, neighbor->addr.addr.short_addr);
    }

    slist_del(&neighbor->next, &hal->neighbors_list);
    ur_mem_free(neighbor, sizeof(neighbor_t));
    hal->neighbors_num--;
    return UR_ERROR_NONE;
}

static void handle_update_nbr_timer(void *args)
{
    neighbor_t    *node;
    hal_context_t *hal = (hal_context_t *)args;
    uint16_t sid = umesh_mm_get_local_sid();
    network_context_t *network = NULL;

    hal->update_nbr_timer = NULL;
    slist_for_each_entry(&hal->neighbors_list, node, neighbor_t, next) {
        if (node->state < STATE_NEIGHBOR) {
            continue;
        }

        if (node->attach_candidate_timeout > 0) {
            node->attach_candidate_timeout--;
        }

        if ((ur_get_now() - node->last_heard) < hal->neighbor_alive_interval) {
            continue;
        }

        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
               "%x neighbor " EXT_ADDR_FMT " become inactive\r\n",
               sid, EXT_ADDR_DATA(node->mac.addr));
        network = get_network_context_by_meshnetid(node->addr.netid);
        if (network && network->router->sid_type == STRUCTURED_SID &&
            node->state == STATE_CHILD) {
            free_sid(network->sid_base, node->addr.addr.short_addr);
        }
        node->state = STATE_INVALID;
        g_neighbor_updater_head(node);
    }

    hal->update_nbr_timer = ur_start_timer(hal->advertisement_interval,
                                           handle_update_nbr_timer, hal);
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
    mm_ueid_tv_t      *src_ueid = NULL;
    mm_ssid_info_tv_t *ssid_info = NULL;
    mm_mode_tv_t      *mode;
    mm_channel_tv_t   *channel;
    hal_context_t     *hal;
    network_context_t *network;
    uint8_t channel_orig;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "update neighbor\r\n");

    hal = get_hal_context(info->hal_type);
    nbr = get_neighbor_by_mac_addr(&(info->src_mac.addr));

    if (length == 0) {
        goto exit;
    }

    path_cost = (mm_cost_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_PATH_COST);
    src_ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_SRC_UEID);
    ssid_info = (mm_ssid_info_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_SSID_INFO);
    mode = (mm_mode_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_MODE);
    channel = (mm_channel_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_UCAST_CHANNEL);

    // remove nbr, if mode changed
    if (nbr && mode && nbr->mode != 0 && nbr->mode != mode->mode) {
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

    if (src_ueid != NULL) {
        memcpy(nbr->ueid, src_ueid->ueid, sizeof(src_ueid->ueid));
    }
    if (mode != NULL) {
        nbr->mode = (node_mode_t)mode->mode;
    }

    if (nbr->state < STATE_CANDIDATE) {
        nbr->state = STATE_NEIGHBOR;
        nbr->stats.link_cost = 256;
    }
    if (path_cost != NULL) {
        nbr->path_cost = path_cost->cost;
    }
    if (nbr->addr.addr.short_addr != info->src.addr.short_addr) {
        nbr->flags |= NBR_SID_CHANGED;
    }
    if (nbr->addr.netid != info->src.netid) {
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

        if (nbr->state == STATE_CHILD &&
            ((nbr->flags & NBR_NETID_CHANGED) ||
             (nbr->flags & NBR_SID_CHANGED))) {
            free_sid(network->sid_base, nbr->addr.addr.short_addr);
            nbr->state = STATE_NEIGHBOR;
        }
        network = get_network_context_by_meshnetid(info->src.netid);
        if (network && is_direct_child(network->sid_base, info->src.addr.short_addr) &&
            is_allocated_child(network->sid_base, nbr)) {
            nbr->state = STATE_CHILD;
        }
    }
    nbr->addr.addr.short_addr = info->src.addr.short_addr;
    nbr->addr.netid = info->src.netid;
    g_neighbor_updater_head(nbr);

    if (hal->update_nbr_timer == NULL) {
        hal->update_nbr_timer = ur_start_timer(hal->advertisement_interval,
                                               handle_update_nbr_timer, hal);
    }

exit:
    if (nbr) {
        nbr->rssi = info->rssi;
        nbr->last_heard = ur_get_now();
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

neighbor_t *get_neighbor_by_ueid(const uint8_t *ueid)
{
    neighbor_t *node;
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        slist_for_each_entry(&hal->neighbors_list, node, neighbor_t, next) {
            if ((memcmp(ueid, node->ueid, sizeof(node->ueid)) == 0) &&
                (node->state > STATE_INVALID)) {
                return node;
            }
        }
    }

    return NULL;
}

neighbor_t *get_neighbor_by_mac_addr(const mac_address_t *mac_addr)
{
    neighbor_t *nbr = NULL;
    slist_t *hals;
    hal_context_t *hal;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
            if (mac_addr->len == nbr->mac.len &&
                memcmp(mac_addr->addr, nbr->mac.addr, sizeof(nbr->mac.addr)) == 0 &&
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
        if (node->addr.addr.short_addr == sid && node->addr.netid == meshnetid &&
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
    ur_error_t           error = UR_ERROR_NONE;
    mm_header_t          *mm_header;
    mm_tlv_request_tlv_t *request_tlvs;
    message_t            *message;
    uint8_t              *data;
    uint16_t             length;
    message_info_t *info;

    length = sizeof(mm_header_t);
    if (tlvs_length) {
        length += (tlvs_length + sizeof(mm_tlv_request_tlv_t));
    }
    message = message_alloc(length, LINK_MGMT_1);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_LINK_REQUEST;
    data += sizeof(mm_header_t);

    if (tlvs_length) {
        request_tlvs = (mm_tlv_request_tlv_t *)data;
        umesh_mm_init_tlv_base((mm_tlv_t *)request_tlvs, TYPE_TLV_REQUEST, tlvs_length);
        data += sizeof(mm_tlv_request_tlv_t);
        memcpy(data, tlvs, tlvs_length);
        data += tlvs_length;
    }

    info = message->info;
    info->network = network;
    // dest
    memcpy(&info->dest, dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send link request, len %d\r\n", length);

    return error;
}

static ur_error_t send_link_accept_and_request(network_context_t *network,
                                               ur_addr_t *dest,
                                               uint8_t *tlvs,
                                               uint8_t tlvs_length)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    mm_tlv_request_tlv_t *request_tlvs;
    message_t   *message;
    uint8_t     *data;
    int16_t     length;
    neighbor_t  *node;
    message_info_t *info;
    uint8_t     tlv_types_length = 0;

    node = get_neighbor_by_mac_addr(&dest->addr);
    if (node == NULL) {
        return UR_ERROR_FAIL;
    }

    length = tlvs_calc_length(tlvs, tlvs_length);
    if (length < 0) {
        return UR_ERROR_FAIL;
    }
    length += sizeof(mm_header_t);

    if (memcmp(node->ueid, INVALID_UEID, sizeof(node->ueid))) {
        tlv_types_length += 1;
    }
    if (tlv_types_length) {
        length += (sizeof(mm_tlv_request_tlv_t) + tlv_types_length);
    }

    message = message_alloc(length, LINK_MGMT_2);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_LINK_ACCEPT_AND_REQUEST;
    data += sizeof(mm_header_t);
    data += tlvs_set_value(network, data, tlvs, tlvs_length);

    if (tlv_types_length) {
        request_tlvs = (mm_tlv_request_tlv_t *)data;
        umesh_mm_init_tlv_base((mm_tlv_t *)request_tlvs, TYPE_TLV_REQUEST,
                               tlv_types_length);
        data += sizeof(mm_tlv_request_tlv_t);
        data[0] = TYPE_TARGET_UEID;
        data += tlv_types_length;
    }

    info = message->info;
    info->network = network;
    // dest
    memcpy(&info->dest, dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);
    node->stats.link_request++;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send link accept and request, len %d\r\n", length);
    return error;
}

static ur_error_t send_link_accept(network_context_t *network,
                                   ur_addr_t *dest,
                                   uint8_t *tlvs, uint8_t tlvs_length)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    message_t   *message;
    uint8_t     *data;
    int16_t     length;
    neighbor_t  *node;
    message_info_t *info;

    node = get_neighbor_by_mac_addr(&dest->addr);
    if (node == NULL) {
        return UR_ERROR_FAIL;
    }

    length = tlvs_calc_length(tlvs, tlvs_length);
    if (length < 0) {
        return UR_ERROR_FAIL;
    }
    length += sizeof(mm_header_t);
    message = message_alloc(length, LINK_MGMT_3);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }

    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_LINK_ACCEPT;
    data += sizeof(mm_header_t);
    data += tlvs_set_value(network, data, tlvs, tlvs_length);

    info = message->info;
    info->network = network;
    // dest
    memcpy(&info->dest, dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send link accept, len %d\r\n", length);
    return error;
}

ur_error_t handle_link_request(message_t *message)
{
    mm_tlv_request_tlv_t *tlvs_request;
    uint8_t              *tlvs;
    uint16_t             tlvs_length;
    message_info_t       *info;
    network_context_t    *network;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link request\r\n");

    info = message->info;
    network = info->network;
    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
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
    return UR_ERROR_NONE;
}

ur_error_t handle_link_accept_and_request(message_t *message)
{
    uint8_t    *tlvs;
    uint16_t   tlvs_length;
    mm_tlv_request_tlv_t *tlvs_request;
    mm_ueid_tv_t *ueid;
    mm_channel_tv_t *channel;
    neighbor_t *node;
    message_info_t *info;
    network_context_t *network;
    uint8_t local_channel;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "handle link accept and resquest\r\n");

    info = message->info;
    network = info->network;
    node = get_neighbor_by_mac_addr(&(info->src_mac.addr));
    if (node == NULL) {
        return UR_ERROR_NONE;
    }

    node->stats.link_accept++;

    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);
    if (ueid) {
        memcpy(node->ueid, ueid->ueid, sizeof(node->ueid));
    }

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
    return UR_ERROR_NONE;
}

ur_error_t handle_link_accept(message_t *message)
{
    uint8_t      *tlvs;
    uint16_t     tlvs_length;
    mm_ueid_tv_t *ueid;
    neighbor_t   *node;
    message_info_t *info;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link accept\r\n");

    info = message->info;
    node = get_neighbor_by_mac_addr(&(info->src_mac.addr));
    if (node == NULL) {
        return UR_ERROR_NONE;
    }

    node->stats.link_accept++;

    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    ueid = (mm_ueid_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);
    if (ueid) {
        memcpy(node->ueid, ueid->ueid, sizeof(node->ueid));
    }
    return UR_ERROR_NONE;
}

ur_error_t handle_link_reject(message_t *message)
{
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link reject\r\n");

    return UR_ERROR_NONE;
}

void start_neighbor_updater(void)
{
    slist_t *hals;
    hal_context_t *hal;
    uint32_t interval;

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        if (umesh_mm_get_mode() & MODE_MOBILE) {
            interval = hal->link_request_mobile_interval;
        } else {
            interval = hal->link_request_interval;
        }
        hal->link_request_timer = ur_start_timer(interval, handle_link_request_timer,
                                                 hal);
        hal->link_quality_update_timer = ur_start_timer(interval * LINK_ESTIMATE_TIMES,
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

