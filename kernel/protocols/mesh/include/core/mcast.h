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

#ifndef UR_MCAST_H
#define UR_MCAST_H

#include "hal/interface_context.h"

typedef struct mcast_header_s {
    uint8_t  control;
    uint8_t subnetid;
    uint16_t sid;
    uint8_t  sequence;
} __attribute__((packed)) mcast_header_t;

ur_error_t insert_mcast_header(network_context_t *network,
                               uint8_t *message);
ur_error_t process_mcast_header(network_context_t *network,
                                uint8_t *message);

#endif  /* UR_MCAST_H */
