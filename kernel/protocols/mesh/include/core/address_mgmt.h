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

#ifndef UR_AR_H
#define UR_AR_H

#include <stdint.h>

#include "core/sid_allocator.h"
#include "core/mesh_forwarder.h"
#include "hal/interface_context.h"

enum {
    UR_MESH_ADDRESS_CACHE_SIZE = 8,

    ADDR_CACHE_CHECK_INTERVAL = 60000,  /* 1 mins */
};

typedef enum {
    AQ_STATE_INVALID,
    AQ_STATE_CACHED,
    AQ_STATE_QUERY,
} cache_state_t;

typedef struct address_cache_s {
    uint8_t       ueid[EXT_ADDR_SIZE];
    uint16_t      meshnetid;
    uint16_t      sid;
    uint16_t      attach_sid;
    uint16_t      attach_netid;
    uint8_t       timeout;
    uint8_t       retry_timeout;
    cache_state_t state;
} address_cache_t;

enum {
    ADDRESS_QUERY_TIMEOUT             = 3,    /* seconds */
    ADDRESS_QUERY_RETRY_TIMEOUT       = 3,    /* seconds */
    ADDRESS_QUERY_STATE_UPDATE_PERIOD = 1000, /* ms */
};

enum {
    PF_ATTACH_QUERY = 0,
    TARGET_QUERY = 1,
};

typedef void (* address_resolved_handler_t)(network_context_t *network,
                                            address_cache_t *target,
                                            ur_error_t error);

void address_resolver_init(address_resolved_handler_t handler);
ur_error_t address_resolve(uint8_t query_type, ur_node_id_t *target,
                           ur_node_id_t *attach);
ur_error_t handle_address_query(message_t *message);
ur_error_t handle_address_query_response(message_t *message);
ur_error_t handle_address_notification(message_t *message);
ur_error_t handle_address_unreachable(message_t *message);

ur_error_t send_address_notification(network_context_t *network,
                                     ur_addr_t *dest);
ur_error_t send_address_unreachable(network_context_t *network,
                                    ur_addr_t *dest, ur_addr_t *target);

void start_addr_cache(void);
void stop_addr_cache(void);

ur_error_t update_address_cache(media_type_t type, ur_node_id_t *target,
                                ur_node_id_t *attach);

void get_attach_by_nodeid(ur_node_id_t *attach, ur_node_id_t *target);
void get_target_by_ueid(ur_node_id_t *node_id, uint8_t *ueid);

#endif  /* UR_AR_H */
