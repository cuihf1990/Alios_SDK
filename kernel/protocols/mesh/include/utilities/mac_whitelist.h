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

#ifndef UR_MAC_WHITELIST_H
#define UR_MAC_WHITELIST_H

#include "umesh_hal.h"

typedef struct whitelist_entry_s {
    mac_address_t address;
    int8_t        rssi;
    bool          valid;
    bool          constant_rssi;
} whitelist_entry_t;

void whitelist_enable(void);
void whitelist_disable(void);
bool whitelist_is_enabled(void);
const whitelist_entry_t *whitelist_get_entries(void);
whitelist_entry_t *whitelist_add(const mac_address_t *address);
void whitelist_remove(const mac_address_t *address);
whitelist_entry_t *whitelist_find(const mac_address_t *address);
void whitelist_clear(void);
void whitelist_clear_constant_rssi(whitelist_entry_t *entry);
ur_error_t whitelist_get_constant_rssi(const whitelist_entry_t *entry,
                                       int8_t *rssi);
void whitelist_set_constant_rssi(whitelist_entry_t *entry, int8_t rssi);

#endif  /* UR_MAC_WHITELIST_H */
