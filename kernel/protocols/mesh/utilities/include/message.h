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

#include "mesh_types.h"
#include "lwip_adapter.h"
#include "memory.h"

enum {
    MESSAGE_BUF_SIZE = 64,
};

typedef struct pbuf data_t;
typedef struct network_context_s network_context_t;

typedef struct message_s {
    dlist_t           next;
    data_t            *data;
    mesh_dest_t       *dest;
    network_context_t *network;
    uint16_t          frag_offset;
} message_t;

typedef dlist_t message_queue_t;

typedef struct ur_message_stats_s {
    uint16_t num;
    uint16_t fails;
} ur_message_stats_t;

message_t  *message_alloc(network_context_t *network, uint16_t length);
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
ur_error_t message_concatenate(const message_t *dest, message_t *message,
                               bool reference);

message_t *message_queue_get_head(message_queue_t *queue);
ur_error_t message_queue_enqueue(message_queue_t *queue, message_t *message);
ur_error_t message_queue_dequeue(message_t *message);

void message_stats_reset(void);
const ur_message_stats_t *message_get_stats(void);

#define for_each_message(msg, queue) \
    dlist_t *__tmp; \
    dlist_for_each_entry_safe(queue, __tmp, msg, message_t, next)

#endif  /* UR_MESSAGE_H */
