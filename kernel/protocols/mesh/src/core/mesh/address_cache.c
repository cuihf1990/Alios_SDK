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

#include "core/address_cache.h"
#include "core/network_data.h"
#include "utilities/logging.h"
#include "utilities/timer.h"
#include "hal/interfaces.h"
#include "hal/hals.h"

typedef struct address_cache_state_s {
    slist_t    cache_list;
    uint16_t   cache_num;
    ur_timer_t timer;
} address_cache_state_t;

static address_cache_state_t g_ac_state;

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
            } else if ((network = get_network_context_by_meshnetid(node->node_id.meshnetid)) != NULL) {
                if (network->router->sid_type == SHORT_RANDOM_SID ||
                    network->router->sid_type == RANDOM_SID) {
                    rsid_free_sid(network, &node->node_id);
                }
            }
            slist_del(&node->next, &g_ac_state.cache_list);
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
        if (node->node_id.sid == target->sid && node->node_id.meshnetid == target->meshnetid) {
            memcpy(target->ueid, node->node_id.ueid, sizeof(target->ueid));
            break;
        }
    }
    if (node) {
        attach->sid = node->node_id.attach_sid;
        attach->meshnetid = node->node_id.meshnetid;
    }
}

void get_attach_by_addr(ur_node_id_t *attach, ur_addr_t *target)
{
    sid_node_t *node;

    if (attach == NULL || target == NULL) {
        return;
    }
    attach->sid = INVALID_SID;
    attach->meshnetid = INVALID_NETID;
    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        if (node->node_id.sid == target->addr.short_addr &&
            node->node_id.meshnetid == target->netid) {
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

    if (memcmp(ueid, mm_get_local_ueid(), 8) == 0) {
        node_id->sid = mm_get_local_sid();
        network = get_default_network_context();
        node_id->meshnetid = mm_get_meshnetid(network);
        memcpy(node_id->ueid, mm_get_local_ueid(), sizeof(node_id->ueid));
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
