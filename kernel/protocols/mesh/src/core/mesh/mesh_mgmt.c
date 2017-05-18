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
#include <stdlib.h>

#include "umesh.h"
#include "umesh_hal.h"
#include "core/mesh_forwarder.h"
#include "core/mesh_mgmt.h"
#include "core/mesh_mgmt_tlvs.h"
#include "core/sid_allocator.h"
#include "core/router_mgr.h"
#include "core/network_data.h"
#include "core/keys_mgr.h"
#include "core/address_mgmt.h"
#include "core/link_mgmt.h"
#include "core/network_mgmt.h"
#include "utilities/logging.h"
#include "utilities/timer.h"
#include "utilities/memory.h"
#include "hal/interfaces.h"
#include "hal/hals.h"

typedef struct mm_device_s {
    mm_device_state_t state;
    node_mode_t       mode;
    uint8_t           ueid[8];
    bool              reboot_flag;
    ur_timer_t        alive_timer;
    uint8_t           seclevel;
} mm_device_t;

typedef struct mesh_mgmt_state_s {
    mm_device_t             device;
    mm_cb_t                 *callback;
    nd_updater_t            network_data_updater;
    umesh_raw_data_received raw_data_receiver;
} mesh_mgmt_state_t;

static mesh_mgmt_state_t g_mm_state;

static void handle_attach_timer(void *args);
static void handle_advertisement_timer(void *args);
static void handle_migrate_wait_timer(void *args);

static ur_error_t send_attach_request(network_context_t *network,
                                      ur_addr_t *dest);
static ur_error_t send_attach_response(network_context_t *network,
                                       ur_addr_t *dest);
static ur_error_t send_sid_request(network_context_t *network);
static ur_error_t send_sid_response(network_context_t *network,
                                    ur_addr_t *dest, ur_addr_t *dest2,
                                    ur_node_id_t *node_id);
static ur_error_t send_advertisement(network_context_t *network);

static void write_prev_netinfo(void);
static void read_prev_netinfo(void);

static void network_data_update_handler(bool stable)
{
}

static void neighbor_updated_handler(hal_context_t *hal)
{
    slist_t *networks;
    network_context_t *network;
    bool need_attach = false;

    if (hal == NULL) {
        return;
    }

    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        if (network->hal != hal || network->state != INTERFACE_UP) {
            continue;
        }
        if (network->attach_node &&
            network->attach_node->stats.link_cost >= LINK_COST_MAX) {
            network->attach_node->state = STATE_INVALID;
            need_attach = true;
        }
    }

    if (need_attach) {
        attach_start(hal, NULL);
    }
}

static void set_default_network_data(void)
{
    network_data_t network_data;
    stable_network_data_t stable_network_data;

    nd_init();
    memset(&network_data, 0, sizeof(network_data));
    memset(&stable_network_data, 0, sizeof(stable_network_data));
    network_data.version = 1;
    network_data.size = 1;
    stable_network_data.minor_version = 1;
    stable_network_data.meshnetid = nd_get_stable_meshnetid();
    stable_network_data.mcast_addr[0].m8[0] = 0xff;
    stable_network_data.mcast_addr[0].m8[1] = 0x08;
    stable_network_data.mcast_addr[0].m8[6] = (uint8_t)(stable_network_data.meshnetid >> 8);
    stable_network_data.mcast_addr[0].m8[7] = (uint8_t)stable_network_data.meshnetid;
    stable_network_data.mcast_addr[0].m8[15] = 0xfc;
    nd_set(NULL, &network_data);
    nd_stable_set(&stable_network_data);
}

static uint16_t generate_meshnetid(uint8_t sid, uint8_t index)
{
    uint16_t meshnetid;
    uint8_t sub_meshnetid;

    sub_meshnetid = ((sid << 2) | index);
    meshnetid = nd_get_stable_meshnetid() | sub_meshnetid;
    if (g_mm_state.device.state == DEVICE_STATE_LEADER &&
        index == 0) {
        nd_set_stable_meshnetid(meshnetid);
    }
    return meshnetid;
}

static void handle_device_alive_timer(void *args)
{
    network_context_t *network;

    g_mm_state.device.alive_timer = NULL;
    network = get_default_network_context();
    send_address_notification(network, NULL);

    g_mm_state.device.alive_timer = ur_start_timer(network->notification_interval, handle_device_alive_timer, NULL);
}

static void start_keep_alive_timer(network_context_t *network)
{
    hal_context_t *hal;

    if (g_mm_state.device.state == DEVICE_STATE_LEADER) {
        return;
    }

    hal = get_default_hal_context();
    if (network->hal == hal && g_mm_state.device.alive_timer == NULL) {
        g_mm_state.device.alive_timer = ur_start_timer(network->notification_interval,
                                                       handle_device_alive_timer, NULL);
    }
}

static void start_advertisement_timer(network_context_t *network)
{
    send_advertisement(network);
    if (network->advertisement_timer == NULL) {
        network->advertisement_timer = ur_start_timer(network->hal->advertisement_interval,
                                                      handle_advertisement_timer, network);
    }
}

static void sid_allocator_init(network_context_t *network)
{
    if (network->router->sid_type == STRUCTURED_SID) {
        allocator_init(network);
    } else {
        rsid_allocator_init(network);
    }
}

static void sid_allocator_deinit(network_context_t *network)
{
    if (network->router->sid_type == STRUCTURED_SID) {
        allocator_deinit(network);
    } else {
        rsid_allocator_deinit(network);
    }
}

void set_command_type(message_info_t *info, uint8_t command)
{
    info->type = MESH_FRAME_TYPE_CMD;
    info->command = command;
}

void get_leader_addr(ur_addr_t *addr)
{
    network_context_t *network;

    network = get_default_network_context();
    addr->addr.len = SHORT_ADDR_SIZE;
    addr->addr.short_addr = LEADER_SID;
    addr->netid = mm_get_main_netid(network);
}

void become_leader(void)
{
    ur_configs_t      configs;
    slist_t           *networks;
    network_context_t *network;
    uint8_t           index = 0;

    g_mm_state.device.state = DEVICE_STATE_LEADER;
    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        hal_context_t *hal = network->hal;
        mm_set_channel(network, hal->channel_list.channels[0]);

        network->state = INTERFACE_UP;
        network->attach_state = ATTACH_DONE;
        network->attach_candidate = NULL;
        network->candidate_meshnetid = BCAST_NETID;
        network->sid = LEADER_SID;
        network->meshnetid = generate_meshnetid(network->sid, index);
        ++index;
        network->path_cost = 0;
        network->change_sid = false;
        if (network->attach_node) {
            network->attach_node->state = STATE_NEIGHBOR;
            network->attach_node = NULL;
        }
        ur_stop_timer(&network->attach_timer, network);

        ur_router_start(network);
        ur_router_sid_updated(network, LEADER_SID);
        sid_allocator_init(network);
        start_advertisement_timer(network);
    }

    set_default_network_data();
    memset(&configs, 0, sizeof(configs));
    ur_configs_read(&configs);
    if (g_mm_state.device.reboot_flag == true) {
        g_mm_state.device.reboot_flag = false;
        configs.main_version++;
        configs.main_version %= 7;
    }
    nd_set_stable_main_version(configs.main_version);
    g_mm_state.callback->interface_up();
    stop_addr_cache();
    start_addr_cache();
    set_master_key(NULL, 0);
    set_group_key(NULL, 0);

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
           "become leader\r\n");
    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM, "interface %d, type %d, netid %x\r\n",
               network->index, network->hal->module->type, network->meshnetid);
    }
}

void mm_init_tlv_base(mm_tlv_t *tlv, uint8_t type, uint8_t length)
{
    tlv->type = type;
    tlv->length = length;
}

void mm_init_tv_base(mm_tv_t *tv, uint8_t type)
{
    tv->type = type;
}

static uint8_t get_tv_value_length(uint8_t type)
{
    uint8_t length;

    switch (type) {
        case TYPE_VERSION:
        case TYPE_MODE:
        case TYPE_NODE_TYPE:
        case TYPE_SCAN_MASK:
        case TYPE_FORWARD_RSSI:
        case TYPE_REVERSE_RSSI:
        case TYPE_SID_TYPE:
        case TYPE_ADDR_QUERY:
        case TYPE_DEF_HAL_TYPE:
            length = 1;
            break;
        case TYPE_SRC_SID:
        case TYPE_DEST_SID:
        case TYPE_ATTACH_NODE_SID:
        case TYPE_ALLOCATE_SID:
        case TYPE_TARGET_SID:
        case TYPE_NETWORK_SIZE:
        case TYPE_PATH_COST:
        case TYPE_LINK_COST:
        case TYPE_WEIGHT:
            length = 2;
            break;
        case TYPE_SSID_INFO:
            length = 3;
            break;
        case TYPE_TIMESTAMP:
            length = 4;
            break;
        case TYPE_NODE_ID:
        case TYPE_ATTACH_NODE_ID:
            length = sizeof(mm_node_id_tv_t) - 1;
            break;
        case TYPE_NETWORK_INFO:
            length = sizeof(mm_netinfo_tv_t) - 1;
            break;
        case TYPE_SRC_UEID:
        case TYPE_DEST_UEID:
        case TYPE_ATTACH_NODE_UEID:
        case TYPE_SRC_MAC_ADDR:
        case TYPE_TARGET_UEID:
            length = 8;
            break;
        case TYPE_MCAST_ADDR:
        case TYPE_GROUP_KEY:
            length = 16;
            break;
        default:
            length = 0;
            break;
    }
    return length;
}

static uint8_t get_tlv_value_length(uint8_t type)
{
    /* this should not happen so far */
    assert(0);
    return 0;
}

static uint8_t get_tv_value(uint8_t *data, uint8_t type)
{
    uint8_t  length = 1;

    *data = type;
    data++;
    switch (type) {
        case TYPE_VERSION:
            *data = (nd_get_stable_main_version() << STABLE_MAIN_VERSION_OFFSET) |
                    nd_get_stable_minor_version();
            length += 1;
            break;
        case TYPE_MCAST_ADDR:
            memcpy(data, nd_get_subscribed_mcast(), 16);
            length += 16;
            break;
        case TYPE_TARGET_UEID:
            memcpy(data, mm_get_local_ueid(), 8);
            length += 8;
            break;
        default:
            assert(0);
            break;
    }
    return length;
}

static uint8_t get_tlv_value(uint8_t *data, uint8_t type)
{
    /* this should not happen so far */
    assert(0);
    return 0;
}

uint16_t tlvs_set_value(uint8_t *buf, const uint8_t *tlvs, uint8_t tlvs_length)
{
    uint8_t index;
    uint8_t *base = buf;

    for (index = 0; index < tlvs_length; index++) {
        if (tlvs[index] & TYPE_LENGTH_FIXED_FLAG) {
            buf += get_tv_value(buf, tlvs[index]);
        } else {
            buf += get_tlv_value(buf, tlvs[index]);
        }
    }

    return (buf - base);
}

int16_t tlvs_calc_length(const uint8_t *tlvs, uint8_t tlvs_length)
{
    int16_t length = 0;
    uint8_t index;

    for (index = 0; index < tlvs_length; index++) {
        if (tlvs[index] & TYPE_LENGTH_FIXED_FLAG) {
            if (get_tv_value_length(tlvs[index]) != 0) {
                length += (get_tv_value_length(tlvs[index]) + sizeof(mm_tv_t));
            } else {
                return -1;
            }
        } else {
            length += (get_tlv_value_length(tlvs[index]) + sizeof(mm_tlv_t));
        }
    }

    return length;
}

mm_tv_t *mm_get_tv(const uint8_t *data, const uint16_t length, uint8_t type)
{
    uint16_t offset = 0;
    mm_tv_t  *tv = NULL;

    while (offset < length) {
        tv = (mm_tv_t *)(data + offset);
        if (tv->type == type) {
            break;
        }

        if (tv->type & TYPE_LENGTH_FIXED_FLAG) {
            if (get_tv_value_length(tv->type) != 0) {
                offset += sizeof(mm_tv_t) + get_tv_value_length(tv->type);
            } else {
                return NULL;
            }
        } else {
            offset += sizeof(mm_tlv_t) + ((mm_tlv_t *)tv)->length;
        }
    }

    if (offset >= length) {
        tv = NULL;
    }

    return tv;
}

static void handle_attach_timer(void *args)
{
    bool detached = false;
    network_context_t *network = (network_context_t *)args;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle attach timer\r\n");

    network->attach_timer = NULL;
    switch (network->attach_state) {
        case ATTACH_REQUEST:
            if (network->retry_times < ATTACH_REQUEST_RETRY_TIMES) {
                ++network->retry_times;
                if (network->attach_candidate) {
                    send_attach_request(network, &network->attach_candidate->addr);
                } else {
                    send_attach_request(network, NULL);
                }
                network->attach_timer = ur_start_timer(network->hal->attach_request_interval,
                                                       handle_attach_timer, args);
            } else {
                detached = true;
                network->leader_times = BECOME_LEADER_TIMEOUT;
            }
            break;
        case ATTACH_SID_REQUEST:
            if (network->retry_times < ATTACH_SID_RETRY_TIMES) {
                ++network->retry_times;
                send_sid_request(network);
                network->attach_timer = ur_start_timer(network->hal->sid_request_interval,
                                                       handle_attach_timer, args);
            } else {
                detached = true;
            }
            break;
        case ATTACH_DONE:
            break;
        case ATTACH_IDLE:
            break;
        default:
            assert(0);
            break;
    }

    if (detached) {
        network->attach_state = ATTACH_IDLE;
        network->candidate_meshnetid = BCAST_NETID;
        network->attach_candidate = NULL;
        if (g_mm_state.device.state < DEVICE_STATE_LEAF || network->change_sid == true) {
            network->leader_times++;
            if (network->leader_times < BECOME_LEADER_TIMEOUT) {
                attach_start(network->hal, NULL);
            } else {
                network->leader_times = 0;
                if ((g_mm_state.device.mode & MODE_SUPER || g_mm_state.device.mode & MODE_RX_ON) &&
                    ((g_mm_state.device.mode & MODE_MOBILE) == 0)) {
                    become_leader();
                } else {
                    become_detached();
                }
            }
            return;
        }
        if (g_mm_state.device.state > DEVICE_STATE_ATTACHED) {
            start_advertisement_timer(network);
        }
    }
}

static void handle_advertisement_timer(void *args)
{
    network_context_t *network = (network_context_t *)args;

    network->advertisement_timer = NULL;
    if (send_advertisement(network) == UR_ERROR_FAIL) {
        return;
    }
    network->advertisement_timer = ur_start_timer(network->hal->advertisement_interval,
                                                  handle_advertisement_timer, args);
}

static void handle_migrate_wait_timer(void *args)
{
    network_context_t *network = (network_context_t *)args;

    network->migrate_wait_timer = NULL;
    network->migrate_times = 0;
    network->candidate_meshnetid = BCAST_NETID;
}

static uint16_t calc_ssid_child_num(network_context_t *network)
{
    uint16_t num = 0;
    neighbor_t *nbr;
    neighbor_t *next_nbr;
    bool       dup = false;
    slist_t    *nbrs;

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
            if (next_nbr->addr.netid == mm_get_meshnetid(network) &&
                nbr->addr.addr.short_addr != INVALID_SID &&
                nbr->addr.addr.short_addr == next_nbr->addr.addr.short_addr) {
                dup = true;
            }
        }
        if (dup == false) {
            num += nbr->ssid_info.child_num;
        }
    }

    if (g_mm_state.device.state == DEVICE_STATE_LEADER) {
        num += get_allocated_pf_number(network);
    }

    /* including myelf */
    num += 1;

    return num;
}

ur_error_t send_advertisement(network_context_t *network)
{
    ur_error_t        error = UR_ERROR_NONE;
    message_t         *message;
    uint16_t          length;
    uint8_t           *data;
    mm_header_t       *mm_header;
    mm_netinfo_tv_t   *netinfo;
    mm_ssid_info_tv_t *ssid_info;
    mm_cost_tv_t      *path_cost;
    mm_mode_tv_t      *mode;
    message_info_t    *info;
    uint16_t          subnet_size = 0;

    if (network == NULL || mm_get_local_sid() == INVALID_SID) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_netinfo_tv_t) +
             sizeof(mm_cost_tv_t) + sizeof(mm_mode_tv_t);
    if (network->router->sid_type == STRUCTURED_SID) {
        length += sizeof(mm_ssid_info_tv_t);
    }
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ADVERTISEMENT;
    data += sizeof(mm_header_t);

    if (network->router->sid_type == STRUCTURED_SID) {
        subnet_size = calc_ssid_child_num(network);
    }
    switch (g_mm_state.device.state) {
        case DEVICE_STATE_LEADER:
            if (network->router->sid_type == SHORT_RANDOM_SID ||
                network->router->sid_type == RANDOM_SID) {
                nd_set_meshnetsize(network, rsid_get_allocated_number(network) + 1);
            }
        case DEVICE_STATE_SUPER_ROUTER:
            if (network->router->sid_type == STRUCTURED_SID) {
                nd_set_meshnetsize(network, subnet_size);
            }
            break;
        default:
            break;
    }

    netinfo = (mm_netinfo_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)netinfo, TYPE_NETWORK_INFO);
    netinfo->stable_version = (nd_get_stable_main_version() << STABLE_MAIN_VERSION_OFFSET) |
                               nd_get_stable_minor_version();
    netinfo->version        = nd_get_version(NULL);
    netinfo->size           = nd_get_meshnetsize(NULL);
    set_subnetsize_to_netinfo(netinfo, nd_get_meshnetsize(network));
    data += sizeof(mm_netinfo_tv_t);

    if (network->router->sid_type == STRUCTURED_SID) {
        ssid_info = (mm_ssid_info_tv_t *)data;
        mm_init_tv_base((mm_tv_t *)ssid_info, TYPE_SSID_INFO);
        ssid_info->child_num = subnet_size;
        ssid_info->free_slots = get_free_number(network);
        data += sizeof(mm_ssid_info_tv_t);
    }

    path_cost = (mm_cost_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)path_cost, TYPE_PATH_COST);
    path_cost->cost = network->path_cost;
    if (g_mm_state.device.state == DEVICE_STATE_LEAF) {
        path_cost->cost = INFINITY_PATH_COST;
    }
    data += sizeof(mm_cost_tv_t);

    mode = (mm_mode_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)mode, TYPE_MODE);
    mode->mode = (uint8_t)g_mm_state.device.mode;
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
           "send advertisement, len %d\r\n", length);
    return error;
}

static ur_error_t send_attach_request(network_context_t *network,
                                      ur_addr_t *dest)
{
    ur_error_t      error = UR_ERROR_NONE;
    uint16_t        length;
    mm_header_t     *mm_header;
    mm_version_tv_t *version;
    mm_ueid_tv_t    *src_ueid;
    mm_mode_tv_t    *mode;
    uint8_t         *data;
    message_t       *message = NULL;
    message_info_t  *info;

    length = sizeof(mm_header_t) + sizeof(mm_version_tv_t) +
             sizeof(mm_ueid_tv_t) + sizeof(mm_mode_tv_t);
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ATTACH_REQUEST;
    data += sizeof(mm_header_t);

    version = (mm_version_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)version, TYPE_VERSION);
    version->version = 1;
    data += sizeof(mm_version_tv_t);

    src_ueid = (mm_ueid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)src_ueid, TYPE_SRC_UEID);
    memcpy(src_ueid->ueid, g_mm_state.device.ueid, sizeof(src_ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    mode = (mm_mode_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)mode, TYPE_MODE);
    mode->mode = (uint8_t)g_mm_state.device.mode;
    data += sizeof(mm_mode_tv_t);

    info = message->info;
    info->network = network;
    // dest
    if (dest) {
        memcpy(&info->dest, dest, sizeof(info->dest));
    } else {
        info->dest.addr.len = SHORT_ADDR_SIZE;
        info->dest.addr.short_addr = BCAST_SID;
        info->dest.netid = network->candidate_meshnetid;
    }

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send attach request, len %d\r\n", length);
    return error;
}

static ur_error_t send_attach_response(network_context_t *network,
                                       ur_addr_t *dest)
{
    ur_error_t    error = UR_ERROR_NONE;
    mm_header_t   *mm_header;
    mm_ueid_tv_t  *src_ueid;
    mm_cost_tv_t  *path_cost;
    mm_mode_tv_t  *mode;
    mm_group_key_tv_t *group_key;
    message_t     *message;
    uint8_t       *data;
    uint16_t      length;
    message_info_t *info;

    if (network == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_cost_tv_t) +
             sizeof(mm_ueid_tv_t) + sizeof(mm_mode_tv_t);
    if (mm_get_seclevel() > SEC_LEVEL_0) {
        length += sizeof(mm_group_key_tv_t);
    }
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ATTACH_RESPONSE;
    data += sizeof(mm_header_t);

    src_ueid = (mm_ueid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)src_ueid, TYPE_SRC_UEID);
    memcpy(src_ueid->ueid, g_mm_state.device.ueid, sizeof(g_mm_state.device.ueid));
    data += sizeof(mm_ueid_tv_t);

    path_cost = (mm_cost_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)path_cost, TYPE_PATH_COST);
    path_cost->cost = network->path_cost;
    data += sizeof(mm_cost_tv_t);

    mode = (mm_mode_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)mode, TYPE_MODE);
    mode->mode = g_mm_state.device.mode;
    data += sizeof(mm_mode_tv_t);

    if (mm_get_seclevel() > SEC_LEVEL_0) {
        group_key = (mm_group_key_tv_t *)data;
        mm_init_tv_base((mm_tv_t *)group_key, TYPE_GROUP_KEY);
        memcpy(group_key->group_key, get_group_key(), sizeof(group_key->group_key));
        data += sizeof(mm_group_key_tv_t);
    }

    info = message->info;
    info->network = network;
    // dest
    memcpy(&info->dest, dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send attach response, len %d\r\n", length);
    return error;
}

static ur_error_t handle_attach_request(message_t *message)
{
    ur_error_t      error = UR_ERROR_NONE;
    mm_version_tv_t *version;
    mm_ueid_tv_t    *ueid;
    mm_mode_tv_t    *mode;
    uint8_t         *tlvs;
    uint16_t        tlvs_length;
    neighbor_t      *node;
    network_context_t *network;
    message_info_t *info;

    info = message->info;
    network = (network_context_t *)info->network;
    if (g_mm_state.device.state < DEVICE_STATE_LEADER ||
        network->attach_candidate) {
        return UR_ERROR_FAIL;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle attach request\r\n");

    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    if ((version = (mm_version_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_VERSION)) == NULL) {
        return UR_ERROR_FAIL;
    }

    if (version->version != 1) {
        return UR_ERROR_FAIL;
    }

    if ((ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_SRC_UEID)) == NULL) {
        return UR_ERROR_FAIL;
    }

    if ((mode = (mm_mode_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_MODE)) == NULL) {
        return UR_ERROR_FAIL;
    }

    node = get_neighbor_by_ueid(ueid->ueid);
    if (node && node == network->attach_node) {
        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM, "ignore attach point's attach request\r\n");
        return UR_ERROR_FAIL;
    }

    if (update_neighbor(info, tlvs, tlvs_length, true) == NULL) {
        return UR_ERROR_FAIL;
    }

    if (((mm_get_mode() & MODE_SUPER) && (mode->mode & MODE_SUPER)) ||
        ((mm_get_mode() & MODE_SUPER) == 0 && (mode->mode & MODE_SUPER) == 0)) {
        if (network != get_default_network_context()) {
            return UR_ERROR_NONE;
        }
    } else if ((mm_get_mode() & MODE_SUPER) && (mode->mode & MODE_SUPER) == 0) {
        if (network == get_default_network_context()) {
            return UR_ERROR_NONE;
        }
    } else if ((mm_get_mode() & MODE_SUPER) == 0 && (mode->mode & MODE_SUPER)) {
        return UR_ERROR_NONE;
    }

    send_attach_response(network, &info->src_mac);

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
           "attach response to " EXT_ADDR_FMT "\r\n", EXT_ADDR_DATA(info->src_mac.addr.addr));
    return error;
}

static ur_error_t handle_attach_response(message_t *message)
{
    neighbor_t    *nbr;
    mm_cost_tv_t  *path_cost;
    mm_group_key_tv_t *group_key;
    uint8_t       *tlvs;
    uint16_t      tlvs_length;
    network_context_t *network;
    message_info_t *info;

    info = message->info;
    network = info->network;
    if (network != get_default_network_context() ||
        g_mm_state.device.state == DEVICE_STATE_ATTACHED ||
        g_mm_state.device.state == DEVICE_STATE_DISABLED ||
        network->attach_state == ATTACH_DONE) {
        return UR_ERROR_NONE;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle attach response\r\n");

    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    nbr = update_neighbor(info, tlvs, tlvs_length, true);
    if (nbr == NULL) {
        return UR_ERROR_FAIL;
    }

    path_cost = (mm_cost_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_PATH_COST);
    if (path_cost == NULL) {
         return UR_ERROR_NONE;
    }

    group_key = (mm_group_key_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_GROUP_KEY);
    if (mm_get_seclevel() > SEC_LEVEL_0) {
        if (group_key == NULL) {
            return UR_ERROR_NONE;
        }
        set_group_key(group_key->group_key, sizeof(group_key->group_key));
    }

    if ((info->src.netid == network->prev_netid) &&
        (network->prev_path_cost < path_cost->cost)) {
        return UR_ERROR_NONE;
    }

    nbr->attach_candidate_timeout = 0;
    if (network->attach_candidate == NULL) {
        network->attach_candidate = nbr;
        network->attach_candidate->flags &= (~NBR_SID_CHANGED);
    }

    if (g_mm_state.device.state == DEVICE_STATE_DETACHED) {
        g_mm_state.device.state = DEVICE_STATE_ATTACHED;
        ur_router_start(network);
    }

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
           "try to attach network 0x%04x via node %04x\r\n",
           nbr->addr.netid, nbr->addr.addr.short_addr);

    if (network->attach_timer) {
        ur_stop_timer(&network->attach_timer, network);
    }

    if ((network->sid != INVALID_SID) && (g_mm_state.device.mode & MODE_MOBILE) &&
        (network->attach_candidate->addr.netid == network->meshnetid)) {
        network->attach_state = ATTACH_DONE;
        network->path_cost = nbr->path_cost + nbr->stats.link_cost;
        network->attach_node = nbr;
        network->attach_node->state = STATE_PARENT;
        network->attach_candidate = NULL;
        send_address_notification(network, NULL);
        if (network->advertisement_timer == NULL) {
            network->advertisement_timer = ur_start_timer(network->hal->advertisement_interval,
                                                          handle_advertisement_timer, network);
        }
        g_mm_state.callback->interface_up();
        start_keep_alive_timer(network);
    } else {
        network->attach_state = ATTACH_SID_REQUEST;
        send_sid_request(network);
        network->retry_times = 1;
        network->attach_timer = ur_start_timer(network->hal->sid_request_interval,
                                               handle_attach_timer, network);
    }
    return UR_ERROR_NONE;
}

static ur_error_t send_sid_request(network_context_t *network)
{
    ur_error_t   error = UR_ERROR_NONE;
    mm_header_t  *mm_header;
    mm_node_id_tv_t *attach_node_id;
    mm_sid_tv_t  *src_sid;
    mm_ueid_tv_t *src_ueid;
    mm_mode_tv_t *mode;
    message_t    *message;
    uint8_t      *data;
    uint16_t     length;
    message_info_t *info;

    if (network == NULL || network->attach_candidate == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_node_id_tv_t) + sizeof(mm_ueid_tv_t) +
             sizeof(mm_mode_tv_t);
    if (network->sid != INVALID_SID &&
        network->attach_candidate->addr.netid == network->meshnetid) {
        length += sizeof(mm_sid_tv_t);
    }
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_SID_REQUEST;
    data += sizeof(mm_header_t);

    attach_node_id = (mm_node_id_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)attach_node_id, TYPE_ATTACH_NODE_ID);
    attach_node_id->sid = network->attach_candidate->addr.addr.short_addr;
    attach_node_id->mode = g_mm_state.device.mode;
    attach_node_id->meshnetid = network->attach_candidate->addr.netid;
    data += sizeof(mm_node_id_tv_t);

    src_ueid = (mm_ueid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)src_ueid, TYPE_SRC_UEID);
    memcpy(src_ueid->ueid, g_mm_state.device.ueid, sizeof(src_ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    mode = (mm_mode_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)mode, TYPE_MODE);
    mode->mode = (uint8_t)g_mm_state.device.mode;
    data += sizeof(mm_mode_tv_t);

    if (network->sid != INVALID_SID &&
        network->attach_candidate->addr.netid == network->meshnetid) {
        src_sid = (mm_sid_tv_t *)data;
        mm_init_tv_base((mm_tv_t *)src_sid, TYPE_SRC_SID);
        src_sid->sid = network->sid;
        data += sizeof(mm_sid_tv_t);
    }

    info = message->info;
    info->network = network;
    // dest
    info->dest.addr.len = SHORT_ADDR_SIZE;
    if ((g_mm_state.device.mode & MODE_MOBILE) ||
        (network->router->sid_type != STRUCTURED_SID)) {
        info->dest.addr.short_addr = network->attach_candidate->addr.addr.short_addr;
        info->dest.netid = get_main_netid(network->attach_candidate->addr.netid);
        info->dest2.addr.len = SHORT_ADDR_SIZE;
        info->dest2.addr.short_addr = BCAST_SID;
        info->dest2.netid = BCAST_NETID;
    } else {
        info->dest.addr.short_addr = network->attach_candidate->addr.addr.short_addr;
        info->dest.netid = network->attach_candidate->addr.netid;
    }

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send sid request, len %d\r\n", length);
    return error;
}

static ur_error_t send_sid_response(network_context_t *network,
                                    ur_addr_t *dest, ur_addr_t *dest2,
                                    ur_node_id_t *node_id)
{
    ur_error_t        error = UR_ERROR_NONE;
    mm_header_t       *mm_header;
    mm_sid_tv_t       *allocated_sid;
    mm_node_type_tv_t *allocated_node_type;
    mm_netinfo_tv_t   *netinfo;
    mm_mcast_addr_tv_t *mcast;
    uint8_t           *data;
    message_t         *message;
    uint16_t          length;
    message_info_t    *info;

    if (network == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_sid_tv_t) +
             sizeof(mm_node_type_tv_t) + sizeof(mm_netinfo_tv_t) +
             sizeof(mm_mcast_addr_tv_t);
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_SID_RESPONSE;
    data += sizeof(mm_header_t);

    allocated_sid = (mm_sid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)allocated_sid, TYPE_ALLOCATE_SID);
    allocated_sid->sid = node_id->sid;
    data += sizeof(mm_sid_tv_t);

    allocated_node_type = (mm_node_type_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)allocated_node_type, TYPE_NODE_TYPE);
    allocated_node_type->type = node_id->type;
    data += sizeof(mm_node_type_tv_t);

    netinfo = (mm_netinfo_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)netinfo, TYPE_NETWORK_INFO);
    netinfo->stable_version = (nd_get_stable_main_version() << STABLE_MAIN_VERSION_OFFSET) |
                               nd_get_stable_minor_version();
    netinfo->version = nd_get_version(NULL);
    netinfo->size = nd_get_meshnetsize(NULL);
    netinfo->subnet_size_1  = (uint8_t)((nd_get_meshnetsize(network) >> 8) & 0x7);
    netinfo->subnet_size_2  = (uint8_t)(nd_get_meshnetsize(network) & 0xff);
    data += sizeof(mm_netinfo_tv_t);

    mcast = (mm_mcast_addr_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)mcast, TYPE_MCAST_ADDR);
    memcpy(mcast->mcast.m8, nd_get_subscribed_mcast(), 16);
    data += sizeof(mm_mcast_addr_tv_t);

    info = message->info;
    info->network = network;
    // dest
    memcpy(&info->dest, dest, sizeof(info->dest));
    memcpy(&info->dest2, dest2, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send sid response %04x:%d, len %d\r\n", node_id->sid, node_id->type, length);
    return error;
}

static ur_error_t handle_sid_request(message_t *message)
{
    ur_error_t   error = UR_ERROR_NONE;
    mm_node_id_tv_t  *attach_node_id;
    mm_sid_tv_t  *src_sid;
    mm_ueid_tv_t *ueid;
    mm_mode_tv_t *mode;
    uint8_t      *tlvs;
    uint16_t     tlvs_length;
    network_context_t *network;
    message_info_t *info;
    ur_node_id_t node_id;

    info = message->info;
    network = info->network;
    if (network->attach_candidate ||
        g_mm_state.device.state < DEVICE_STATE_LEADER) {
        return UR_ERROR_FAIL;
    }

    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    attach_node_id = (mm_node_id_tv_t *)mm_get_tv(tlvs, tlvs_length,
                                                 TYPE_ATTACH_NODE_ID);
    src_sid = (mm_sid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_SRC_SID);

    if ((ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_SRC_UEID)) == NULL) {
        return UR_ERROR_FAIL;
    }

    if ((mode = (mm_mode_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_MODE)) == NULL) {
        return UR_ERROR_FAIL;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle sid request\r\n");

    memset(&node_id, 0, sizeof(node_id));
    node_id.sid = INVALID_SID;
    if (src_sid) {
        node_id.sid = src_sid->sid;
    }

    if (mode->mode & MODE_MOBILE) {
        network = get_sub_network_context(network->hal);
    }
    switch (network->router->sid_type) {
        memcpy(&node_id.ueid, ueid->ueid, sizeof(node_id.ueid));
        case STRUCTURED_SID:
            if (attach_node_id->mode & MODE_SUPER) {
                node_id.attach_sid = SUPER_ROUTER_SID;
            } else {
                node_id.attach_sid = attach_node_id->sid;
            }
            node_id.mode = mode->mode;
            error = allocate_sid(network, &node_id);
            break;
        case SHORT_RANDOM_SID:
        case RANDOM_SID:
            error = rsid_allocate_sid(network, &node_id);
            break;
        default:
            error = UR_ERROR_PARSE;
    }
    if (error == UR_ERROR_NONE) {
        ur_addr_t dest;
        ur_addr_t dest2;
        dest.addr.len =  SHORT_ADDR_SIZE;
        dest.addr.short_addr = attach_node_id->sid;
        dest.netid = attach_node_id->meshnetid;
        dest2.addr.len = EXT_ADDR_SIZE;
        memcpy(dest2.addr.addr, ueid->ueid, sizeof(dest2.addr.addr));
        dest2.netid = BCAST_NETID;
        network = get_network_context_by_meshnetid(dest.netid);
        if (network == NULL) {
            network = get_default_network_context();
        }
        error = send_sid_response(network, &dest, &dest2, &node_id);
    }

    return error;
}

ur_error_t send_raw_data(network_context_t *network,
                         ur_addr_t *dest, ur_addr_t *dest2,
                         uint8_t *payload, uint8_t payload_length)
{
    ur_error_t     error = UR_ERROR_NONE;
    mm_header_t    *mm_header;
    mm_ueid_tv_t   *ueid;
    message_t      *message;
    uint8_t        *data;
    uint16_t       length;
    message_info_t *info;
    hal_context_t  *hal;
    neighbor_t     *nbr;

    hal = network->hal;
    if (dest == NULL && hal->discovery_result.meshnetid == BCAST_NETID) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_ueid_tv_t) + payload_length;
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }

    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_RAW_DATA;
    data += sizeof(mm_header_t);

    ueid = (mm_ueid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)ueid, TYPE_SRC_UEID);
    memcpy(ueid->ueid, g_mm_state.device.ueid, sizeof(ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    memcpy(data, payload, payload_length);
    data += payload_length;

    info = message->info;
    info->network = network;
    // dest
    if (dest == NULL) {
        nbr = get_neighbor_by_mac_addr(&hal->discovery_result.addr);
        if (nbr == NULL) {
            message_free(message);
            return UR_ERROR_FAIL;
        }
        info->dest.addr.len = SHORT_ADDR_SIZE;
        info->dest.addr.short_addr = nbr->addr.addr.short_addr;
        info->dest.netid = get_main_netid(nbr->addr.netid);
        info->dest2.addr.len = SHORT_ADDR_SIZE;
        info->dest2.addr.short_addr = BCAST_SID;
        info->dest2.netid = BCAST_NETID;
    } else {
        memcpy(&info->dest, dest, sizeof(info->dest));
        if (dest2) {
            memcpy(&info->dest2, dest2, sizeof(info->dest2));
        }
    }

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send raw data, len %d\r\n", length);

    return error;
}

ur_error_t register_raw_data_receiver(umesh_raw_data_received receiver)
{
    g_mm_state.raw_data_receiver = receiver;
    return UR_ERROR_NONE;
}

ur_error_t handle_raw_data(message_t *message)
{
    ur_error_t   error = UR_ERROR_NONE;
    mm_ueid_tv_t *ueid;
    uint8_t      *tlvs;
    ur_addr_t    src;
    uint8_t      *payload;
    uint8_t      length;

    tlvs = message_get_payload(message) + sizeof(mm_header_t);

    if ((ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, sizeof(mm_ueid_tv_t), TYPE_SRC_UEID)) == NULL) {
        return UR_ERROR_FAIL;
    }

    src.addr.len = EXT_ADDR_SIZE;
    memcpy(&src.addr.addr, ueid, sizeof(src.addr.addr));
    payload = tlvs + sizeof(mm_ueid_tv_t);
    length = message_get_msglen(message) - sizeof(mm_header_t) - sizeof(mm_ueid_tv_t);
    error = g_mm_state.raw_data_receiver(&src, payload, length);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle raw data\r\n");

    return error;
}

static ur_error_t handle_sid_response(message_t *message)
{
    ur_error_t        error = UR_ERROR_NONE;
    mm_sid_tv_t       *allocated_sid = NULL;
    mm_node_type_tv_t *allocated_node_type = NULL;
    mm_netinfo_tv_t   *netinfo;
    mm_mcast_addr_tv_t *mcast;
    uint8_t           *tlvs;
    uint16_t          tlvs_length;
    network_context_t *network;
    network_context_t *default_network;
    slist_t           *networks;
    uint8_t           index = 0;
    stable_network_data_t stable_network_data;
    message_info_t *info;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle sid response\r\n");

    info = message->info;
    network = info->network;
    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    allocated_sid = (mm_sid_tv_t *)mm_get_tv(tlvs, tlvs_length,
                                             TYPE_ALLOCATE_SID);
    if (allocated_sid == NULL) {
        return UR_ERROR_FAIL;
    }

    allocated_node_type = (mm_node_type_tv_t *)mm_get_tv(tlvs, tlvs_length,
                                                         TYPE_NODE_TYPE);
    if (allocated_node_type == NULL) {
        return UR_ERROR_FAIL;
    }

    netinfo = (mm_netinfo_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_NETWORK_INFO);
    if (netinfo == NULL) {
        return UR_ERROR_FAIL;
    }

    mcast = (mm_mcast_addr_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_MCAST_ADDR);
    if (mcast == NULL) {
        return UR_ERROR_FAIL;
    }
    stable_network_data.main_version =
                (netinfo->stable_version & STABLE_MAIN_VERSION_MASK) >> STABLE_MAIN_VERSION_OFFSET;
    stable_network_data.minor_version = netinfo->version & STABLE_MINOR_VERSION_MASK;
    stable_network_data.meshnetid = get_main_netid(info->src.netid);
    memcpy(stable_network_data.mcast_addr, &mcast->mcast, sizeof(mcast->mcast));
    nd_stable_set(&stable_network_data);

    write_prev_netinfo();

    switch (allocated_node_type->type) {
        case LEAF_NODE:
            g_mm_state.device.state = DEVICE_STATE_LEAF;
            break;
        case ROUTER_NODE:
            g_mm_state.device.state = DEVICE_STATE_ROUTER;
            if (g_mm_state.device.mode & MODE_SUPER) {
                g_mm_state.device.state = DEVICE_STATE_SUPER_ROUTER;
            }
            break;
        default:
            network->attach_state = ATTACH_IDLE;
            return UR_ERROR_FAIL;
    }
    network->sid = allocated_sid->sid;
    sid_allocator_init(network);
    network->attach_state = ATTACH_DONE;
    network->attach_node = network->attach_candidate;
    network->attach_candidate->flags &=
                      (~(NBR_SID_CHANGED | NBR_DISCOVERY_REQUEST | NBR_NETID_CHANGED));
    network->attach_candidate = NULL;
    network->candidate_meshnetid = BCAST_NETID;
    network->attach_node->stats.link_cost = 256;
    network->path_cost = network->attach_node->path_cost +
                         network->attach_node->stats.link_cost;
    network->attach_node->state = STATE_PARENT;
    network->change_sid = false;
    memset(&network->network_data, 0,  sizeof(network->network_data));
    ur_stop_timer(&network->attach_timer, network);
    ur_stop_timer(&network->advertisement_timer, network);
    start_neighbor_updater();
    nm_stop_discovery();
    network->meshnetid = network->attach_node->addr.netid;
    ur_router_sid_updated(network, network->sid);
    start_advertisement_timer(network);
    network->state = INTERFACE_UP;
    g_mm_state.callback->interface_up();
    start_keep_alive_timer(network);
    network->meshnetid = network->attach_node->addr.netid;
    ur_router_sid_updated(network, network->sid);
    send_address_notification(network, NULL);

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
             "allocate sid 0x%04x, become %s in net %04x\r\n",
              network->sid, state2str(g_mm_state.device.state), network->meshnetid);

    if (g_mm_state.device.state == DEVICE_STATE_SUPER_ROUTER ||
        g_mm_state.device.state == DEVICE_STATE_LEADER) {
        default_network = network;
        networks = get_network_contexts();
        slist_for_each_entry(networks, network, network_context_t, next) {
            if (default_network == network) {
                continue;
            }
            network->state = INTERFACE_UP;
            network->attach_state = ATTACH_DONE;
            network->attach_candidate = NULL;
            network->sid = default_network->sid;
            network->meshnetid = generate_meshnetid(network->sid, index);
            ++index;
            network->path_cost = 0;
            network->change_sid = false;
            if (network->attach_node) {
                network->attach_node->state = STATE_NEIGHBOR;
                network->attach_node = NULL;
            }
            ur_stop_timer(&network->attach_timer, network);

            ur_router_start(network);
            ur_router_sid_updated(network, LEADER_SID);
            sid_allocator_init(network);
            start_advertisement_timer(network);
        }
    }

    return error;
}

void become_detached(void)
{
    slist_t *networks;
    network_context_t *network;

    write_prev_netinfo();
    g_mm_state.device.state = DEVICE_STATE_DETACHED;
    ur_stop_timer(&g_mm_state.device.alive_timer, NULL);
    reset_network_context();
    nd_init();
    nm_stop_discovery();
    stop_neighbor_updater();
    stop_addr_cache();
    g_mm_state.callback->interface_down();

    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        sid_allocator_deinit(network);
    }

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM, "become detached\r\n");
}

ur_error_t attach_start(hal_context_t *hal, neighbor_t *nbr)
{
    ur_error_t        error = UR_ERROR_NONE;
    network_context_t *network = NULL;

    network = get_default_network_context();
    if (network->attach_candidate ||
        (network->attach_state != ATTACH_IDLE && network->attach_state != ATTACH_DONE)) {
        return UR_ERROR_BUSY;
    }

    if (nbr == NULL && g_mm_state.device.state > DEVICE_STATE_DETACHED) {
        become_detached();
    }
    if (nbr && nbr->attach_candidate_timeout > 0) {
        nbr = NULL;
    } else if (nbr) {
        nbr->attach_candidate_timeout = ATTACH_CANDIDATE_TIMEOUT;
    }

    if (nbr == NULL && g_mm_state.device.state > DEVICE_STATE_ATTACHED) {
        return error;
    }

    network->attach_state = ATTACH_REQUEST;
    network->attach_candidate = nbr;
    if (nbr) {
        network->candidate_meshnetid = nbr->addr.netid;
        send_attach_request(network, &nbr->addr);
    } else {
        send_attach_request(network, NULL);
    }
    ur_stop_timer(&network->attach_timer, network);
    network->attach_timer = ur_start_timer(network->hal->attach_request_interval,
                                           handle_attach_timer, network);
    network->retry_times = 1;
    stop_neighbor_updater();
    ur_stop_timer(&network->advertisement_timer, network);

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
           "%s node, attach start, from %04x:%04x to %04x:%x\r\n",
           state2str(g_mm_state.device.state), network->attach_node?
           network->attach_node->addr.addr.short_addr: 0,
           network->meshnetid, nbr?nbr->addr.addr.short_addr:0, network->candidate_meshnetid);

    return error;
}

static uint16_t compute_network_metric(uint16_t size, uint16_t path_cost)
{
    return size / SIZE_WEIGHT + path_cost / PATH_COST_WEIGHT;
}

static void write_prev_netinfo(void)
{
    network_context_t *network;
    ur_configs_t configs;

    network = get_default_network_context();
    if (network == NULL) {
        return;
    }

    if (network->meshnetid == INVALID_NETID ||
        network->path_cost == INFINITY_PATH_COST) {
        return;
    }

    ur_configs_read(&configs);
    configs.prev_netinfo.meshnetid = network->meshnetid;
    configs.prev_netinfo.path_cost = network->path_cost;
    ur_configs_write(&configs);

    network->prev_netid = network->meshnetid;
    network->prev_path_cost = network->path_cost;
}

static void read_prev_netinfo(void)
{
    network_context_t *network;
    ur_configs_t configs;

    network = get_default_network_context();
    if (network == NULL) {
        return;
    }
    ur_configs_read(&configs);
    network->prev_netid = configs.prev_netinfo.meshnetid;
    network->prev_path_cost = configs.prev_netinfo.path_cost;
}

static void update_migrate_times(network_context_t *network, neighbor_t *nbr)
{
    bool     migrate = false;
    uint16_t netid;

    netid = nbr->addr.netid;
    if (netid == network->candidate_meshnetid ||
        network->candidate_meshnetid == BCAST_NETID) {
        network->candidate_meshnetid = netid;
        if (network->migrate_times == 0) {
            network->migrate_wait_timer = ur_start_timer(network->migrate_interval,
                                                         handle_migrate_wait_timer, network);
        }
        network->migrate_times++;
        if (network->migrate_times >= MIGRATE_TIMEOUT) {
            migrate = true;
        }
    }

    if (migrate == false) {
        return;
    }

    if (network->router->sid_type == STRUCTURED_SID && nbr->ssid_info.free_slots < 1) {
        nbr = NULL;
    }
    if (nbr == NULL && g_mm_state.device.state > DEVICE_STATE_ATTACHED) {
        return;
    }

    network->migrate_times = 0;
    ur_stop_timer(&network->migrate_wait_timer, network);
    attach_start(network->hal, nbr);
}

static ur_error_t handle_advertisement(message_t *message)
{
    uint8_t           *tlvs;
    uint16_t          tlvs_length;
    neighbor_t        *nbr;
    mm_netinfo_tv_t   *netinfo;
    mm_cost_tv_t      *path_cost;
    mm_mode_tv_t      *mode;
    uint16_t          new_metric;
    uint16_t          cur_metric;
    uint8_t           main_version;
    int8_t            diff;
    network_context_t *network;
    neighbor_t        *attach_node;
    bool              from_same_net = false;
    bool              from_same_core = false;
    bool              leader_reboot = false;
    uint16_t          subnet_size = 0;
    uint8_t           tlv_type;
    message_info_t    *info;

    if (g_mm_state.device.state < DEVICE_STATE_DETACHED) {
        return UR_ERROR_NONE;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle advertisement\r\n");

    info = message->info;
    network = info->network;

    attach_node = network->attach_node;
    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    netinfo = (mm_netinfo_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_NETWORK_INFO);
    path_cost = (mm_cost_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_PATH_COST);
    mode = (mm_mode_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_MODE);
    if (netinfo == NULL || info->src.netid == BCAST_NETID ||
        path_cost == NULL || mode == NULL) {
        return UR_ERROR_FAIL;
    }

    if ((is_subnet(network->meshnetid) && is_subnet(info->src.netid) == 0) ||
        (is_subnet(network->meshnetid) == 0 && is_subnet(info->src.netid))) {
        return UR_ERROR_NONE;
    }

    nbr = update_neighbor(info, tlvs, tlvs_length, false);
    if (nbr == NULL) {
        return UR_ERROR_NONE;
    }

    if (g_mm_state.device.state > DEVICE_STATE_ATTACHED &&
        memcmp(nbr->ueid, INVALID_UEID, sizeof(nbr->ueid)) == 0) {
        tlv_type = TYPE_TARGET_UEID;
        send_link_request(network, &nbr->addr, &tlv_type, 1);
    }

    // mode super should be leader
    if ((mode->mode & MODE_SUPER) && (g_mm_state.device.mode & MODE_SUPER) == 0 &&
         g_mm_state.device.state == DEVICE_STATE_LEADER) {
        become_detached();
        update_migrate_times(network, nbr);
        return UR_ERROR_NONE;
    }

    // detached node try to migrate
    if (g_mm_state.device.state == DEVICE_STATE_DETACHED) {
        update_migrate_times(network, nbr);
        return UR_ERROR_NONE;
    }

    // attach node changed
    if (nbr == attach_node &&
        ((nbr->flags & NBR_NETID_CHANGED) || (nbr->flags & NBR_SID_CHANGED))) {
        become_detached();
        nbr->flags &= (~(NBR_NETID_CHANGED | NBR_SID_CHANGED)) ;
        attach_start(network->hal, nbr);
        return UR_ERROR_NONE;
    }

    // not try to migrate to pf node
    if (mode->mode & MODE_MOBILE) {
        return UR_ERROR_NONE;
    }
    // leader not try to migrate to the same net
    if (network->meshnetid == info->src.netid) {
        from_same_net = true;
    } else if (is_same_mainnet(network->meshnetid, info->src.netid)) {
        from_same_core = true;
    }
    if (from_same_net &&
        (attach_node == NULL || g_mm_state.device.state == DEVICE_STATE_LEADER)) {
        return UR_ERROR_NONE;
    }

    if (from_same_net) {
        main_version = (netinfo->stable_version & STABLE_MAIN_VERSION_MASK) >>
                       STABLE_MAIN_VERSION_OFFSET;
        diff = (main_version + 8 - nd_get_stable_main_version()) % 8;
        if (diff > 0 && diff < 4 &&
            g_mm_state.device.state > DEVICE_STATE_ATTACHED &&
            g_mm_state.device.state != DEVICE_STATE_LEADER) {
            nd_set_stable_main_version(main_version);
            if (g_mm_state.device.mode & MODE_MOBILE) {
                leader_reboot = true;
            }
        }
        if ((nbr == attach_node) &&
            (attach_node->flags & NBR_DISCOVERY_REQUEST) &&
            ((attach_node->flags & NBR_SID_CHANGED) == 0)) {
            leader_reboot = true;
            attach_node->flags &= (~NBR_DISCOVERY_REQUEST);
        }
        if (leader_reboot) {
            network->attach_candidate = network->attach_node;
            network->attach_state = ATTACH_SID_REQUEST;
            ur_stop_timer(&network->attach_timer, network);
            network->attach_timer = ur_start_timer(network->hal->sid_request_interval,
                                                   handle_attach_timer, network);
            send_sid_request(network);
        }
        if (attach_node == nbr) {
            network->path_cost = nbr->path_cost + nbr->stats.link_cost;
        }

        if (network->path_cost <= (nbr->path_cost + PATH_COST_SWITCH_HYST) ||
            attach_node == nbr) {
            return UR_ERROR_NONE;
        }
    } else {
        if ((nbr->addr.netid == network->prev_netid) &&
            (network->prev_path_cost < nbr->path_cost)) {
            return UR_ERROR_NONE;
        }
        subnet_size = (netinfo->subnet_size_1 << 8) | netinfo->subnet_size_2;
        new_metric = compute_network_metric(subnet_size, 0);
        cur_metric = compute_network_metric(nd_get_meshnetsize(network), 0);
        if (from_same_core) {
            if (cur_metric < (new_metric + 5)) {
                return UR_ERROR_NONE;
            }
        } else {
            if ((new_metric < cur_metric) ||
                (new_metric == cur_metric && info->src.netid <= network->meshnetid)) {
                return UR_ERROR_NONE;
            }
        }
    }
    update_migrate_times(network, nbr);

    return UR_ERROR_NONE;
}

ur_error_t mm_handle_frame_received(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
    mm_header_t *mm_header;

    mm_header = (mm_header_t *)message_get_payload(message);
    switch (mm_header->command & COMMAND_COMMAND_MASK) {
        case COMMAND_ADVERTISEMENT:
            handle_advertisement(message);
            break;
        case COMMAND_DISCOVERY_REQUEST:
            handle_discovery_request(message);
            break;
        case COMMAND_DISCOVERY_RESPONSE:
            handle_discovery_response(message);
            break;
        case COMMAND_DATA_REQUEST:
            break;
        case COMMAND_DATA_RESPONSE:
            break;
        case COMMAND_ATTACH_REQUEST:
            error = handle_attach_request(message);
            break;
        case COMMAND_ATTACH_RESPONSE:
            error = handle_attach_response(message);
            break;
        case COMMAND_NETWORK_DATA_REQUEST:
            break;
        case COMMAND_NETWORK_DATA_RESPONSE:
            break;
        case COMMAND_SID_REQUEST:
            error = handle_sid_request(message);
            break;
        case COMMAND_SID_RESPONSE:
            error = handle_sid_response(message);
            break;
        case COMMAND_ADDRESS_QUERY:
            error = handle_address_query(message);
            break;
        case COMMAND_ADDRESS_QUERY_RESPONSE:
            error = handle_address_query_response(message);
            break;
        case COMMAND_ADDRESS_NOTIFICATION:
            error = handle_address_notification(message);
            break;
        case COMMAND_LINK_REQUEST:
            error = handle_link_request(message);
            break;
        case COMMAND_LINK_ACCEPT:
            error = handle_link_accept(message);
            break;
        case COMMAND_LINK_ACCEPT_AND_REQUEST:
            error = handle_link_accept_and_request(message);
            break;
        case COMMAND_LINK_REJECT:
            error = handle_link_reject(message);
            break;
        case COMMAND_DEST_UNREACHABLE:
            break;
        case COMMAND_ADDRESS_ERROR:
            break;
        case COMMAND_ROUTING_INFO_UPDATE:
            error = handle_router_message_received(message);
            break;
        case COMMAND_RAW_DATA:
            error = handle_raw_data(message);
            break;
        default:
            break;
    }
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "cmd %d error %d\r\n",
           mm_header->command & COMMAND_COMMAND_MASK, error);
    return error;
}

ur_error_t mm_init(void)
{
    ur_error_t error = UR_ERROR_NONE;
    ur_configs_t configs;

    // init device
    g_mm_state.device.state = DEVICE_STATE_DISABLED;
    memcpy(g_mm_state.device.ueid, hal_ur_mesh_get_mac_address(NULL),
           sizeof(g_mm_state.device.ueid));
    g_mm_state.device.reboot_flag = true;

    g_mm_state.device.seclevel = SEC_LEVEL_1;

    memset(&g_mm_state.network_data_updater, 0 , sizeof(g_mm_state.network_data_updater));
    g_mm_state.network_data_updater.handler = network_data_update_handler;
    nd_register_update_handler(&g_mm_state.network_data_updater);

    register_neighbor_updater(neighbor_updated_handler);

    memset(&configs, 0, sizeof(configs));
    ur_configs_read(&configs);
    nd_set_stable_main_version(configs.main_version);

    g_mm_state.device.mode = MODE_RX_ON;
    if (get_hal_contexts_num() > 1) {
        g_mm_state.device.mode |= MODE_SUPER;
    }
    return error;
}

ur_error_t mm_deinit(void)
{
    nd_unregister_update_handler(&g_mm_state.network_data_updater);
    return UR_ERROR_NONE;
}

ur_error_t mm_start(mm_cb_t *mm_cb)
{
    assert(mm_cb);

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM, "ur started\r\n");

    reset_network_context();
    read_prev_netinfo();
    g_mm_state.device.state = DEVICE_STATE_DETACHED;
    g_mm_state.callback = mm_cb;
    g_mm_state.device.alive_timer = NULL;
    return nm_start_discovery();
}

ur_error_t mm_stop(void)
{
    stop_neighbor_updater();
    mf_init();
    nd_init();
    nm_stop_discovery();
    ur_router_stop();
    become_detached();
    /* finally free all neighbor structures */
    neighbors_init();
    g_mm_state.device.state = DEVICE_STATE_DISABLED;
    return UR_ERROR_NONE;
}

void mm_set_meshnetid(network_context_t *network, uint16_t meshnetid)
{
    network = network ? : get_default_network_context();
    nd_set_stable_meshnetid(meshnetid);
    network->meshnetid = meshnetid;

    if (g_mm_state.device.state == DEVICE_STATE_DETACHED) {
        nm_start_discovery();
    }
}

uint16_t mm_get_meshnetid(network_context_t *network)
{
    uint16_t meshnetid = BCAST_NETID;

    if (network == NULL) {
        network = get_default_network_context();
    }
    if (network == NULL) {
        return meshnetid;
    }

    if (network->attach_state == ATTACH_IDLE || network->attach_state == ATTACH_DONE) {
        meshnetid = network->meshnetid;
    } else if (network->attach_candidate) {
        meshnetid = network->attach_candidate->addr.netid;
    }
    return meshnetid;
}

uint16_t mm_get_meshnetsize(void)
{
    return nd_get_meshnetsize(NULL);
}

uint16_t mm_get_local_sid(void)
{
    network_context_t *network;

    network = get_default_network_context();
    if (!network) {
        return INVALID_SID;
    }

    return network->sid;
}

ur_error_t mm_set_local_sid(uint16_t sid)
{
    network_context_t *network;

    network = get_default_network_context();
    network->sid = sid;
    if (g_mm_state.device.state > DEVICE_STATE_ATTACHED) {
        network->change_sid = true;
        network->attach_state = ATTACH_SID_REQUEST;
        ur_stop_timer(&network->attach_timer, network);
        network->attach_timer = ur_start_timer(network->hal->sid_request_interval,
                                               handle_attach_timer, network);
        send_sid_request(network);
    }
    return UR_ERROR_NONE;
}

uint8_t *mm_get_local_ueid(void)
{
    return g_mm_state.device.ueid;
}

uint16_t mm_get_path_cost(void)
{
    network_context_t *network;

    network = get_default_network_context();
    return network->path_cost;
}

ur_error_t mm_set_mode(node_mode_t mode)
{
    g_mm_state.device.mode = mode;
    return UR_ERROR_NONE;
}

node_mode_t mm_get_mode(void)
{
    return g_mm_state.device.mode;
}

ur_error_t mm_set_seclevel(int8_t level)
{
    if (level >= SEC_LEVEL_0 && level <= SEC_LEVEL_1) {
        g_mm_state.device.seclevel = level;
        return UR_ERROR_NONE;
    }

    return UR_ERROR_FAIL;
}

int8_t mm_get_seclevel(void)
{
    return g_mm_state.device.seclevel;
}

const mac_address_t *mm_get_mac_address(void)
{
    hal_context_t *hal;
    network_context_t *network;

    network = get_default_network_context();
    if (network) {
        hal = network->hal;
        return &hal->mac_addr;
    }
    return NULL;
}

uint16_t mm_get_channel(network_context_t *network)
{
    network = network ? : get_default_network_context();
    if (network) {
        return network->channel;
    } else {
        return 0xffff;
    }
}

void mm_set_channel(network_context_t *network, uint16_t channel)
{
    network = network ? : get_default_network_context();
    hal_ur_mesh_set_bcast_channel(network->hal->module, channel);
    network->channel = channel;
}

mm_device_state_t mm_get_device_state(void)
{
    return g_mm_state.device.state;
}

attach_state_t mm_get_attach_state(void)
{
    network_context_t *network;

    network = get_default_network_context();
    return network->attach_state;
}

neighbor_t *mm_get_attach_node(network_context_t *network)
{
    network = network ? : get_default_network_context();
    if (!network)
        return NULL;

    return network->attach_node;
}

neighbor_t *mm_get_attach_candidate(network_context_t *network)
{
    network = network ? : get_default_network_context();
    if (!network)
        return NULL;

    return network->attach_candidate;
}
