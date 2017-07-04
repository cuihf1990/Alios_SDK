/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>

#include "core/mesh_mgmt.h"
#include "core/mesh_forwarder.h"
#include "core/network_mgmt.h"
#include "core/topology.h"
#include "core/network_data.h"
#include "core/link_mgmt.h"
#include "core/keys_mgr.h"
#include "hal/interfaces.h"
#include "umesh_utils.h"

static void handle_discovery_timer(void *args);
static ur_error_t send_discovery_request(network_context_t *netowrk);
static ur_error_t send_discovery_response(network_context_t *network,
                                          ur_addr_t *dest);

static void handle_discovery_timer(void *args)
{
    network_context_t *network = (network_context_t *)args;
    hal_context_t *hal = network->hal;
    neighbor_t *nbr;
    bool migrate = false;

    MESH_LOG_DEBUG("handle discovery timer");

    hal->discovery_timer = NULL;
    if (hal->discovery_channel >= hal->channel_list.num) {
        hal->discovery_channel = 0;
        hal->discovery_times++;
    }

    if (hal->discovery_result.meshnetid != BCAST_NETID) {
        mm_netinfo_tv_t netinfo;
        nbr = get_neighbor_by_mac_addr(hal->discovery_result.addr.addr);
        netinfo.leader_mode = hal->discovery_result.leader_mode;
        netinfo.size = hal->discovery_result.net_size;
        if (nbr && umesh_mm_migration_check(network, nbr, &netinfo)) {
            migrate = true;
        }
    }
    if (hal->discovery_times > 0 && migrate) {
        umesh_mm_set_channel(network, hal->discovery_result.channel);
        nbr = get_neighbor_by_mac_addr(hal->discovery_result.addr.addr);
        hal->discovered_handler(nbr);
        return;
    } else if (hal->discovery_times < DISCOVERY_RETRY_TIMES) {
        if (umesh_mm_get_prev_channel() == hal->discovery_channel) {
            hal->discovery_channel++;
        }
        umesh_mm_set_channel(network,
                             hal->channel_list.channels[hal->discovery_channel]);
        send_discovery_request(network);
        hal->discovery_timer = ur_start_timer(hal->discovery_interval,
                                              handle_discovery_timer, network);
        hal->discovery_channel++;
        return;
    } else if (umesh_mm_get_device_state() >= DEVICE_STATE_LEAF) {
        umesh_mm_set_channel(network, umesh_mm_get_prev_channel());
    } else {
        umesh_mm_set_channel(network, hal->def_channel);
        if ((umesh_mm_get_mode() & MODE_MOBILE) == 0) {
            become_leader();
        }
    }
    if (umesh_mm_get_device_state() == DEVICE_STATE_LEADER &&
        (umesh_mm_get_mode() & MODE_LEADER) == 0) {
        umesh_mm_start_net_scan_timer();
    }
}

static ur_error_t send_discovery_request(network_context_t *network)
{
    ur_error_t error = UR_ERROR_MEM;
    uint16_t        length;
    mm_state_flags_tv_t *flag;
    uint8_t         *data;
    uint8_t *data_orig;
    message_t       *message = NULL;
    message_info_t  *info;

    length = sizeof(mm_header_t) + sizeof(mm_state_flags_tv_t);
    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);

    flag = (mm_state_flags_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)flag, TYPE_STATE_FLAGS);
    flag->flags = umesh_mm_get_reboot_flag();
    data += sizeof(mm_state_flags_tv_t);

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_DISCOVERY_REQUEST,
                               data_orig, length, NETWORK_MGMT_1);
    if (message) {
        info = message->info;
        info->network = network;
        set_mesh_short_addr(&info->dest, BCAST_NETID, BCAST_SID);
        error = mf_send_message(message);
    }
    ur_mem_free(data_orig, length);

    MESH_LOG_DEBUG("send discovery request in channel %d, len %d",
                   umesh_mm_get_channel(network), length);

    return error;
}

static ur_error_t send_discovery_response(network_context_t *network,
                                          ur_addr_t *dest)
{
    ur_error_t error = UR_ERROR_MEM;
    message_t       *message;
    uint8_t         *data;
    uint16_t        length;
    message_info_t  *info;
    uint8_t *data_orig;

    length = sizeof(mm_header_t) + sizeof(mm_netinfo_tv_t) + sizeof(mm_channel_tv_t);
    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);
    data += set_mm_netinfo_tv(network, data);
    data += set_mm_channel_tv(network, data);

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_DISCOVERY_RESPONSE,
                               data_orig, length, NETWORK_MGMT_2);
    if (message) {
        info = message->info;
        info->network = network;
        memcpy(&info->dest, dest, sizeof(info->dest));
        error = mf_send_message(message);
    }
    ur_mem_free(data_orig, length);

    MESH_LOG_DEBUG("send discovery response, len %d", length);
    return error;
}

ur_error_t handle_discovery_request(message_t *message)
{
    ur_error_t        error = UR_ERROR_NONE;
    mm_state_flags_tv_t *flag;
    uint8_t           *tlvs;
    uint16_t          tlvs_length;
    neighbor_t        *nbr;
    network_context_t *network;
    message_info_t    *info;

    if (umesh_mm_get_device_state() < DEVICE_STATE_LEADER) {
        return UR_ERROR_FAIL;
    }

    MESH_LOG_DEBUG("handle discovery request");

    info = message->info;
    network = info->network;
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    tlvs = ur_mem_alloc(tlvs_length);
    if (tlvs == NULL) {
        return UR_ERROR_MEM;
    }
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);

    if ((nbr = update_neighbor(info, tlvs, tlvs_length, true)) == NULL) {
        ur_mem_free(tlvs, tlvs_length);
        return UR_ERROR_FAIL;
    }

    flag = (mm_state_flags_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_STATE_FLAGS);
    if (flag && flag->flags) {
        nbr->flags |= NBR_REBOOT;
    } else {
        nbr->flags |= NBR_DISCOVERY_REQUEST;
    }

    send_discovery_response(network, &info->src_mac);
    ur_mem_free(tlvs, tlvs_length);
    return error;
}

ur_error_t handle_discovery_response(message_t *message)
{
    uint8_t           *tlvs;
    uint16_t          tlvs_length;
    neighbor_t        *nbr;
    scan_result_t     *res;
    network_context_t *network;
    message_info_t    *info;
    mm_netinfo_tv_t   *netinfo;
    mm_channel_tv_t   *channel;

    if (umesh_mm_get_device_state() != DEVICE_STATE_DETACHED) {
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

    nbr = update_neighbor(info, tlvs, tlvs_length, true);
    if (nbr == NULL) {
        ur_mem_free(tlvs, tlvs_length);
        return UR_ERROR_FAIL;
    }
    nbr->flags &= (~NBR_DISCOVERY_REQUEST);

    netinfo = (mm_netinfo_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_NETWORK_INFO);
    if (netinfo == NULL) {
        ur_mem_free(tlvs, tlvs_length);
        return UR_ERROR_FAIL;
    }

    MESH_LOG_DEBUG("handle discovery response from %x",
                   info->src.netid);

    if (is_bcast_netid(info->src.netid)) {
        ur_mem_free(tlvs, tlvs_length);
        return UR_ERROR_NONE;
    }

    channel = (mm_channel_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                 TYPE_UCAST_CHANNEL);
    if (channel) {
        info->src_channel = channel->channel;
    }

    res = &network->hal->discovery_result;
    if ((is_bcast_netid(res->meshnetid) ||
         res->meshnetid < get_main_netid(info->src.netid)) &&
        is_same_mainnet(network->meshnetid, info->src.netid) == false) {
        memcpy(&res->addr, &info->src_mac.addr, sizeof(res->addr));
        res->channel = info->src_channel;
        res->meshnetid = info->src.netid;
        res->leader_mode = netinfo->leader_mode;
        res->net_size = netinfo->size;
    }

    ur_mem_free(tlvs, tlvs_length);
    return UR_ERROR_NONE;
}

ur_error_t nm_start_discovery(discovered_handler_t handler)
{
    network_context_t *network;
    hal_context_t     *hal;

    network = get_default_network_context();
    hal = network->hal;
    hal->discovery_channel = 0;
    hal->discovery_times = 0;
    if (hal->discovery_timer) {
        ur_stop_timer(&hal->discovery_timer, network);
    }
    hal->discovery_timer = ur_start_timer(hal->discovery_interval,
                                          handle_discovery_timer, network);
    memset(&hal->discovery_result, 0, sizeof(hal->discovery_result));
    hal->discovery_result.meshnetid = BCAST_NETID;
    hal->discovered_handler = handler;

    umesh_mm_set_prev_channel();
    return UR_ERROR_NONE;
}

ur_error_t nm_stop_discovery(void)
{
    network_context_t *network;
    hal_context_t     *hal;

    network = get_default_network_context();
    hal = network->hal;
    if (hal->discovery_timer) {
        ur_stop_timer(&hal->discovery_timer, network);
    }
    return UR_ERROR_NONE;
}
