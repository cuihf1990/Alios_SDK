/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include <string.h>

#include "core/sid_allocator.h"
#include "core/router_mgr.h"
#include "core/mesh_mgmt.h"
#include "umesh_utils.h"
#include "hal/interfaces.h"
#include "hal/interface_context.h"
#include "hal/hals.h"

AOS_SLIST_HEAD(g_networks_list);
AOS_SLIST_HEAD(g_hals_list);

static network_context_t *new_network_context(hal_context_t *hal, uint8_t index,
                                              int router_id)
{
    network_context_t *network;

    network = (network_context_t *)ur_mem_alloc(sizeof(network_context_t));
    if (network == NULL) {
        return network;
    }
    memset(network, 0, sizeof(network_context_t));
    network->index = index;
    network->hal = hal;
    network->sid = INVALID_SID;

    network->mcast_sequence = 0;
    memset(network->mcast_entry, 0, sizeof(network->mcast_entry));
    network->router = ur_get_router_by_id(router_id);
    network->router->network = network;
    if (index == 0) {
        ur_router_set_default_router(router_id);
    }

    network->one_time_key = ur_mem_alloc(KEY_SIZE);
    if (network->one_time_key == NULL) {
        ur_mem_free(network, sizeof(network_context_t));
        return NULL;
    }

    slist_add_tail(&network->next, &g_networks_list);

    return network;
}

static hal_context_t *new_hal_context(umesh_hal_module_t *module)
{
    hal_context_t *hal;
    int i;

    hal = (hal_context_t *)ur_mem_alloc(sizeof(hal_context_t));
    if (hal == NULL) {
        return NULL;
    }
    memset(hal, 0, sizeof(hal_context_t));
    hal->module = module;
    slist_add_tail(&hal->next, &g_hals_list);

    for (i = 0; i < QUEUE_SIZE; i++) {
        dlist_init(&hal->send_queue[i]);
    }

    dlist_init(&hal->recv_queue);

    return hal;
}

void interface_init(void)
{
    umesh_hal_module_t *module;
    hal_context_t        *hal_context;
    int16_t              mtu;

    module = hal_umesh_get_default_module();
    while (module) {
        if (module->type <= MEDIA_TYPE_15_4) {
            hal_context = new_hal_context(module);
            hal_context->channel_list.num =
                hal_umesh_get_chnlist(module, &hal_context->channel_list.channels);
            memcpy(&hal_context->mac_addr, hal_umesh_get_mac_address(module),
                   sizeof(hal_context->mac_addr));

            // mesh forwarder
            mtu = hal_umesh_get_bcast_mtu(module);
            if (mtu < 0) {
                mtu = 127;
            }
            if (hal_umesh_get_ucast_mtu(module) > mtu) {
                mtu = hal_umesh_get_ucast_mtu(module);
            }
            hal_context->frame.data = (uint8_t *)ur_mem_alloc(mtu);
            memset(hal_context->frame.data, 0 , mtu);
            memset(&hal_context->link_stats, 0, sizeof(hal_context->link_stats));

            if (module->type == MEDIA_TYPE_WIFI) {
                hal_context->def_channel = 6;
                hal_context->discovery_interval = WIFI_DISCOVERY_TIMEOUT;
                hal_context->attach_request_interval = WIFI_ATTACH_REQUEST_TIMEOUT;
                hal_context->sid_request_interval = WIFI_SID_REQUEST_TIMEOUT;
                hal_context->link_request_interval = WIFI_LINK_REQUEST_TIMEOUT;
                hal_context->link_request_mobile_interval = WIFI_LINK_REQUEST_MOBILE_TIMEOUT;
                hal_context->neighbor_alive_interval = WIFI_NEIGHBOR_ALIVE_TIMEOUT;
                hal_context->advertisement_interval = WIFI_ADVERTISEMENT_TIMEOUT;
            } else if (module->type == MEDIA_TYPE_BLE) {
                hal_context->def_channel = hal_context->channel_list.channels[0];
                hal_context->discovery_interval = BLE_DISCOVERY_TIMEOUT;
                hal_context->attach_request_interval = BLE_ATTACH_REQUEST_TIMEOUT;
                hal_context->sid_request_interval = BLE_SID_REQUEST_TIMEOUT;
                hal_context->link_request_interval = BLE_LINK_REQUEST_TIMEOUT;
                hal_context->link_request_mobile_interval = BLE_LINK_REQUEST_MOBILE_TIMEOUT;
                hal_context->neighbor_alive_interval = BLE_NEIGHBOR_ALIVE_TIMEOUT;
                hal_context->advertisement_interval = BLE_ADVERTISEMENT_TIMEOUT;
            } else if (module->type == MEDIA_TYPE_15_4) {
                hal_context->def_channel = hal_context->channel_list.channels[0];
                hal_context->discovery_interval = IEEE154_DISCOVERY_TIMEOUT;
                hal_context->attach_request_interval = IEEE154_ATTACH_REQUEST_TIMEOUT;
                hal_context->sid_request_interval = IEEE154_SID_REQUEST_TIMEOUT;
                hal_context->link_request_interval = IEEE154_LINK_REQUEST_TIMEOUT;
                hal_context->link_request_mobile_interval = IEEE154_LINK_REQUEST_MOBILE_TIMEOUT;
                hal_context->neighbor_alive_interval = IEEE154_NEIGHBOR_ALIVE_TIMEOUT;
                hal_context->advertisement_interval = IEEE154_ADVERTISEMENT_TIMEOUT;
            }
        }

        module = hal_umesh_get_next_module(module);

#ifndef CONFIG_AOS_MESH_SUPER
        break;
#endif
    }
}

static void set_network_configs(network_context_t *network)
{
    switch (network->hal->module->type) {
        case MEDIA_TYPE_WIFI:
            network->migrate_interval = WIFI_MIGRATE_WAIT_TIMEOUT;
            network->notification_interval = WIFI_NOTIFICATION_TIMEOUT;
            network->net_scan_interval = WIFI_NET_SCAN_TIMEOUT;
            break;
        case MEDIA_TYPE_BLE:
            network->migrate_interval = BLE_MIGRATE_WAIT_TIMEOUT;
            network->notification_interval = BLE_NOTIFICATION_TIMEOUT;
            network->net_scan_interval = BLE_NET_SCAN_TIMEOUT;
            break;
        case MEDIA_TYPE_15_4:
            network->migrate_interval = IEEE154_MIGRATE_WAIT_TIMEOUT;
            network->notification_interval = IEEE154_NOTIFICATION_TIMEOUT;
            network->net_scan_interval = IEEE154_NET_SCAN_TIMEOUT;
            break;
        default:
            break;
    }
}

void interface_start(void)
{
    hal_context_t *hal;
    network_context_t *network;
    uint8_t index;

    index = 0;
    slist_for_each_entry(&g_hals_list, hal, hal_context_t, next) {
        bool is_wifi = hal->module->type == MEDIA_TYPE_WIFI;

        if (is_wifi) {
            if (umesh_mm_get_mode() & MODE_SUPER) {
                network = new_network_context(hal, index++, VECTOR_ROUTER);
                set_network_configs(network);

                network = new_network_context(hal, index++, SID_ROUTER);
                set_network_configs(network);
            } else {
                network = new_network_context(hal, index++, SID_ROUTER);
                set_network_configs(network);
            }
        } else {
            network = new_network_context(hal, index++, SID_ROUTER);
            set_network_configs(network);
        }
    }
}

static void cleanup_one_queue(message_queue_t *queue)
{
    message_t *message;

    while ((message = message_queue_get_head(queue))) {
        message_queue_dequeue(message);
        message_free(message);
    }
}

static void cleanup_queues(hal_context_t *hal)
{
    int i;

    for (i = 0; i < QUEUE_SIZE; i++) {
        cleanup_one_queue(&hal->send_queue[i]);
    }

    cleanup_one_queue(&hal->recv_queue);
}

void interface_stop(void)
{
    hal_context_t *hal;

    reset_network_context();

    slist_for_each_entry(&g_hals_list, hal, hal_context_t, next) {
        cleanup_queues(hal);
        hal->send_message = NULL;
    }

    while (!slist_empty(&g_networks_list)) {
        network_context_t *network;
        network = slist_first_entry(&g_networks_list, network_context_t, next);
        ur_mem_free(network->one_time_key, KEY_SIZE);
        slist_del(&network->next, &g_networks_list);
        ur_mem_free(network, sizeof(*network));
    }
}

void interface_deinit(void)
{
    hal_context_t *hal;
    int16_t mtu;

    while (!slist_empty(&g_hals_list)) {
        hal = slist_first_entry(&g_hals_list, hal_context_t, next);
        slist_del(&hal->next, &g_hals_list);

        mtu = hal_umesh_get_bcast_mtu(hal->module);
        if (mtu < 0) {
            mtu = 127;
        }
        if (hal_umesh_get_ucast_mtu(hal->module) > mtu) {
            mtu = hal_umesh_get_ucast_mtu(hal->module);
        }
        ur_mem_free(hal->frame.data, mtu);
        hal->frame.data = NULL;

        ur_mem_free(hal, sizeof(*hal));
    }
}

void reset_network_context(void)
{
    slist_t           *networks;
    network_context_t *network;

    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        network->state = INTERFACE_DOWN;
        network->attach_state = ATTACH_IDLE;
        ur_stop_timer(&network->advertisement_timer, network);
        ur_stop_timer(&network->attach_timer, network);
        ur_stop_timer(&network->migrate_wait_timer, network);
        ur_stop_timer(&network->mcast_timer, network);
        network->sid       = BCAST_SID;
        network->path_cost = INFINITY_PATH_COST;
        network->meshnetid = INVALID_NETID;
        if (network->attach_node != NULL &&
            network->attach_node->state == STATE_PARENT) {
            network->attach_node->state = STATE_NEIGHBOR;
        }
        network->attach_node = NULL;
        network->attach_candidate = NULL;
        network->candidate_meshnetid = BCAST_NETID;
        network->migrate_times = 0;
        network->channel = -1;
    }
}

slist_t *get_network_contexts(void)
{
    return &g_networks_list;
}

network_context_t *get_default_network_context(void)
{
    return slist_first_entry(&g_networks_list, network_context_t, next);
}

network_context_t *get_sub_network_context(hal_context_t *hal)
{
    network_context_t *network = get_default_network_context();

    if (slist_entry_number(&g_networks_list) < 2) {
        return network;
    }
    slist_for_each_entry(&g_networks_list, network, network_context_t, next) {
        if (network->hal == hal) {
            break;
        }
    }
    if (network->hal->module->type == MEDIA_TYPE_WIFI) {
        return slist_first_entry(&network->next, network_context_t, next);
    } else {
        return network;
    }
}

network_context_t *get_hal_default_network_context(hal_context_t *hal)
{
    network_context_t *network = NULL;

    slist_for_each_entry(&g_networks_list, network, network_context_t, next) {
        if (network->hal == hal) {
            break;
        }
    }

    return network;
}

network_context_t *get_network_context_by_meshnetid(uint16_t meshnetid)
{
    network_context_t *network = NULL;

    slist_for_each_entry(&g_networks_list, network, network_context_t, next) {
        if (network->meshnetid == meshnetid) {
            break;
        }
    }
    return network;
}

slist_t *get_hal_contexts(void)
{
    return &g_hals_list;
}

uint8_t get_hal_contexts_num(void)
{
    return slist_entry_number(&g_hals_list);
}

hal_context_t *get_default_hal_context(void)
{
    return slist_first_entry(&g_hals_list, hal_context_t, next);
}

hal_context_t *get_hal_context(media_type_t type)
{
    hal_context_t *hal = NULL;

    slist_for_each_entry(&g_hals_list, hal, hal_context_t, next) {
        if (hal->module->type == type) {
            break;
        }
    }
    return hal;
}
