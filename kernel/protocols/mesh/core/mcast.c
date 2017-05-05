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
#include <stdint.h>
#include <string.h>

#include "mesh_types.h"
#include "mesh_mgmt.h"
#include "mcast.h"
#include "timer.h"
#include "interfaces.h"

enum {
    MCAST_CACHE_ENTRY_LIFETIME = 10,
    MCAST_CACHE_LIFETIME_UNIT  = 1000,
};

static void handle_mcast_timer(void *args);

ur_error_t insert_mcast_header(network_context_t *network, uint8_t *message)
{
    mcast_header_t *header = (mcast_header_t *)message;

    assert(header);
    network = get_default_network_context();
    if (network == NULL) {
        return UR_ERROR_FAIL;
    }

    header->control = 0;
    header->subnetid = mm_get_sub_netid(network);
    header->sid = mm_get_local_sid();
    header->sequence = network->mcast_sequence++;
    return UR_ERROR_NONE;
}

ur_error_t process_mcast_header(network_context_t *network, uint8_t *message)
{
    ur_error_t     error = UR_ERROR_NONE;
    uint16_t       index;
    mcast_header_t *header = (mcast_header_t *)message;
    mcast_entry_t  *entry = NULL;
    int8_t         diff;
    bool           from_local_net = false;
    slist_t        *networks;

    assert(message);
    networks = get_network_contexts();
    slist_for_each_entry(networks, network, network_context_t, next) {
        if (header->subnetid == mm_get_sub_netid(network)) {
            from_local_net = true;
            break;
        }
    }
    network = get_default_network_context();
    if (header->sid == mm_get_local_sid() && from_local_net) {
        return UR_ERROR_DROP;
    }

    for (index = 0; index < MCAST_CACHE_ENTRIES_SIZE; index++) {
        if (network->mcast_entry[index].lifetime == 0) {
            entry = &network->mcast_entry[index];
        } else if (network->mcast_entry[index].sid == header->sid &&
                   network->mcast_entry[index].subnetid == header->subnetid) {
            entry = &network->mcast_entry[index];
            diff = header->sequence - entry->sequence;
            if (diff <= 0) {
                error = UR_ERROR_DROP;
            }
            break;
       }
    }

    if (entry == NULL) {
        return UR_ERROR_DROP;
    }

    entry->subnetid = header->subnetid;
    entry->sid = header->sid;
    entry->sequence = header->sequence;
    entry->lifetime = MCAST_CACHE_ENTRY_LIFETIME;
    if (network->mcast_timer == NULL) {
        network->mcast_timer = ur_start_timer(MCAST_CACHE_LIFETIME_UNIT,
                                              handle_mcast_timer, network);
    }
    return error;
}

static void handle_mcast_timer(void *args)
{
    bool    start_timer = false;
    uint8_t index;
    network_context_t *network;

    network = (network_context_t *)args;
    network->mcast_timer = NULL;
    for (index = 0; index < MCAST_CACHE_ENTRIES_SIZE; index++) {
        if (network->mcast_entry[index].lifetime > 0) {
            network->mcast_entry[index].lifetime--;
            start_timer = true;
        }
    }

    if (start_timer) {
        network->mcast_timer = ur_start_timer(MCAST_CACHE_LIFETIME_UNIT,
                                              handle_mcast_timer, network);
    }
}
