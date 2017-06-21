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
#include "core/network_mgmt.h"
#include "core/topology.h"
#include "core/network_data.h"
#include "core/link_mgmt.h"
#include "core/master_key.h"
#include "core/keys_mgr.h"
#include "utilities/logging.h"
#include "utilities/message.h"
#include "utilities/timer.h"
#include "utilities/memory.h"
#include "hal/interfaces.h"

static void handle_discovery_timer(void *args);
static ur_error_t send_discovery_request(network_context_t *netowrk);
static ur_error_t send_discovery_response(network_context_t *network,
                                          ur_addr_t *dest);

static void handle_discovery_timer(void *args)
{
    uint32_t discovery_interval;
    network_context_t *network = (network_context_t *)args;
    hal_context_t *hal = network->hal;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle discovery timer\r\n");

    hal->discovery_timer = NULL;
    if (hal->discovery_channel >= hal->channel_list.num) {
        hal->discovery_channel = 0;
        if (hal->discovery_result.meshnetid != BCAST_SID) {
            hal->discovery_times = DISCOVERY_RETRY_TIMES;
        }
        hal->discovery_times++;
    }

    discovery_interval = hal->discovery_interval;
    if (hal->discovery_times > 0) {
        discovery_interval *= 2;
    }

    if (hal->discovery_times < DISCOVERY_RETRY_TIMES) {
        umesh_mm_set_channel(network,
                             hal->channel_list.channels[hal->discovery_channel]);
        send_discovery_request(network);
        hal->discovery_timer = ur_start_timer(discovery_interval,
                                              handle_discovery_timer, network);
        hal->discovery_channel++;
        return;
    } else if (hal->discovery_result.meshnetid != BCAST_NETID) {
        umesh_mm_set_channel(network, hal->discovery_result.channel);
        if (umesh_mm_get_mode() != MODE_NONE) {
            master_key_request_start();
        }
        return;
    }

    if ((umesh_mm_get_mode() & MODE_MOBILE) == 0) {
        become_leader();
    } else {
        umesh_mm_set_channel(network, hal->channel_list.channels[0]);
    }
}

static ur_error_t send_discovery_request(network_context_t *network)
{
    ur_error_t      error = UR_ERROR_NONE;
    uint16_t        length;
    mm_header_t     *mm_header;
    mm_version_tv_t *version;
    mm_mode_tv_t    *mode;
    uint8_t         *data;
    message_t       *message = NULL;
    message_info_t  *info;

    length = sizeof(mm_header_t) + sizeof(mm_version_tv_t) + sizeof(mm_mode_tv_t);
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_DISCOVERY_REQUEST;
    data += sizeof(mm_header_t);

    version = (mm_version_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)version, TYPE_VERSION);
    version->version = 1;
    data += sizeof(mm_version_tv_t);

    mode = (mm_mode_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)mode, TYPE_MODE);
    mode->mode = (uint8_t)umesh_mm_get_mode();
    data += sizeof(mm_mode_tv_t);

    info = message->info;
    info->network = network;
    // dest
    info->dest.addr.len = SHORT_ADDR_SIZE;
    info->dest.addr.short_addr = BCAST_SID;
    info->dest.netid = BCAST_NETID;

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send discovery request in channel %d, len %d\r\n",
           umesh_mm_get_channel(network), length);

    return error;
}

static ur_error_t send_discovery_response(network_context_t *network,
                                          ur_addr_t *dest)
{
    ur_error_t      error = UR_ERROR_NONE;
    mm_header_t     *mm_header;
    mm_netinfo_tv_t *netinfo;
    message_t       *message;
    uint8_t         *data;
    uint16_t        length;
    message_info_t  *info;

    length = sizeof(mm_header_t) + sizeof(mm_netinfo_tv_t);
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_DISCOVERY_RESPONSE;
    data += sizeof(mm_header_t);

    netinfo = (mm_netinfo_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)netinfo, TYPE_NETWORK_INFO);
    netinfo->stable_version = (nd_get_stable_main_version() <<
                               STABLE_MAIN_VERSION_OFFSET) |
                              nd_get_stable_minor_version();
    netinfo->size = umesh_mm_get_meshnetsize();
    netinfo->leader_mode = umesh_mm_get_leader_mode();
    data += sizeof(mm_netinfo_tv_t);

    info = message->info;
    info->network = network;
    // dest
    memcpy(&info->dest, dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send discovery response, len %d\r\n", length);

    return error;
}

ur_error_t handle_discovery_request(message_t *message)
{
    ur_error_t        error = UR_ERROR_NONE;
    mm_version_tv_t   *version;
    mm_mode_tv_t      *mode;
    uint8_t           *tlvs;
    uint16_t          tlvs_length;
    neighbor_t        *nbr;
    network_context_t *network;
    message_info_t    *info;

    if (umesh_mm_get_device_state() < DEVICE_STATE_LEADER) {
        return UR_ERROR_FAIL;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle discovery request\r\n");

    info = message->info;
    network = info->network;
    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    if ((version = (mm_version_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                      TYPE_VERSION)) == NULL) {
        return UR_ERROR_FAIL;
    }

    if (version->version != 1) {
        return UR_ERROR_FAIL;
    }

    if ((mode = (mm_mode_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                TYPE_MODE)) == NULL) {
        return UR_ERROR_FAIL;
    }

    if ((nbr = update_neighbor(info, tlvs, tlvs_length, true)) == NULL) {
        return UR_ERROR_FAIL;
    }

    if (((umesh_mm_get_mode() & MODE_SUPER) && (mode->mode & MODE_SUPER)) ||
        ((umesh_mm_get_mode() & MODE_SUPER) == 0 && (mode->mode & MODE_SUPER) == 0)) {
        network = get_default_network_context();
    } else if ((umesh_mm_get_mode() & MODE_SUPER) &&
               (mode->mode & MODE_SUPER) == 0) {
        network = get_sub_network_context(network->hal);
    }

    nbr->flags |= NBR_DISCOVERY_REQUEST;
    send_discovery_response(network, &info->src_mac);
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
    int8_t            cmp_mode = 0;

    if (umesh_mm_get_device_state() != DEVICE_STATE_DETACHED) {
        return UR_ERROR_NONE;
    }

    info = message->info;
    network = info->network;
    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    if ((nbr = update_neighbor(info, tlvs, tlvs_length, true)) != NULL) {
        nbr->flags &= (~NBR_DISCOVERY_REQUEST);
    }

    netinfo = (mm_netinfo_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length, TYPE_NETWORK_INFO);
    if (netinfo == NULL) {
        return UR_ERROR_FAIL;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "handle discovery response from %x\r\n",
           info->src.netid);

    if (is_bcast_netid(info->src.netid)) {
        return UR_ERROR_NONE;
    }

    cmp_mode = umesh_mm_compare_mode(umesh_mm_get_mode(), netinfo->leader_mode);
    if (cmp_mode > 0) {
        return UR_ERROR_NONE;
    }

    res = &network->hal->discovery_result;
    if (is_bcast_netid(res->meshnetid) ||
        res->meshnetid < get_main_netid(info->src.netid)) {
        memcpy(&res->addr, &info->src_mac.addr, sizeof(res->addr));
        res->channel = info->channel;
        res->meshnetid = info->src.netid;
    }

    return UR_ERROR_NONE;
}

ur_error_t nm_start_discovery(void)
{
    network_context_t *network;
    hal_context_t     *hal;

    network = get_default_network_context();
    hal = network->hal;
    hal->discovery_channel = 0;
    hal->discovery_times = 0;
    hal->discovery_timer = ur_start_timer(hal->discovery_interval,
                                          handle_discovery_timer, network);
    memset(&hal->discovery_result, 0, sizeof(hal->discovery_result));
    hal->discovery_result.meshnetid = BCAST_NETID;
    return UR_ERROR_NONE;
}

ur_error_t nm_stop_discovery(void)
{
    network_context_t *network;
    hal_context_t     *hal;

    network = get_default_network_context();
    hal = network->hal;
    ur_stop_timer(&hal->discovery_timer, network);

    return UR_ERROR_NONE;
}
