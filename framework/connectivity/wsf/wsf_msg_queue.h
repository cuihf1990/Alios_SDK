/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

#ifndef WSF_MSG_QUEUE_H
#define WSF_MSG_QUEUE_H

#include "wsf_list.h"
#include "yos/list.h"
#include "wsf_msg.h"
#include <semaphore.h>

typedef struct wsf_request_queue_t {
    dlist_t list;
    void *mutex;
    sem_t *psem;
    int length;
} wsf_request_queue_t;

void wsf_msg_queue_init(wsf_request_queue_t **ppqueue);
void wsf_msg_queue_destroy(wsf_request_queue_t *pqueue);
void wsf_request_queue_init(wsf_request_queue_t *req_queue);
int wsf_request_queue_push(wsf_request_queue_t *req_queue,
                           wsf_request_node_t *node);
int wsf_request_queue_pop(wsf_request_queue_t *req_queue,
                          wsf_request_node_t *req_node);
wsf_request_node_t *wsf_request_queue_trigger(wsf_request_queue_t *req_queue,
                                              wsf_msg_t *rsp);
void wsf_msg_request_queue_destroy(wsf_request_queue_t *req_queue);
#endif
