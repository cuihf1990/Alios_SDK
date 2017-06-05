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

#ifndef UR_BLE_HAL_H
#define UR_BLE_HAL_H

enum {
    BLE_DISCOVERY_TIMEOUT          = 500,    /* ms */
    BLE_ATTACH_REQUEST_TIMEOUT     = 10000,   /* ms */
    BLE_SID_REQUEST_TIMEOUT        = 30000,   /* ms */
    BLE_LINK_REQUEST_MOBILE_TIMEOUT = 3000,     /* ms */
    BLE_LINK_REQUEST_TIMEOUT        = 30000,   /* ms */
    BLE_ADVERTISEMENT_TIMEOUT      = 20000,  /* ms, 20 seconds */
    BLE_NEIGHBOR_ALIVE_TIMEOUT     = 120000, /* ms, 2  mins */
    BLE_MIGRATE_WAIT_TIMEOUT       = 6 * BLE_ADVERTISEMENT_TIMEOUT,
    BLE_NOTIFICATION_TIMEOUT       = 300000,  /* ms, 5 mins */
    BLE_ADDR_CACHE_ALIVE_TIMEOUT   = 3,
};

#endif  /* UR_BLE_HAL_H */
