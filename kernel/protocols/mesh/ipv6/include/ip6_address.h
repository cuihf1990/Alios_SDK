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

#ifndef UR_IP6_ADDRESS_H
#define UR_IP6_ADDRESS_H

#include <stdint.h>

#include "ip6.h"

enum {
    IP6_UCAST_ADDR_NUM = 2,
    IP6_MCAST_ADDR_NUM = 1,
};

bool ur_is_mcast(const ur_ip6_addr_t *addr);
bool ur_is_unique_local(const ur_ip6_addr_t *addr);

#endif  /* UR_IP6_ADDRESS_H */
