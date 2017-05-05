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

#include "address_cache.h"
#include "mesh_types.h"
#include "interfaces.h"
#include "network_data.h"
#include "logging.h"
#include "timer.h"
#include "hals.h"

typedef struct address_cache_state_s {
    slist_t    cache_list;
    uint16_t   cache_num;
    ur_timer_t timer;
} address_cache_state_t;

static address_cache_state_t g_ac_state;

static void handle_addr_cache_timer(void *args)
{
    sid_node_t *node;
    uint8_t timeout;
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

        node->timeout++;
        if (node->timeout > timeout) {
            if (is_partial_function_sid(node->sid)) {
                update_sid_mapping(node->ueid, node->sid, node->attach_sid, false);
            } else if ((network = get_network_context_by_meshnetid(node->meshnetid)) != NULL) {
                if (network->router->sid_type == SHORT_RANDOM_SID ||
                    network->router->sid_type == RANDOM_SID) {
                    rsid_free_sid(node->ueid, network->router->sid_type, node->sid);
                } else if (network->router->sid_type == STRUCTURED_SID) {
                    update_sid_mapping(node->ueid, node->sid, node->attach_sid, false);
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
    sid_node_t *node;
    sid_node_t *new_node = NULL;

    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        if (memcmp(node->ueid, target->ueid, sizeof(node->ueid)) == 0) {
            new_node = node;
            break;
        }
    }

    if (new_node == NULL) {
        new_node = (sid_node_t *)ur_mem_alloc(sizeof(sid_node_t));
        if (!new_node)
            return UR_ERROR_MEM;
        slist_add(&new_node->next, &g_ac_state.cache_list);
        g_ac_state.cache_num++;
        nd_set_meshnetsize(NULL, g_ac_state.cache_num + 1);
    }

    memcpy(new_node->ueid, target->ueid, sizeof(new_node->ueid));
    new_node->sid = target->sid;
    new_node->meshnetid = target->meshnetid;
    new_node->attach_sid = attach->sid;
    new_node->attach_netid = attach->meshnetid;
    new_node->type = type;
    new_node->timeout = 0;

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "update_address_cache, ueid %x, sid %x, netid %x, attach_sid %x, attach_netid %x\r\n",
           new_node->ueid[0], new_node->sid, new_node->meshnetid, new_node->attach_sid, new_node->attach_netid);
    return UR_ERROR_NONE;
}

void get_attach_by_nodeid(ur_node_id_t *attach, ur_node_id_t *target)
{
    sid_node_t *node;

    if (attach == NULL || target == NULL) {
        return;
    }
    attach->sid = INVALID_SID;
    attach->meshnetid = INVALID_NETID;
    slist_for_each_entry(&g_ac_state.cache_list, node, sid_node_t, next) {
        if (node->sid == target->sid && node->meshnetid == target->meshnetid) {
            memcpy(target->ueid, node->ueid, sizeof(target->ueid));
            break;
        }
    }
    if (node) {
        attach->sid = node->attach_sid;
        attach->meshnetid = node->attach_netid;
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
        if (memcmp(node->ueid, ueid, sizeof(node_id->ueid)) == 0) {
            node_id->sid = node->sid;
            node_id->meshnetid = node->meshnetid;
            memcpy(node_id->ueid, node->ueid, sizeof(node_id->ueid));
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
