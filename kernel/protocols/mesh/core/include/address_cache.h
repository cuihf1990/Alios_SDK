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

#ifndef UR_ADDRESS_CACHE_H
#define UR_ADDRESS_CACHE_H

#include "sid_allocator.h"

enum {
    ADDR_CACHE_CHECK_INTERVAL = 60000,  /* 1 mins */
};

void start_addr_cache(void);
void stop_addr_cache(void);

ur_error_t update_address_cache(media_type_t type, ur_node_id_t *target,
                                ur_node_id_t *attach);

void get_attach_by_nodeid(ur_node_id_t *attach, ur_node_id_t *target);
void get_target_by_ueid(ur_node_id_t *node_id, uint8_t *ueid);

#endif  /* UR_ADDRESS_CACHE_H */
