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

#include "hal/mesh.h"
#include "mesh_types.h"
#include "urmesh.h"
#include "mesh_forwarder.h"
#include "mesh_mgmt.h"
#include "mesh_mgmt_tlvs.h"
#include "sid_allocator.h"
#include "router_mgr.h"
#include "logging.h"
#include "timer.h"
#include "network_data.h"
#include "address_resolver.h"
#include "link_mgmt.h"
#include "network_mgmt.h"
#include "memory.h"
#include "interfaces.h"
#include "address_cache.h"
#include "hals.h"
#include "keys_mgr.h"

typedef struct mm_device_s {
    mm_device_state_t state;
    node_mode_t       mode;
    uint8_t           ueid[8];
    bool              reboot_flag;
    ur_timer_t        alive_timer;
    uint8_t           seclevel;
} mm_device_t;

typedef struct mesh_mgmt_state_s {
    mm_device_t  device;
    mm_cb_t      *callback;
    nd_updater_t network_data_updater;
} mesh_mgmt_state_t;

static mesh_mgmt_state_t g_mm_state;

static void handle_attach_timer(void *args);
static void handle_advertisement_timer(void *args);
static void handle_migrate_wait_timer(void *args);

static ur_error_t send_attach_request(network_context_t *network,
                                      neighbor_t *dest);
static ur_error_t send_attach_response(network_context_t *network,
                                       const mac_address_t *addr);
static ur_error_t send_network_data_request(network_context_t *network,
                                            neighbor_t *dest);
static ur_error_t send_network_data_response(network_context_t *context,
                                             const mac_address_t *dest,
                                             const uint8_t *tlv_types,
                                             uint8_t tlv_types_length);
static ur_error_t send_sid_request(network_context_t *network);
static ur_error_t send_sid_response(network_context_t *network,
                                    uint16_t alloc_sid, uint8_t alloc_node_type,
                                    ur_node_id_t * relayer, uint8_t *target_ueid);
static ur_error_t send_advertisement(network_context_t *network);

static bool update_network_data(network_context_t *context,
                                mm_netinfo_tv_t *netinfo, bool from_same_net);
static void write_prev_netinfo(network_context_t *network);
static void read_prev_netinfo(network_context_t *network);

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

static ur_error_t message_relayer(network_context_t *network,
                                  const neighbor_t *node,
                                  uint8_t *payload, uint16_t length)
{
    ur_error_t  error = UR_ERROR_NONE;
    message_t   *message;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "message_relayer\r\n");

    if (node != NULL &&
        (message = message_alloc(network, length)) != NULL) {
        message_copy_from(message, payload, length);
        memcpy(&message->dest->dest, &node->addr, sizeof(message->dest->dest));
        message->dest->flags = INSERT_DESTNETID_FLAG | ENCRYPT_ENABLE_FLAG;
        message->dest->meshnetid = BCAST_NETID;
        error = mf_send_command(0, message);
    } else {
        error = UR_ERROR_DROP;
    }
    return error;
}

static void set_default_network_data(void)
{
    network_data_t        network_data;
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

static uint8_t generate_sub_meshnetid(uint8_t sid, uint8_t index)
{
    return ((sid << 2) | index);
}

static void handle_device_alive_timer(void *args)
{
    network_context_t *network;
    uint32_t interval;

    g_mm_state.device.alive_timer = NULL;
    network = get_default_network_context();
    switch (network->hal->module->type) {
        case MEDIA_TYPE_WIFI:
            interval = WIFI_NOTIFICATION_TIMEOUT;
            break;
        case MEDIA_TYPE_BLE:
            interval = BLE_NOTIFICATION_TIMEOUT;
            break;
        case MEDIA_TYPE_15_4:
            interval = IEEE154_NOTIFICATION_TIMEOUT;
            break;
        default:
            assert(0);
            break;
    }
    send_address_notification(network, LEADER_SID);

    g_mm_state.device.alive_timer = ur_start_timer(interval, handle_device_alive_timer, NULL);
}

static void start_keep_alive_timer(network_context_t *network)
{
    hal_context_t *hal;
    uint32_t      interval;

    if (g_mm_state.device.state == DEVICE_STATE_LEADER) {
        return;
    }

    hal = get_default_hal_context();
    if (network->hal == hal && g_mm_state.device.alive_timer == NULL) {
        switch (hal->module->type) {
            case MEDIA_TYPE_WIFI:
                interval = WIFI_NOTIFICATION_TIMEOUT;
                break;
            case MEDIA_TYPE_BLE:
                interval = BLE_NOTIFICATION_TIMEOUT;
                break;
            case MEDIA_TYPE_15_4:
                interval = IEEE154_NOTIFICATION_TIMEOUT;
                break;
            default:
                assert(0);
                break;
        }
        g_mm_state.device.alive_timer = ur_start_timer(interval, handle_device_alive_timer, NULL);
    }
}

void become_leader(void)
{
    ur_configs_t      configs;
    slist_t           *networks;
    network_context_t *network;
    uint8_t           index = 0;

    set_default_network_data();
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
        if (network->index == 0) {
            network->meshnetid = nd_get_stable_meshnetid();
        } else {
            network->meshnetid = nd_get_stable_meshnetid() |
                                 generate_sub_meshnetid(network->sid, ++index);
        }
        network->path_cost = 0;
        network->change_sid = false;
        rsid_allocator_deinit();
        rsid_allocator_init();
        if (network->attach_node) {
            network->attach_node->state = STATE_NEIGHBOR;
            network->attach_node = NULL;
        }
        network->attach_candidate = NULL;
        ur_stop_timer(&network->attach_timer, network);

        send_advertisement(network);
        if (network->advertisement_timer == NULL) {
            network->advertisement_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
                                                   handle_advertisement_timer, network);
        }

        write_prev_netinfo(network);
        ur_router_start(network);
        ur_router_sid_updated(network, LEADER_SID);
    }
    allocator_init(g_mm_state.device.ueid, SUPER_ROUTER_SID);

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
                send_attach_request(network, network->attach_candidate);
                network->attach_timer = ur_start_timer(ATTACH_REQUEST_TIMEOUT,
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
                network->attach_timer = ur_start_timer(SID_REQUEST_TIMEOUT,
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
        if (g_mm_state.device.state > DEVICE_STATE_ATTACHED &&
            network->advertisement_timer == NULL) {
            send_advertisement(network);
            network->advertisement_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
                                                          handle_advertisement_timer, args);
        }
    }
}

static void handle_advertisement_timer(void *args)
{
    network_context_t *network = (network_context_t *)args;
    uint16_t meshnetid;

    network->advertisement_timer = NULL;
    if (send_advertisement(network) == UR_ERROR_FAIL) {
        return;
    }
    network->advertisement_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
                                                  handle_advertisement_timer, args);

    meshnetid = get_main_netid(network->meshnetid);
    if (meshnetid != nd_get_stable_meshnetid() &&
        g_mm_state.device.state >= DEVICE_STATE_LEAF) {
        send_network_data_request(network, network->attach_node);
    }
}

static void handle_migrate_wait_timer(void *args)
{
    network_context_t *network = (network_context_t *)args;

    network->migrate_wait_timer = NULL;
    network->migrate_timeout = 0;
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
        if (nbr->state != STATE_CHILD || network->meshnetid != nbr->netid ||
            network->meshnetid != nbr->sub_netid) {
            continue;
        }
        dup = false;
        slist_for_each_entry(nbrs, next_nbr, neighbor_t, next) {
            if (nbr == next_nbr) {
                continue;
            }
            if (next_nbr->netid == mm_get_meshnetid(network) &&
                nbr->sid != INVALID_SID && nbr->sid == next_nbr->sid) {
                dup = true;
            }
        }
        if (dup == false) {
            num += nbr->child_num;
        }
    }

    if (g_mm_state.device.state == DEVICE_STATE_LEADER) {
        num += get_allocated_pf_number();
    }

    /* including ourself */
    num += 1;

    return num;
}

ur_error_t send_advertisement(network_context_t *network)
{
    ur_error_t      error = UR_ERROR_NONE;
    message_t       *message;
    uint16_t        length;
    uint8_t         *data;
    mm_header_t     *mm_header;
    mm_netinfo_tv_t *netinfo;
    mm_cost_tv_t    *path_cost;
    mm_mode_tv_t    *mode;
    mesh_dest_t     *dest;
    uint16_t        subnet_size = 0;
    network_context_t *default_network;

    default_network = get_default_network_context();
    if (network == NULL || default_network->sid == INVALID_SID) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_netinfo_tv_t) +
             sizeof(mm_cost_tv_t) + sizeof(mm_mode_tv_t);
    message = message_alloc(network, length);
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
                nd_set_meshnetsize(network, rsid_get_allocated_number() + 1);
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
    netinfo->subnet_version = nd_get_version(network);
    set_subnetsize_to_netinfo(netinfo, nd_get_meshnetsize(network));
    if (network->router->sid_type == STRUCTURED_SID) {
        netinfo->child_num = subnet_size;
    }
    netinfo->meshnetid = default_network->meshnetid;
    default_network = get_sub_network_context(network->hal);
    if (default_network) {
        netinfo->sub_meshnetid = get_sub_netid(default_network->meshnetid);
    } else {
        netinfo->sub_meshnetid = get_sub_netid(netinfo->meshnetid);
    }
    netinfo->free_slots = get_free_number();
    data += sizeof(mm_netinfo_tv_t);

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

    dest = message->dest;
    dest->dest.len = sizeof(dest->dest.addr);
    memset(dest->dest.addr, 0xff, sizeof(dest->dest.addr));
    dest->flags = INSERT_DESTNETID_FLAG;
    dest->meshnetid = BCAST_NETID;
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send advertisement, len %d\r\n", length);
    return error;
}

static ur_error_t send_attach_request(network_context_t *network,
                                      neighbor_t *dest)
{
    ur_error_t      error = UR_ERROR_NONE;
    uint16_t        length;
    mm_header_t     *mm_header;
    mm_version_tv_t *version;
    mm_ueid_tv_t    *src_ueid;
    mm_mode_tv_t    *mode;
    uint8_t         *data;
    message_t       *message = NULL;
    mesh_dest_t     *mesh_dest;

    length = sizeof(mm_header_t) + sizeof(mm_version_tv_t) +
             sizeof(mm_ueid_tv_t) + sizeof(mm_mode_tv_t);
    message = message_alloc(network, length);
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

    mesh_dest = message->dest;
    mesh_dest->flags = 0;
    if (dest) {
        memcpy(&mesh_dest->dest, &dest->addr, sizeof(mesh_dest->dest));
        mesh_dest->flags = INSERT_DESTNETID_FLAG;
        mesh_dest->meshnetid = network->candidate_meshnetid;
    } else {
        mesh_dest->dest.len = sizeof(mesh_dest->dest.addr);
        memset(mesh_dest->dest.addr, 0xff, sizeof(mesh_dest->dest.addr));
    }
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send attach request, len %d\r\n", length);
    return error;
}

static ur_error_t send_attach_response(network_context_t *network,
                                       const mac_address_t *addr)
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
    mesh_dest_t   *dest;

    if (network == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_cost_tv_t) +
             sizeof(mm_ueid_tv_t) + sizeof(mm_mode_tv_t);
    if (mm_get_seclevel() > SEC_LEVEL_0) {
        length += sizeof(mm_group_key_tv_t);
    }
    message = message_alloc(network, length);
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

    dest = message->dest;
    memcpy(&dest->dest, addr, sizeof(dest->dest));
    dest->flags = 0;
    dest->meshnetid = network->meshnetid;
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send attach response, len %d\r\n", length);
    return error;
}

static ur_error_t handle_attach_request(uint8_t *payload, uint16_t length,
                                        const mesh_src_t *src,
                                        const mac_address_t *dest)
{
    ur_error_t      error = UR_ERROR_NONE;
    mm_version_tv_t *version;
    mm_ueid_tv_t    *ueid;
    mm_mode_tv_t    *mode;
    uint8_t         *tlvs;
    uint16_t        tlvs_length;
    neighbor_t      *node;
    network_context_t *network;

    network = src->dest_network;
    if (g_mm_state.device.state < DEVICE_STATE_LEADER ||
        network->attach_candidate) {
        return UR_ERROR_FAIL;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle attach request\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);
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

    if (update_neighbor(src, tlvs, tlvs_length, true) == NULL) {
        return UR_ERROR_FAIL;
    }

    if (((mm_get_mode() & MODE_SUPER) && (mode->mode & MODE_SUPER)) ||
        ((mm_get_mode() & MODE_SUPER) == 0 && (mode->mode & MODE_SUPER) == 0)) {
        if (src->dest_network != get_default_network_context()) {
            return UR_ERROR_NONE;
        }
    } else if ((mm_get_mode() & MODE_SUPER) && (mode->mode & MODE_SUPER) == 0) {
        if (src->dest_network == get_default_network_context()) {
            return UR_ERROR_NONE;
        }
    } else if ((mm_get_mode() & MODE_SUPER) == 0 && (mode->mode & MODE_SUPER)) {
        return UR_ERROR_NONE;
    }

    send_attach_response(network, &src->src);

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
           "attach response to " EXT_ADDR_FMT "\r\n", EXT_ADDR_DATA(src->src.addr));
    return error;
}

static ur_error_t handle_attach_response(uint8_t *payload, uint16_t length,
                                         const mesh_src_t *src,
                                         const mac_address_t *dest)
{
    neighbor_t    *nbr;
    mm_cost_tv_t  *path_cost;
    mm_group_key_tv_t *group_key;
    uint8_t       *tlvs;
    uint16_t      tlvs_length;
    network_context_t *network;

    network = src->dest_network;
    if (network != get_default_network_context() ||
        g_mm_state.device.state == DEVICE_STATE_ATTACHED ||
        g_mm_state.device.state == DEVICE_STATE_DISABLED ||
        network->attach_state == ATTACH_DONE) {
        return UR_ERROR_NONE;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle attach response\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    nbr = update_neighbor(src, tlvs, tlvs_length, true);
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

    if ((src->meshnetid == network->prev_netid) &&
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
           nbr->netid, nbr->sid);

    if (network->attach_timer) {
        ur_stop_timer(&network->attach_timer, network);
    }

    if ((network->sid != INVALID_SID) && (g_mm_state.device.mode & MODE_MOBILE) &&
        (network->attach_candidate->netid == network->meshnetid)) {
        network->attach_state = ATTACH_DONE;
        network->path_cost = nbr->path_cost + nbr->stats.link_cost;
        network->attach_node = nbr;
        network->attach_node->state = STATE_PARENT;
        network->attach_candidate = NULL;
        send_address_notification(network, LEADER_SID);
        if (network->advertisement_timer == NULL) {
            network->advertisement_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
                                                   handle_advertisement_timer, network);
        }
        g_mm_state.callback->interface_up();
        start_keep_alive_timer(network);
    } else {
        network->attach_state = ATTACH_SID_REQUEST;
        send_sid_request(network);
        network->retry_times = 1;
        network->attach_timer = ur_start_timer(SID_REQUEST_TIMEOUT,
                                        handle_attach_timer, network);
    }
    return UR_ERROR_NONE;
}

static ur_error_t send_network_data_request(network_context_t *network,
                                            neighbor_t *node)
{
    ur_error_t           error = UR_ERROR_NONE;
    mm_header_t          *mm_header;
    mm_tlv_request_tlv_t *request_tlvs;
    message_t            *message;
    uint8_t              *data;
    uint16_t             length;
    uint8_t              tlv_types[2] = {TYPE_VERSION, TYPE_MCAST_ADDR};
    mesh_dest_t          *dest;

    if (g_mm_state.device.state == DEVICE_STATE_LEADER || node == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_tlv_request_tlv_t) + sizeof(tlv_types);
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_NETWORK_DATA_REQUEST;
    data += sizeof(mm_header_t);

    request_tlvs = (mm_tlv_request_tlv_t *)data;
    mm_init_tlv_base((mm_tlv_t *)request_tlvs, TYPE_TLV_REQUEST, sizeof(tlv_types));
    data += sizeof(mm_tlv_request_tlv_t);
    memcpy(data, tlv_types, sizeof(tlv_types));
    data += sizeof(tlv_types);

    dest = message->dest;
    memcpy(&dest->dest, &node->addr, sizeof(dest->dest));
    dest->flags = INSERT_DESTNETID_FLAG;
    if (g_mm_state.device.mode & MODE_SUPER) {
        dest->meshnetid = node->netid;
    } else {
        dest->meshnetid = node->sub_netid;
    }
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send network data request, len %d\r\n", length);
    return error;
}

static ur_error_t send_network_data_response(network_context_t *network,
                                             const mac_address_t *dest,
                                             const uint8_t *tlv_types,
                                             uint8_t tlv_types_length)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    uint8_t     *data;
    message_t   *message;
    int16_t     length;
    neighbor_t  *node;
    mesh_dest_t *mesh_dest;

    node = get_neighbor_by_mac_addr(network->hal, dest->addr);
    if (node == NULL) {
        return UR_ERROR_FAIL;
    }

    length = tlvs_calc_length(tlv_types, tlv_types_length);
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
    mm_header->command = COMMAND_NETWORK_DATA_RESPONSE;
    data += sizeof(mm_header_t);
    data += tlvs_set_value(data, tlv_types, tlv_types_length);
    mesh_dest = message->dest;
    memcpy(&mesh_dest->dest, dest, sizeof(mesh_dest->dest));
    mesh_dest->flags = INSERT_DESTNETID_FLAG;
    mesh_dest->meshnetid = mm_get_meshnetid(network);
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send network data response, len %d\r\n", length);
    return error;
}

static ur_error_t handle_network_data_request(uint8_t *payload, uint16_t length,
                                              const mesh_src_t *src,
                                              const mac_address_t *dest)
{
    mm_tlv_request_tlv_t *tlvs_request;
    uint8_t              *tlvs;
    uint16_t             tlvs_length;
    network_context_t    *network;

    network = src->dest_network;
    if (g_mm_state.device.state < DEVICE_STATE_LEADER ||
        network->attach_candidate) {
        return UR_ERROR_FAIL;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle network data request\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);
    if ((tlvs_request = (mm_tlv_request_tlv_t *)mm_get_tv(tlvs, tlvs_length,
                                                          TYPE_TLV_REQUEST)) == NULL) {
        return UR_ERROR_FAIL;
    }

    return send_network_data_response(network, &src->src, tlvs + sizeof(mm_tlv_t),
                                      ((mm_tlv_t *)tlvs)->length);
}

static ur_error_t handle_network_data_response(uint8_t *payload,
                                               uint16_t length,
                                               const mesh_src_t *src,
                                               const mac_address_t *dest)
{
    mm_version_tv_t       *version;
    mm_mcast_addr_tv_t    *mcast;
    uint8_t               *tlvs;
    uint16_t              tlvs_length;
    stable_network_data_t stable_network_data;
    network_context_t     *network = src->dest_network;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle network data response\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    version = (mm_version_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_VERSION);
    if (version == NULL) {
        return UR_ERROR_FAIL;
    }
    stable_network_data.main_version =
                (version->version & STABLE_MAIN_VERSION_MASK) >> STABLE_MAIN_VERSION_OFFSET;
    stable_network_data.minor_version = version->version & STABLE_MINOR_VERSION_MASK;

    if (src->meshnetid != network->meshnetid) {
        become_detached();
        return UR_ERROR_FAIL;
    }
    stable_network_data.meshnetid = get_main_netid(src->meshnetid);

    mcast = (mm_mcast_addr_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_MCAST_ADDR);
    if (mcast == NULL) {
        return UR_ERROR_FAIL;
    }
    memcpy(stable_network_data.mcast_addr, &mcast->mcast, sizeof(mcast->mcast));
    nd_stable_set(&stable_network_data);

    send_advertisement(network);
    if (network->advertisement_timer == NULL) {
        network->advertisement_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
                                                      handle_advertisement_timer, network);
    }
    network->state = INTERFACE_UP;
    g_mm_state.callback->interface_up();
    start_keep_alive_timer(network);
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
    mm_sid_type_tv_t *sid_type;
    message_t    *message;
    uint8_t      *data;
    uint16_t     length;
    mesh_dest_t  *dest;

    if (network == NULL || network->attach_candidate == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_node_id_tv_t) + sizeof(mm_ueid_tv_t) +
             sizeof(mm_mode_tv_t) + sizeof(mm_sid_type_tv_t);
    if (network->sid != INVALID_SID &&
        network->attach_candidate->netid == network->meshnetid) {
        length += sizeof(mm_sid_tv_t);
    }
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_SID_REQUEST;
    data += sizeof(mm_header_t);

    attach_node_id = (mm_node_id_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)attach_node_id, TYPE_ATTACH_NODE_ID);
    attach_node_id->sid = network->attach_candidate->sid;
    attach_node_id->mode = g_mm_state.device.mode;
    if (g_mm_state.device.mode & MODE_SUPER) {
        attach_node_id->meshnetid = network->attach_candidate->netid;
    } else {
        attach_node_id->meshnetid = network->attach_candidate->sub_netid;
    }
    data += sizeof(mm_node_id_tv_t);

    sid_type = (mm_sid_type_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)sid_type, TYPE_SID_TYPE);
    sid_type->type = network->router->sid_type;
    data += sizeof(mm_sid_type_tv_t);

    src_ueid = (mm_ueid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)src_ueid, TYPE_SRC_UEID);
    memcpy(src_ueid->ueid, g_mm_state.device.ueid, sizeof(src_ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    mode = (mm_mode_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)mode, TYPE_MODE);
    mode->mode = (uint8_t)g_mm_state.device.mode;
    data += sizeof(mm_mode_tv_t);

    if (network->sid != INVALID_SID &&
        network->attach_candidate->netid == network->meshnetid) {
        src_sid = (mm_sid_tv_t *)data;
        mm_init_tv_base((mm_tv_t *)src_sid, TYPE_SRC_SID);
        src_sid->sid = network->sid;
        data += sizeof(mm_sid_tv_t);
    }

    dest = message->dest;
    dest->flags = INSERT_DESTNETID_FLAG;
    if ((g_mm_state.device.mode & MODE_MOBILE) ||
        (network->router->sid_type != STRUCTURED_SID)) {
        dest->dest.len = 2;
        dest->dest.short_addr = LEADER_SID;
        dest->meshnetid = get_main_netid(network->attach_candidate->netid);
    } else {
        memcpy(&dest->dest, &network->attach_candidate->addr, sizeof(dest->dest));
        if (g_mm_state.device.mode & MODE_SUPER) {
            dest->meshnetid = network->attach_candidate->netid;
        } else {
            dest->meshnetid = network->attach_candidate->sub_netid;
        }
    }

    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send sid request, len %d\r\n", length);
    return error;
}

static ur_error_t send_sid_response(network_context_t *network,
                                    uint16_t alloc_sid, uint8_t alloc_node_type,
                                    ur_node_id_t *relayer, uint8_t *target_ueid)
{
    ur_error_t        error = UR_ERROR_NONE;
    mm_header_t       *mm_header;
    mm_sid_tv_t       *allocated_sid;
    mm_node_type_tv_t *allocated_node_type;
    mm_ueid_tv_t      *dest_ueid;
    mm_netinfo_tv_t   *netinfo;
    uint8_t           *data;
    message_t         *message;
    uint16_t          length;
    mesh_dest_t       *dest;
    network_context_t *default_network;

    if (network == NULL) {
        return UR_ERROR_FAIL;
    }

    length = sizeof(mm_header_t) + sizeof(mm_sid_tv_t) +
             sizeof(mm_node_type_tv_t) + sizeof(mm_ueid_tv_t) +
             sizeof(mm_netinfo_tv_t);
    message = message_alloc(network, length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_SID_RESPONSE;
    data += sizeof(mm_header_t);

    allocated_sid = (mm_sid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)allocated_sid, TYPE_ALLOCATE_SID);
    allocated_sid->sid = alloc_sid;
    data += sizeof(mm_sid_tv_t);

    allocated_node_type = (mm_node_type_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)allocated_node_type, TYPE_NODE_TYPE);
    allocated_node_type->type = alloc_node_type;
    data += sizeof(mm_node_type_tv_t);

    dest_ueid = (mm_ueid_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)dest_ueid, TYPE_DEST_UEID);
    memcpy(dest_ueid->ueid, target_ueid, sizeof(dest_ueid->ueid));
    data += sizeof(mm_ueid_tv_t);

    netinfo = (mm_netinfo_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)netinfo, TYPE_NETWORK_INFO);
    netinfo->stable_version = (nd_get_stable_main_version() << STABLE_MAIN_VERSION_OFFSET) |
                               nd_get_stable_minor_version();
    netinfo->version = nd_get_version(NULL);
    netinfo->size = nd_get_meshnetsize(NULL);
    netinfo->subnet_version = nd_get_version(network);
    netinfo->subnet_size_1  = (uint8_t)((nd_get_meshnetsize(network) >> 8) & 0x7);
    netinfo->subnet_size_2  = (uint8_t)(nd_get_meshnetsize(network) & 0xff);
    default_network = get_default_network_context();
    netinfo->meshnetid = default_network->meshnetid;
    default_network = get_sub_network_context(default_network->hal);
    if (default_network) {
        netinfo->sub_meshnetid = get_sub_netid(default_network->meshnetid);
    } else {
        netinfo->sub_meshnetid = get_sub_netid(netinfo->meshnetid);
    }
    data += sizeof(mm_netinfo_tv_t);

    dest = message->dest;
    dest->dest.len = 2;
    dest->dest.short_addr = relayer->sid;
    dest->flags = INSERT_DESTNETID_FLAG;
    dest->meshnetid = relayer->meshnetid;
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send sid response %04x:%d, len %d\r\n", alloc_sid, alloc_node_type, length);
    return error;
}

static ur_error_t handle_sid_request(uint8_t *payload, uint16_t length,
                                     const mesh_src_t *src,
                                     const mac_address_t *dest)
{
    ur_error_t   error = UR_ERROR_NONE;
    mm_node_id_tv_t  *attach_node_id;
    mm_sid_tv_t  *src_sid;
    mm_ueid_tv_t *ueid;
    mm_mode_tv_t *mode;
    mm_sid_type_tv_t *sid_type;
    uint8_t      *tlvs;
    uint16_t     tlvs_length;
    sid_t        sid;
    network_context_t *network;
    uint16_t attach_sid = SUPER_ROUTER_SID;
    neighbor_t *nbr = NULL;

    network = src->dest_network;
    if (network->attach_candidate ||
        g_mm_state.device.state < DEVICE_STATE_LEADER) {
        return UR_ERROR_FAIL;
    }

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    if ((sid_type = (mm_sid_type_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_SID_TYPE)) == NULL) {
        return UR_ERROR_FAIL;
    }

    if ((attach_node_id = (mm_node_id_tv_t *)mm_get_tv(tlvs, tlvs_length,
                                                       TYPE_ATTACH_NODE_ID)) == NULL) {
        return UR_ERROR_FAIL;
    }

    src_sid = (mm_sid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_SRC_SID);

    if ((ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_SRC_UEID)) == NULL) {
        return UR_ERROR_FAIL;
    }

    if ((mode = (mm_mode_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_MODE)) == NULL) {
        return UR_ERROR_FAIL;
    }

    /* normal sid must be allocated from attach node */
    if ((mode->mode & MODE_MOBILE) == 0 && (mode->mode & MODE_SUPER) == 0 &&
        (attach_node_id && attach_node_id->sid != network->sid)) {
        ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
               "device %04x state %s get an unexpected sid request from %04x\n",
               network->sid, state2str(g_mm_state.device.state), attach_node_id->sid);
        return UR_ERROR_FAIL;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle sid request\r\n");

    sid.sid = INVALID_SID;
    if (src_sid) {
        sid.sid = src_sid->sid;
    }
    if (sid.sid != INVALID_SID) {
        nbr = get_neighbor_by_sid(network->hal, sid.sid, src->meshnetid);
        if (nbr) {
            memcpy(nbr->ueid, ueid->ueid, sizeof(nbr->ueid));
        }
    }
    switch (sid_type->type) {
        case STRUCTURED_SID:
            if (attach_node_id->mode & MODE_SUPER) {
                attach_sid = SUPER_ROUTER_SID;
            } else {
                attach_sid = attach_node_id->sid;
            }
            error = allocate_sid(ueid->ueid, attach_sid, mode->mode, &sid);
            break;
        case SHORT_RANDOM_SID:
        case RANDOM_SID:
            error = rsid_allocate_sid(ueid->ueid, sid_type->type, &sid);
            break;
        default:
            error = UR_ERROR_PARSE;
    }
    if (error == UR_ERROR_NONE) {
        neighbor_t *node;
        ur_node_id_t relayer;
        relayer.sid = attach_node_id->sid;
        relayer.meshnetid = attach_node_id->meshnetid;
        network = get_network_context_by_meshnetid(relayer.meshnetid);
        if (network == NULL) {
            network = get_default_network_context();
        }
        error = send_sid_response(network, sid.sid, sid.type, &relayer, ueid->ueid);
        node = get_neighbor_by_ueid(ueid->ueid);
        if (node) {
            node->sid = INVALID_SID;
        }
    }
    return error;
}

static ur_error_t handle_sid_response(uint8_t *payload, uint16_t length,
                                      const mesh_src_t *src,
                                      const mac_address_t *dest)
{
    ur_error_t        error = UR_ERROR_NONE;
    mm_sid_tv_t       *allocated_sid = NULL;
    mm_node_type_tv_t *allocated_node_type = NULL;
    mm_ueid_tv_t      *dest_ueid = NULL;
    mm_netinfo_tv_t   *netinfo;
    uint8_t           *tlvs;
    uint16_t          tlvs_length;
    neighbor_t        *node;
    network_context_t *network = src->dest_network;
    network_context_t *default_network;
    slist_t           *networks;
    uint8_t           index = 0;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle sid response\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    dest_ueid = (mm_ueid_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_DEST_UEID);
    if (dest_ueid == NULL) {
        return UR_ERROR_FAIL;
    }

    if (memcmp(dest_ueid->ueid, g_mm_state.device.ueid, sizeof(g_mm_state.device.ueid)) != 0) {
        node = get_neighbor_by_ueid(dest_ueid->ueid);
        if (node == NULL) {
            return UR_ERROR_DROP;
        }
        return message_relayer(src->dest_network, node, payload, length);
    }

    if (memcmp(g_mm_state.device.ueid, dest_ueid->ueid, sizeof(g_mm_state.device.ueid)) != 0 ||
        network->attach_candidate == NULL) {
        return UR_ERROR_DROP;
    }

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
    if (g_mm_state.device.state == DEVICE_STATE_SUPER_ROUTER) {
        allocator_init(g_mm_state.device.ueid, SUPER_ROUTER_SID);
    } else {
        allocator_init(g_mm_state.device.ueid, network->sid);
    }
    if (g_mm_state.device.state >= DEVICE_STATE_LEADER) {
        rsid_allocator_deinit();
        rsid_allocator_init();
    }
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
    if (update_network_data(network, netinfo, true)) {
        send_network_data_request(network, network->attach_node);
    } else {
        send_advertisement(network);
        if (network->advertisement_timer == NULL) {
            network->advertisement_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
                                                          handle_advertisement_timer, network);
        }
        g_mm_state.callback->interface_up();
        start_keep_alive_timer(network);
    }
    if (g_mm_state.device.state != DEVICE_STATE_SUPER_ROUTER &&
        network->attach_node->sub_netid != INVALID_NETID) {
        network->meshnetid = network->attach_node->sub_netid;
    } else {
        network->meshnetid = network->attach_node->netid;
    }
    ur_router_sid_updated(network, network->sid);
    write_prev_netinfo(network);
    send_address_notification(network, LEADER_SID);

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
            if (network->index == 0) {
                network->meshnetid = default_network->meshnetid;
            } else {
                network->meshnetid = default_network->meshnetid |
                                     generate_sub_meshnetid(network->sid, ++index);
            }
            network->path_cost = 0;
            network->change_sid = false;
            if (network->attach_node) {
                network->attach_node->state = STATE_NEIGHBOR;
                network->attach_node = NULL;
            }
            network->attach_candidate = NULL;
            ur_stop_timer(&network->attach_timer, network);

            send_advertisement(network);
            if (network->advertisement_timer == NULL) {
                network->advertisement_timer = ur_start_timer(ADVERTISEMENT_TIMEOUT,
                                                       handle_advertisement_timer, network);
            }
            ur_router_start(network);
            ur_router_sid_updated(network, LEADER_SID);
        }
    }

    return error;
}

void become_detached(void)
{
    g_mm_state.device.state = DEVICE_STATE_DETACHED;
    ur_stop_timer(&g_mm_state.device.alive_timer, NULL);
    reset_network_context();
    allocator_deinit();
    rsid_allocator_deinit();
    nd_init();
    nm_stop_discovery();
    stop_neighbor_updater();
    stop_addr_cache();

    g_mm_state.callback->interface_down();

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
        if (g_mm_state.device.mode & MODE_SUPER) {
            network->candidate_meshnetid = nbr->netid;
        } else {
            network->candidate_meshnetid = nbr->sub_netid;
        }
    }
    send_attach_request(network, nbr);
    ur_stop_timer(&network->attach_timer, network);
    network->attach_timer = ur_start_timer(ATTACH_REQUEST_TIMEOUT,
                                           handle_attach_timer, network);
    network->retry_times = 1;
    stop_neighbor_updater();
    ur_stop_timer(&network->advertisement_timer, network);

    set_master_key(NULL, 0);

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM,
           "%s node, attach start, from %04x:%04x to %04x:%x\r\n",
           state2str(g_mm_state.device.state), network->attach_node? network->attach_node->sid: 0,
           network->meshnetid, nbr?nbr->sid:0, network->candidate_meshnetid);

    return error;
}

static uint16_t compute_network_metric(uint16_t size, uint16_t path_cost)
{
    return size / SIZE_WEIGHT + path_cost / PATH_COST_WEIGHT;
}

static bool update_network_data(network_context_t *network,
                                mm_netinfo_tv_t *netinfo, bool from_same_net)
{
    int8_t         diff;
    bool           stable_updated = false;
    uint8_t        minor_version;
    network_data_t network_data;
    bool           net_changed = false;

    if (g_mm_state.device.state == DEVICE_STATE_LEADER) {
        return stable_updated;
    }

    if (!is_same_mainnet(netinfo->meshnetid, network->meshnetid)) {
        net_changed = true;
        network->meshnetid = netinfo->meshnetid;
        g_mm_state.callback->interface_down();
        nd_init();
    }

    minor_version = netinfo->stable_version & STABLE_MINOR_VERSION_MASK;
    diff = (int8_t)(minor_version - nd_get_stable_minor_version());
    if (diff > 0 || net_changed) {
        stable_updated = true;
    }

    if (from_same_net) {
        diff = (int8_t)(netinfo->version - nd_get_version(NULL));
        if (diff > 0 || net_changed) {
            network_data.version = netinfo->version;
            network_data.size = netinfo->size;
            nd_set(NULL, &network_data);
        }
        diff = (int8_t)(netinfo->subnet_version - nd_get_version(network));
        if (diff > 0 || net_changed) {
            network_data.version = netinfo->subnet_version;
            network_data.size = (netinfo->subnet_size_1 << 8) | netinfo->subnet_size_2;
            nd_set(network, &network_data);
        }
    }
    return stable_updated;
}

static void write_prev_netinfo(network_context_t *network)
{
    ur_configs_t configs;
    uint8_t      index;
    uint8_t      available_index = 0xff;

    if (network->meshnetid == INVALID_NETID ||
        network->path_cost == INFINITY_PATH_COST) {
        return;
    }

    ur_configs_read(&configs);
    for (index = 0; index < NETWORK_CONTEXT_NUM; index++) {
        if (configs.prev_netinfo[index].hal_type > MEDIA_TYPE_15_4) {
            available_index = index;
        }
        if (configs.prev_netinfo[index].hal_type == network->hal->module->type &&
            configs.prev_netinfo[index].network_index == network->index) {
            break;
        }
    }
    if (index == NETWORK_CONTEXT_NUM) {
        index = available_index;
    }
    configs.prev_netinfo[index].hal_type = network->hal->module->type;
    configs.prev_netinfo[index].network_index = network->index;
    configs.prev_netinfo[index].meshnetid = network->meshnetid;
    configs.prev_netinfo[index].path_cost = network->path_cost;
    ur_configs_write(&configs);

    network->prev_netid = network->meshnetid;
    network->prev_path_cost = network->path_cost;
}

static void read_prev_netinfo(network_context_t *network)
{
    ur_configs_t configs;
    uint8_t      index;

    ur_configs_read(&configs);
    for (index = 0; index < NETWORK_CONTEXT_NUM; index++) {
        if (configs.prev_netinfo[index].hal_type == network->hal->module->type &&
            configs.prev_netinfo[index].network_index == network->index) {
            break;
        }
    }
    network->prev_netid = INVALID_NETID;
    network->prev_path_cost = INFINITY_PATH_COST;
    if (index < NETWORK_CONTEXT_NUM) {
        network->prev_netid = configs.prev_netinfo[index].meshnetid;
        network->prev_path_cost = configs.prev_netinfo[index].path_cost;
    }
}

static void update_migrate_timeout(network_context_t *network, neighbor_t *nbr)
{
    bool     migrate = false;
    uint16_t netid;

    if (g_mm_state.device.mode & MODE_SUPER) {
        netid = nbr->netid;
    } else {
        netid = nbr->sub_netid;
    }
    if (netid == network->candidate_meshnetid ||
        network->candidate_meshnetid == BCAST_NETID) {
        network->candidate_meshnetid = netid;
        if (network->migrate_timeout == 0) {
            network->migrate_wait_timer = ur_start_timer(MIGRATE_WAIT_TIMEOUT,
                                                         handle_migrate_wait_timer, network);
        }
        network->migrate_timeout++;
        if (network->migrate_timeout >= MIGRATE_TIMEOUT) {
            migrate = true;
        }
    }

    if (migrate == false) {
        return;
    }

    if (network->router->sid_type == STRUCTURED_SID && nbr->free_slots < 1) {
        nbr = NULL;
    }
    if (nbr == NULL && g_mm_state.device.state > DEVICE_STATE_ATTACHED) {
        return;
    }

    network->migrate_timeout = 0;
    ur_stop_timer(&network->migrate_wait_timer, network);
    attach_start(network->hal, nbr);
}

static ur_error_t handle_advertisement(uint8_t *payload, uint16_t length,
                                       const mesh_src_t *src,
                                       const mac_address_t *dest)
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
    network_context_t *network = src->dest_network;
    neighbor_t        *attach_node = network->attach_node;
    bool              from_same_net = false;
    bool              from_same_core = false;
    bool              leader_reboot = false;
    uint16_t          subnet_size = 0;
    uint8_t           tlv_type;

    if (g_mm_state.device.state < DEVICE_STATE_DETACHED) {
        return UR_ERROR_NONE;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle advertisement\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    nbr = update_neighbor(src, tlvs, tlvs_length, false);
    if (nbr == NULL) {
        return UR_ERROR_NONE;
    }

    if (g_mm_state.device.state > DEVICE_STATE_ATTACHED &&
        memcmp(nbr->ueid, INVALID_UEID, sizeof(nbr->ueid)) == 0) {
        tlv_type = TYPE_TARGET_UEID;
        send_link_request(network, &nbr->addr, &tlv_type, 1);
    }

    if (network != get_default_network_context()) {
        return UR_ERROR_NONE;
    }

    netinfo = (mm_netinfo_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_NETWORK_INFO);
    path_cost = (mm_cost_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_PATH_COST);
    mode = (mm_mode_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_MODE);
    if (netinfo == NULL || netinfo->meshnetid == BCAST_NETID ||
        path_cost == NULL || mode == NULL) {
        return UR_ERROR_FAIL;
    }

    // detached node try to migrate
    if (g_mm_state.device.state == DEVICE_STATE_DETACHED) {
        update_migrate_timeout(network, nbr);
        return UR_ERROR_NONE;
    }

    // mode super should be leader
    if ((mode->mode & MODE_SUPER) && (g_mm_state.device.mode & MODE_SUPER) == 0 &&
         g_mm_state.device.state == DEVICE_STATE_LEADER) {
        become_detached();
        update_migrate_timeout(network, nbr);
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
    // not process from different net type
    if ((is_subnet(network->meshnetid) && is_subnet(src->meshnetid) == 0) ||
        (is_subnet(network->meshnetid) == 0 && is_subnet(src->meshnetid))) {
        return UR_ERROR_NONE;
    }
    // leader not try to migrate to the same net
    if (network->meshnetid == src->meshnetid) {
        from_same_net = true;
    } else if (is_same_mainnet(network->meshnetid, src->meshnetid)) {
        from_same_core = true;
    }
    if (from_same_net &&
        (attach_node == NULL || g_mm_state.device.state == DEVICE_STATE_LEADER)) {
        return UR_ERROR_NONE;
    }

    if (from_same_net) {
        if (update_network_data(network, netinfo, from_same_net)) {
            send_network_data_request(src->dest_network, nbr);
        }
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
            network->attach_timer = ur_start_timer(SID_REQUEST_TIMEOUT,
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
        if ((nbr->netid == network->prev_netid) &&
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
                (new_metric == cur_metric && src->meshnetid <= network->meshnetid)) {
                return UR_ERROR_NONE;
            }
        }
    }
    update_migrate_timeout(network, nbr);

    return UR_ERROR_NONE;
}

ur_error_t mm_handle_frame_received(uint8_t *payload, uint16_t length,
                                    const mesh_src_t *src,
                                    const mac_address_t *dest)
{
    ur_error_t    error = UR_ERROR_NONE;
    mm_header_t   *mm_header;
    mm_header = (mm_header_t *)payload;

    switch (mm_header->command & COMMAND_COMMAND_MASK) {
        case COMMAND_ADVERTISEMENT:
            handle_advertisement(payload, length, src, dest);
            break;
        case COMMAND_DISCOVERY_REQUEST:
            handle_discovery_request(payload, length, src, dest);
            break;
        case COMMAND_DISCOVERY_RESPONSE:
            handle_discovery_response(payload, length, src, dest);
            break;
        case COMMAND_DATA_REQUEST:
            break;
        case COMMAND_DATA_RESPONSE:
            break;
        case COMMAND_ATTACH_REQUEST:
            error = handle_attach_request(payload, length, src, dest);
            break;
        case COMMAND_ATTACH_RESPONSE:
            error = handle_attach_response(payload, length, src, dest);
            break;
        case COMMAND_NETWORK_DATA_REQUEST:
            error = handle_network_data_request(payload, length, src, dest);
            break;
        case COMMAND_NETWORK_DATA_RESPONSE:
            error = handle_network_data_response(payload, length, src, dest);
            break;
        case COMMAND_SID_REQUEST:
            error = handle_sid_request(payload, length, src, dest);
            break;
        case COMMAND_SID_RESPONSE:
            error = handle_sid_response(payload, length, src, dest);
            break;
        case COMMAND_ADDRESS_QUERY:
            error = handle_address_query(payload, length, src, dest);
            break;
        case COMMAND_ADDRESS_QUERY_RESPONSE:
            error = handle_address_query_response(payload, length, src, dest);
            break;
        case COMMAND_ADDRESS_NOTIFICATION:
            error = handle_address_notification(payload, length, src, dest);
            break;
        case COMMAND_LINK_REQUEST:
            error = handle_link_request(payload, length, src, dest);
            break;
        case COMMAND_LINK_ACCEPT:
            error = handle_link_accept(payload, length, src, dest);
            break;
        case COMMAND_LINK_ACCEPT_AND_REQUEST:
            error = handle_link_accept_and_request(payload, length, src, dest);
            break;
        case COMMAND_LINK_REJECT:
            error = handle_link_reject(payload, length, src, dest);
            break;
        case COMMAND_DEST_UNREACHABLE:
            error = handle_dest_unreachable(payload, length, src, dest);
            break;
        case COMMAND_ADDRESS_ERROR:
            error = handle_address_error(payload, length, src, dest);
            break;
        case COMMAND_ROUTING_INFO_UPDATE:
            error = handle_router_message_received(payload, length, src, dest);
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
    g_mm_state.device.mode = MODE_RX_ON;
    g_mm_state.device.reboot_flag = true;

    g_mm_state.device.seclevel = SEC_LEVEL_0;

    memset(&g_mm_state.network_data_updater, 0 , sizeof(g_mm_state.network_data_updater));
    g_mm_state.network_data_updater.handler = network_data_update_handler;
    nd_register_update_handler(&g_mm_state.network_data_updater);

    register_neighbor_updater(neighbor_updated_handler);

    memset(&configs, 0, sizeof(configs));
    ur_configs_read(&configs);
    nd_set_stable_main_version(configs.main_version);
    return error;
}

ur_error_t mm_deinit(void)
{
    nd_unregister_update_handler(&g_mm_state.network_data_updater);
    return UR_ERROR_NONE;
}

ur_error_t mm_start(mm_cb_t *mm_cb)
{
    slist_t           *networks;
    network_context_t *network;

    assert(mm_cb);

    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM, "ur started\r\n");

    reset_network_context();
    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        read_prev_netinfo(network);
    }
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
    meshnetid_t hal_meshnetid;

    network = network ? : get_default_network_context();
    nd_set_stable_meshnetid(meshnetid);
    network->meshnetid = meshnetid;
    memset(&hal_meshnetid, 0, sizeof(meshnetid_t));
    hal_meshnetid.len = MESHNETID_SIZE;
    hal_meshnetid.meshnetid[0] = (uint8_t)(meshnetid & 0xff);
    hal_meshnetid.meshnetid[1] = (uint8_t)((meshnetid >> 8) & 0xff);
    //hal_ur_set_meshnetid(NULL, &hal_meshnetid);

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
        return INVALID_NETID;
    }

    if (network->attach_state == ATTACH_IDLE || network->attach_state == ATTACH_DONE) {
        meshnetid = network->meshnetid;
    } else if (network->attach_candidate) {
        if (g_mm_state.device.mode & MODE_SUPER) {
            meshnetid = network->attach_candidate->netid;
        } else {
            meshnetid = network->attach_candidate->sub_netid;
        }
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
        network->attach_timer = ur_start_timer(SID_REQUEST_TIMEOUT,
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

ur_error_t mm_set_seclevel(uint8_t level)
{
    if (level >= SEC_LEVEL_0 && level <= SEC_LEVEL_1) {
        g_mm_state.device.seclevel = level;
        return UR_ERROR_NONE;
    }

    return UR_ERROR_FAIL;
}

uint8_t mm_get_seclevel(void)
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

const uint16_t mm_get_channel(network_context_t *network)
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
