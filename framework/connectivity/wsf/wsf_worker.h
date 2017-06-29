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

#ifndef WSF_WORKER_H
#define WSF_WORKER_H
#include "wsf_client.h"
#include "wsf_config.h"

typedef struct request_msg_node {
    dlist_t list_head;
    uint32_t length;
    uint8_t msg[0];
} request_msg_node_t;

#ifndef CONFIG_REQMSG_LENGTH
#define CONFIG_REQMSG_LENGTH (10)
#endif

void init_req_glist(void);
void deinit_req_glist(void);

wsf_code wsf_start_worker(wsf_config_t *config);
void wsf_wait_worker(void);
void wsf_stop_worker(void);

#endif
