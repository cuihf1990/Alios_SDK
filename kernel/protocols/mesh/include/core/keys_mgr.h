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

#ifndef UR_KEYS_MANAGER_H
#define UR_KEYS_MANAGER_H

#include "umesh_types.h"

enum {
    SEC_LEVEL_0 = 0,
    SEC_LEVEL_1 = 1,
};

ur_error_t calculate_one_time_key(uint8_t *key, uint32_t timestamp,
                                  const mac_address_t *addr);
ur_error_t calculate_network_key(void);

ur_error_t set_symmetric_key(uint8_t key_index, uint8_t *payload,
                             uint8_t length);
const uint8_t *get_symmetric_key(uint8_t key_index);

#endif /* UR_KEYS_MANAGER_H */
