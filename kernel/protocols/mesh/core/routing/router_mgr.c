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

#include "router_mgr.h"
#include "mesh_mgmt.h"
#include "logging.h"

#ifndef DEFAULT_ROUTER
#define DEFAULT_ROUTER SID_ROUTER
#endif

typedef struct router_mgmr_state_s {
    slist_t  router_list;
    router_t *default_router;
} router_mgmr_state_t;

static router_mgmr_state_t g_rm_state;

extern neighbor_t *get_neighbor_by_sid(hal_context_t *hal, uint16_t sid,
                                       uint16_t meshnetid);
extern void sid_router_register(void);
extern void vector_router_register(void);

void ur_router_register_module(void)
{
    router_t *router;

    slist_init(&g_rm_state.router_list);
    sid_router_register();
    vector_router_register();

    /* enable the default router */
    slist_for_each_entry(&g_rm_state.router_list, router, router_t, next) {
        if (router->id == DEFAULT_ROUTER) {
            g_rm_state.default_router = router;
            g_rm_state.default_router->cb.start();
            break;
        }
    }
}

ur_error_t ur_router_start(network_context_t *network)
{
    if (network && network->router) {
        network->router->cb.start();
    }
    else if (g_rm_state.default_router != NULL) {
       g_rm_state.default_router->cb.start();
    }
    return UR_ERROR_NONE;
}

void ur_router_stop(void)
{
    router_t *router;

    slist_for_each_entry(&g_rm_state.router_list, router, router_t, next) {
        if (router->cb.stop) {
            router->cb.stop();
        }
    }
}

uint16_t ur_router_get_next_hop(network_context_t *network, uint16_t dest_sid)
{
    uint16_t  next_hop = INVALID_SID;

    if (network == NULL) {
        network = g_rm_state.default_router->network;
    }
    if (network == NULL) {
        return INVALID_SID;
    }

    if (get_neighbor_by_sid(network->hal, dest_sid, mm_get_meshnetid(network)) != NULL) {
        return dest_sid;
    }

    if (network->router != NULL) {
        next_hop = network->router->cb.get_next_hop_sid(dest_sid);
    } else if (g_rm_state.default_router != NULL) {
        next_hop = g_rm_state.default_router->cb.get_next_hop_sid(dest_sid);
    }

    return next_hop;
}

void ur_router_sid_updated(network_context_t *network, uint16_t sid)
{
    router_t *router;
    netids_t netids;

    router = network->router;
    if (router != NULL) {
        uint8_t i = 0, call = 0;
        for (i = 0; i < router->events.num; i++) {
            if (router->events.events[i] == EVENT_SID_UPDATED) {
                call = 1;
                break;
            }
        }

        if(call > 0 && router->cb.handle_subscribe_event != NULL) {
            netids.meshnetid = network->meshnetid;
            netids.sid = sid;
            router->cb.handle_subscribe_event(EVENT_SID_UPDATED, (uint8_t *)&netids, sizeof(netids_t));
        }
    }
}

void ur_router_neighbor_updated(neighbor_t *neighbor)
{
    if (g_rm_state.default_router != NULL && g_rm_state.default_router->cb.handle_neighbor_updated != NULL) {
        g_rm_state.default_router->cb.handle_neighbor_updated(neighbor);
    }
}

router_t *ur_get_router_by_id(uint8_t id)
{
    router_t *router;
    slist_for_each_entry(&g_rm_state.router_list, router, router_t, next) {
        if (router->id == id) {
            return router;
        }
    }
    return NULL;
}

uint8_t ur_router_get_default_router(void)
{
    if (g_rm_state.default_router != NULL) {
        return g_rm_state.default_router->id;
    }
    return 0;
}

ur_error_t ur_router_set_default_router(uint8_t id)
{
    router_t *router;

    router = ur_get_router_by_id(id);
    if (router == NULL) {
        return UR_ERROR_FAIL;
    }

    if (router == g_rm_state.default_router) {
        return UR_ERROR_NONE;
    }

    if (g_rm_state.default_router->cb.stop != NULL) {
        g_rm_state.default_router->cb.stop();
    }

    g_rm_state.default_router = router;
    if (g_rm_state.default_router->cb.start != NULL) {
        g_rm_state.default_router->cb.start();
    }

    return UR_ERROR_NONE;
}

ur_error_t ur_router_send_message(router_t *router, uint16_t dst,
                                  uint8_t *payload, uint16_t length)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;
    uint16_t    msg_length;
    message_t   *message;
    uint8_t     *data;
    mesh_dest_t *dest;

    if (router == NULL || payload == NULL) {
        return UR_ERROR_FAIL;
    }

    if (ur_get_router_by_id(router->id) == NULL) {
        return UR_ERROR_FAIL;
    }

    /* not in use, ignore */
    if (!router->network) {
        return UR_ERROR_NONE;
    }

    msg_length = sizeof(mm_header_t) + sizeof(router->id) + length;
    data = (uint8_t *)ur_mem_alloc(msg_length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }

    msg_length = 0;
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_ROUTING_INFO_UPDATE | COMMAND_RANGE_MESH;
    msg_length += sizeof(mm_header_t);

    *(data + msg_length) = router->id;
    msg_length += sizeof(router->id);
    memcpy((data + msg_length), payload, length);
    msg_length += length;

    message = message_alloc(router->network, msg_length);
    if (message == NULL) {
        ur_mem_free(data, msg_length);
        return UR_ERROR_MEM;
    }

    message_copy_from(message, data, msg_length);
    ur_mem_free(data, msg_length);

    dest = message->dest;
    dest->dest.len = 2;
    dest->meshnetid = mm_get_meshnetid(NULL);
    dest->dest.short_addr = dst;
    dest->flags = 0;
    error = mf_send_command(mm_header->command, message);
    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_ROUTE,
           "router %d send routing info to %04x, len %d\r\n", router->id, dst, msg_length);
    return error;
}

ur_error_t handle_router_message_received(uint8_t *payload, uint16_t length,
                                          const mesh_src_t *src, const mac_address_t *dest)
{
    router_t *router;

    router = ur_get_router_by_id(payload[sizeof(mm_header_t)]);
    if (router == NULL) {
        return UR_ERROR_FAIL;
    }

    if (router->cb.handle_message_received == NULL) {
        return UR_ERROR_FAIL;
    }
    payload += sizeof(mm_header_t) + sizeof(router->id);
    length  -= sizeof(mm_header_t) + sizeof(router->id);
    router->cb.handle_message_received(payload, length);

    return UR_ERROR_NONE;
}

ur_error_t register_router(router_t *router)
{
    if(router == NULL || ur_get_router_by_id(router->id) != NULL) {
        return UR_ERROR_FAIL;
    }

    slist_add(&router->next, &g_rm_state.router_list);
    return UR_ERROR_NONE;
}
