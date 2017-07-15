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

#ifndef UR_MESSAGE_H
#define UR_MESSAGE_H

#include <stdbool.h>

#include "lwip/pbuf.h"

#include "umesh_types.h"

enum {
    MESSAGE_BUF_SIZE = 16,

    MESSAGE_RX_BUF_SIZE = 2,
};

typedef struct pbuf data_t;

typedef struct message_info_s {
    uint8_t type;

    media_type_t hal_type;
    void *network;
    ur_addr_t src_mac;
    ur_addr_t src;
    uint8_t src_channel;

    ur_addr_t dest;

    uint8_t dir;
    ur_addr_t dest2;

    uint8_t command;
    uint8_t payload_offset;
    uint8_t hops;
    uint16_t flags;
    uint8_t channel;
    int8_t key_index;
    int8_t rssi;
} message_info_t;

typedef struct message_s {
    dlist_t        next;
    data_t         *data;
    message_info_t *info;
    uint16_t       frag_offset;
    uint16_t tot_len;

    uint16_t debug_info;
} message_t;

enum {
    UMESH_1 = 0,
    MESH_FORWARDER_1 = 1,
    MESH_FORWARDER_2 = 2,
    MESH_FORWARDER_3 = 3,
    MESH_MGMT_1 = 4,
    MESH_MGMT_2 = 5,
    MESH_MGMT_3 = 6,
    MESH_MGMT_4 = 7,
    MESH_MGMT_5 = 8,
    MESH_MGMT_6 = 9,
    ADDRESS_MGMT_1 = 10,
    ADDRESS_MGMT_2 = 11,
    ADDRESS_MGMT_3 = 12,
    NETWORK_MGMT_1 = 13,
    NETWORK_MGMT_2 = 14,
    LOWPAN6_1 = 15,
    LOWPAN6_2 = 16,
    LINK_MGMT_1 = 17,
    LINK_MGMT_2 = 18,
    LINK_MGMT_3 = 19,
    ROUTER_MGR_1 = 20,
    DIAGS_1 = 21,
    DIAGS_2 = 22,
};

typedef dlist_t message_queue_t;

message_t  *message_alloc(uint16_t length,uint16_t debug_info);
ur_error_t message_free(message_t *message);
ur_error_t message_copy_to(const message_t *src, uint16_t src_offset,
                           uint8_t *dest, uint16_t dest_length);
ur_error_t message_copy_from(const message_t *dest,
                             uint8_t *src, uint16_t src_length);
ur_error_t message_copy(message_t *dest, const message_t *src);
ur_error_t message_set_payload_offset(const message_t *message, int16_t offset);
uint8_t    *message_get_payload(const message_t *message);
uint16_t   message_get_msglen(const message_t *message);
ur_error_t message_set_msglen(const message_t *message, uint16_t length);
uint16_t   message_get_buflen(const message_t *message);
ur_error_t message_set_buflen(const message_t *message, uint16_t length);
ur_error_t message_concatenate(message_t *dest, message_t *message,
                               bool reference);

message_t *message_queue_get_head(message_queue_t *queue);
ur_error_t message_queue_enqueue(message_queue_t *queue, message_t *message);
ur_error_t message_queue_dequeue(message_t *message);
uint16_t message_queue_get_size(message_queue_t *queue);

void message_stats_reset(void);
const ur_message_stats_t *message_get_stats(void);

#define for_each_message(msg, queue) \
    dlist_t *__tmp; \
    dlist_for_each_entry_safe(queue, __tmp, msg, message_t, next)

#endif  /* UR_MESSAGE_H */
