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

#include "mesh_types.h"
#include "mesh_mgmt.h"
#include "mesh_forwarder.h"
#include "link_mgmt.h"
#include "logging.h"
#include "timer.h"
#include "memory.h"
#include "network_data.h"
#include "sid_allocator.h"
#include "address_resolver.h"
#include "interfaces.h"
#include "mesh_mgmt_tlvs.h"

static neighbor_updated_t g_neighbor_updater_head;

static void handle_link_request_timer(void *args)
{
    neighbor_t *attach_node;
    hal_context_t *hal = (hal_context_t *)args;
    slist_t *networks;
    network_context_t *network;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link update timer\r\n");

    hal->link_request_timer = ur_start_timer(hal->link_request_interval,
                                             handle_link_request_timer, hal);
    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        if (network->hal != hal)
            continue;

        attach_node = mm_get_attach_node(network);
        if (attach_node == NULL) {
            continue;
        }

        send_link_request(network, &attach_node->addr, NULL, 0);
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
        stats->link_cost = ((uint32_t)((LINK_ESTIMATE_COEF - LINK_ESTIMATE_UPDATE_ALPHA) * old) +
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

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle neighbor update timer\r\n");

    hal->link_quality_update_timer = ur_start_timer(
                                 hal->link_request_interval * LINK_ESTIMATE_TIMES,
                                 handle_link_quality_update_timer, hal);
    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        if (network->hal != hal)
            continue;

        attach_node = mm_get_attach_node(network);
        if (attach_node == NULL) {
            continue;
        }

        update_link_cost(&attach_node->stats);
    }
    g_neighbor_updater_head(hal);
}

static neighbor_t *new_neighbor(hal_context_t *hal, const mac_address_t *addr,
                                uint8_t *tlvs, uint16_t length, bool is_attach)
{
    neighbor_t   *nbr;

    slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
        if (nbr->state == STATE_INVALID)
            goto get_nbr;
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

    if (!is_attach)
        return NULL;

    slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
        if (nbr->state == STATE_PARENT)
            continue;
        if (nbr->state == STATE_CHILD)
            continue;

        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
              "sid %04x ueid " EXT_ADDR_FMT " is replaced\n",
              nbr->sid, EXT_ADDR_DATA(nbr->ueid));
        goto get_nbr;
    }
    return NULL;

get_nbr:
    memset(nbr->ueid, 0xff, sizeof(nbr->ueid));
    nbr->netid              = INVALID_NETID;
    nbr->sub_netid          = INVALID_NETID;
    nbr->sid                = INVALID_SID;
    nbr->addr.len           = sizeof(nbr->addr.addr);
    memcpy(nbr->addr.addr, addr->addr, sizeof(nbr->addr.addr));
    nbr->path_cost          = INFINITY_PATH_COST;
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
    if (neighbor == NULL) {
        return UR_ERROR_NONE;
    }

    slist_del(&neighbor->next, &hal->neighbors_list);
    ur_mem_free(neighbor, sizeof(neighbor_t));
    hal->neighbors_num--;
    return UR_ERROR_NONE;
}

static neighbor_t *find_neighbor(hal_context_t *hal, const mac_address_t *addr)
{
    neighbor_t *nbr = NULL;

    slist_for_each_entry(&hal->neighbors_list, nbr, neighbor_t, next) {
        if ((addr->len == nbr->addr.len) &&
            (memcmp(addr->addr, nbr->addr.addr, sizeof(nbr->addr.addr)) == 0)) {
            return nbr;
        }
    }
    return NULL;
}

static void handle_update_nbr_timer(void *args)
{
    neighbor_t *node;
    hal_context_t *hal = (hal_context_t *)args;
    uint16_t sid = mm_get_local_sid();
    network_context_t *network = NULL;

    hal->update_nbr_timer = NULL;
    slist_for_each_entry(&hal->neighbors_list, node, neighbor_t, next) {
        if (node->state < STATE_NEIGHBOR) {
            continue;
        }

        if (node->attach_candidate_timeout > 0) {
            node->attach_candidate_timeout--;
        }

        if ((ur_get_now() - node->last_heard) < NEIGHBOR_ALIVE_TIMEOUT) {
            continue;
        }

        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
               "%x neighbor " EXT_ADDR_FMT " become inactive\r\n",
               sid, EXT_ADDR_DATA(node->addr.addr));
        if (node->state == STATE_CHILD)
            free_sid(node->ueid, sid, node->sid);
        node->state = STATE_INVALID;
    }

    network = get_default_network_context();
    node = mm_get_attach_node(network);
    if (node && node->state == STATE_INVALID) {
        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
               "it is my parent, restart attach\r\n");
        attach_start(hal, NULL);
    }

    hal->update_nbr_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
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

static void handle_child_status(const mesh_src_t *src,
                                neighbor_t *child)
{
    neighbor_t *node;
    network_context_t *network;
    uint16_t netid;
    uint16_t mysid;
    sid_t    dest;

    network = src->dest_network;
    netid = mm_get_meshnetid(network);
    mysid = mm_get_local_sid();

    if (is_same_mainnet(child->netid, netid) == false) {
        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
               EXT_ADDR_FMT " netid changed from %04x to %04x\r\n",
               EXT_ADDR_DATA(child->ueid), netid, child->netid);
        free_sid(child->ueid, mysid, child->sid);
        child->state = STATE_NEIGHBOR;
        return;
    }

    if (child->sid != src->sid && child->sid != INVALID_SID) {
        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
               EXT_ADDR_FMT " sid changed from %04x to %04x\r\n",
               EXT_ADDR_DATA(child->ueid), child->sid, src->sid);
        free_sid(child->ueid, mysid, child->sid);
    }

    slist_for_each_entry(&network->hal->neighbors_list, node, neighbor_t, next) {
        if (node == child) {
            continue;
        }
        if (node->netid == child->netid && child->sid != INVALID_SID &&
            child->sid == node->sid) {
            dest.meshnetid = node->netid;
            dest.sid = node->sid;
            send_address_error(network, &dest);
            break;
        }
    }
}

neighbor_t *update_neighbor(const mesh_src_t *addr,
                            uint8_t *tlvs, uint16_t length, bool is_attach)
{
    neighbor_t      *nbr = NULL;
    mm_cost_tv_t    *path_cost = NULL;
    mm_ueid_tv_t    *src_ueid = NULL;
    mm_netinfo_tv_t *netinfo;
    mm_mode_tv_t    *mode;
    hal_context_t   *hal;
    uint16_t        subnet_size;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "update neighbor\r\n");

    hal = addr->hal;
    nbr = find_neighbor(hal, &addr->src);
    if (nbr == NULL) {
        nbr = new_neighbor(hal, &addr->src, tlvs, length, is_attach);
    }
    else if (is_attach) {
        /* move attaching neighbor to the head of list */
        slist_del(&nbr->next, &hal->neighbors_list);
        slist_add_tail(&nbr->next, &hal->neighbors_list);
    }
    if (nbr == NULL) {
        return NULL;
    }

    path_cost = (mm_cost_tv_t *)mm_get_tv(tlvs, length, TYPE_PATH_COST);
    src_ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, length, TYPE_SRC_UEID);
    netinfo = (mm_netinfo_tv_t *)mm_get_tv(tlvs, length, TYPE_NETWORK_INFO);
    mode = (mm_mode_tv_t *)mm_get_tv(tlvs, length, TYPE_MODE);

    if (src_ueid != NULL) {
        memcpy(nbr->ueid, src_ueid->ueid, sizeof(src_ueid->ueid));
    }
    if (netinfo != NULL) {
        nbr->netid = netinfo->meshnetid;
        nbr->sub_netid = mk_sub_netid(netinfo->meshnetid, netinfo->sub_meshnetid);
        if (netinfo->meshnetid == addr->meshnetid) {
            subnet_size = get_subnetsize_from_netinfo(netinfo);
            nbr->subnet_size = subnet_size;
        }
        nbr->child_num = netinfo->child_num;
        nbr->free_slots = netinfo->free_slots;
    }
    if (mode != NULL) {
        nbr->mode = (node_mode_t)mode->mode;
    }
    if (nbr->state < STATE_NEIGHBOR) {
        nbr->state = STATE_NEIGHBOR;
        nbr->stats.link_cost = 256;
    }
    if (path_cost != NULL) {
        nbr->path_cost = path_cost->cost;
    }
    if (nbr->sid != addr->sid && nbr->sid != INVALID_SID && addr->sid != INVALID_SID) {
        nbr->flags |= NBR_SID_CHANGED;
    }
    if ((nbr->netid != addr->meshnetid && nbr->sub_netid != addr->meshnetid) &&
         nbr->netid != INVALID_NETID && addr->meshnetid != INVALID_NETID) {
        nbr->flags |= NBR_NETID_CHANGED;
    }
    if (nbr->state == STATE_CHILD) {
        handle_child_status(addr, nbr);
    }
    nbr->sid = addr->sid;
    nbr->last_heard = ur_get_now();
    if (nbr->netid == mm_get_meshnetid(addr->dest_network)) {
        nbr->state = is_direct_child(nbr->sid) ? STATE_CHILD : STATE_NEIGHBOR;
    }
    else {
        nbr->state = STATE_NEIGHBOR;
    }

    if (hal->update_nbr_timer == NULL) {
        hal->update_nbr_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
                                               handle_update_nbr_timer, hal);
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

neighbor_t *get_neighbor_by_mac_addr(hal_context_t *hal, const uint8_t *mac_addr)
{
    neighbor_t    *nbr;
    mac_address_t addr;
    slist_t *hals;

    if (hal == NULL) {
        hals = get_hal_contexts();
        hal = slist_first_entry(hals, hal_context_t, next);
    }
    if (hal == NULL) {
        return NULL;
    }
    addr.len = 8;
    memcpy(addr.addr, mac_addr, sizeof(addr.addr));
    nbr =  find_neighbor(hal, &addr);
    if (nbr && nbr->state == STATE_INVALID) {
        nbr = NULL;
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
        if ((node->sid == sid) &&
            (node->netid == meshnetid || node->sub_netid == meshnetid) &&
            (node->state > STATE_INVALID)) {
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

ur_error_t send_link_request(network_context_t *network,
                             const mac_address_t *dest,
                             uint8_t *tlvs, uint8_t tlvs_length)
{
    ur_error_t           error = UR_ERROR_NONE;
    mm_header_t          *mm_header;
    mm_tlv_request_tlv_t *request_tlvs;
    message_t            *message;
    uint8_t              *data;
    uint16_t             length;
    neighbor_t           *node;
    mesh_dest_t          *mesh_dest;

    node = get_neighbor_by_mac_addr(network->hal, dest->addr);
    if (node == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t);
    if (tlvs_length) {
        length += (tlvs_length + sizeof(mm_tlv_request_tlv_t));
    }
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_LINK_REQUEST;
    data += sizeof(mm_header_t);

    if (tlvs_length) {
        request_tlvs = (mm_tlv_request_tlv_t *)data;
        mm_init_tlv_base((mm_tlv_t *)request_tlvs, TYPE_TLV_REQUEST, tlvs_length);
        data += sizeof(mm_tlv_request_tlv_t);
        memcpy(data, tlvs, tlvs_length);
        data += tlvs_length;
    }

    mesh_dest = message->dest;
    memset(mesh_dest, 0, sizeof(mesh_dest_t));
    mesh_dest->meshnetid = mm_get_meshnetid(network);
    memcpy(&mesh_dest->dest, dest, sizeof(mesh_dest->dest));
    error = mf_send_command(mm_header->command, message);
    node->stats.link_request++;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send link request, len %d\r\n", length);

    return error;
}

ur_error_t send_link_accept_and_request(network_context_t *network,
                                        const mac_address_t *dest,
                                        uint8_t *tlvs, uint8_t tlvs_length)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    mm_tlv_request_tlv_t *request_tlvs;
    message_t   *message;
    uint8_t     *data;
    int16_t     length;
    neighbor_t  *node;
    mesh_dest_t *mesh_dest;
    uint8_t     tlv_types_length = 0;

    node = get_neighbor_by_mac_addr(network->hal, dest->addr);
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

    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_LINK_ACCEPT_AND_REQUEST;
    data += sizeof(mm_header_t);
    data += tlvs_set_value(data, tlvs, tlvs_length);

    if (tlv_types_length) {
        request_tlvs = (mm_tlv_request_tlv_t *)data;
        mm_init_tlv_base((mm_tlv_t *)request_tlvs, TYPE_TLV_REQUEST, tlv_types_length);
        data += sizeof(mm_tlv_request_tlv_t);
        data[0] = TYPE_TARGET_UEID;
        data += tlv_types_length;
    }

    mesh_dest = message->dest;
    memset(mesh_dest, 0, sizeof(mesh_dest_t));
    mesh_dest->meshnetid = mm_get_meshnetid(network);
    memcpy(&mesh_dest->dest, dest, sizeof(mesh_dest->dest));
    error = mf_send_command(mm_header->command, message);
    node->stats.link_request++;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send link accept and request, len %d\r\n", length);
    return error;
}

ur_error_t send_link_accept(network_context_t *network,
                            const mac_address_t *dest,
                            uint8_t *tlvs, uint8_t tlvs_length)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    message_t   *message;
    uint8_t     *data;
    uint16_t    length;
    neighbor_t  *node;
    mesh_dest_t *mesh_dest;

    node = get_neighbor_by_mac_addr(network->hal, dest->addr);
    if (node == NULL) {
        return UR_ERROR_FAIL;
    }

    length = tlvs_calc_length(tlvs, tlvs_length);
    if (length < 0) {
        return UR_ERROR_FAIL;
    }
    length += sizeof(mm_header_t);
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }

    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_LINK_ACCEPT;
    data += sizeof(mm_header_t);
    data += tlvs_set_value(data, tlvs, tlvs_length);

    mesh_dest = message->dest;
    memset(mesh_dest, 0, sizeof(mesh_dest_t));
    mesh_dest->meshnetid = mm_get_meshnetid(network);
    memcpy(&mesh_dest->dest, dest, sizeof(mesh_dest->dest));
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send link accept, len %d\r\n", length);
    return error;
}

ur_error_t handle_link_request(uint8_t *payload, uint16_t length,
                               const mesh_src_t *src,
                               const mac_address_t *dest)
{
    mm_tlv_request_tlv_t *tlvs_request;
    uint8_t              *tlvs;
    uint16_t             tlvs_length;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link request\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);
    tlvs_request = (mm_tlv_request_tlv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TLV_REQUEST);

    if (tlvs_request) {
        tlvs = (uint8_t *)tlvs_request + sizeof(mm_tlv_t);
        tlvs_length = tlvs_request->base.length;
    } else {
        tlvs = NULL;
        tlvs_length = 0;
    }
    send_link_accept_and_request(src->dest_network, &src->src, tlvs, tlvs_length);

    return UR_ERROR_NONE;
}

ur_error_t handle_link_accept_and_request(uint8_t *payload, uint16_t length,
                                          const mesh_src_t *src,
                                          const mac_address_t *dest)
{
    uint8_t    *tlvs;
    uint16_t   tlvs_length;
    mm_tlv_request_tlv_t *tlvs_request;
    mm_ueid_tv_t *ueid;
    neighbor_t *node;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link accept and resquest\r\n");

    node = get_neighbor_by_mac_addr(src->dest_network->hal, src->src.addr);
    if (node == NULL) {
        return UR_ERROR_NONE;
    }

    node->stats.link_accept++;

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);
    if (ueid) {
        memcpy(node->ueid, ueid->ueid, sizeof(node->ueid));
    }

    tlvs_request = (mm_tlv_request_tlv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TLV_REQUEST);
    if (tlvs_request) {
        tlvs = (uint8_t *)tlvs_request + sizeof(mm_tlv_t);
        tlvs_length = tlvs_request->base.length;
    } else {
        tlvs = NULL;
        tlvs_length = 0;
    }
    send_link_accept(src->dest_network, &src->src, tlvs, tlvs_length);

    return UR_ERROR_NONE;
}

ur_error_t handle_link_accept(uint8_t *payload, uint16_t length,
                              const mesh_src_t *src,
                              const mac_address_t *dest)
{
    uint8_t      *tlvs;
    uint16_t     tlvs_length;
    mm_ueid_tv_t *ueid;
    neighbor_t   *node;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link accept\r\n");

    node = get_neighbor_by_mac_addr(src->dest_network->hal, src->src.addr);
    if (node == NULL) {
        return UR_ERROR_NONE;
    }

    node->stats.link_accept++;

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TARGET_UEID);
    if (ueid) {
        memcpy(node->ueid, ueid->ueid, sizeof(node->ueid));
    }

    return UR_ERROR_NONE;
}

ur_error_t handle_link_reject(uint8_t *payload, uint16_t length,
                              const mesh_src_t *src,
                              const mac_address_t *dest)
{
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle link reject\r\n");

    return UR_ERROR_NONE;
}

void start_neighbor_updater(void)
{
    slist_t *hals;
    hal_context_t *hal;
    uint32_t interval;

    if (mm_get_mode() & MODE_MOBILE) {
        interval = LINK_REQUEST_MOBILE_TIMEOUT;
    } else {
        interval = LINK_REQUEST_TIMEOUT;
    }

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        hal->link_request_interval = interval;
        hal->link_request_timer = ur_start_timer(interval, handle_link_request_timer, hal);
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

