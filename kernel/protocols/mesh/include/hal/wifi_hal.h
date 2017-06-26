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

#ifndef UR_WIFI_HAL_H
#define UR_WIFI_HAL_H

enum {
    WIFI_DISCOVERY_TIMEOUT           = 75,     /* ms */
    WIFI_ATTACH_REQUEST_TIMEOUT      = 1000,   /* ms */
    WIFI_SID_REQUEST_TIMEOUT         = 3000,   /* ms */
    WIFI_LINK_REQUEST_MOBILE_TIMEOUT = 1000,     /* ms */
    WIFI_LINK_REQUEST_TIMEOUT        = 30000,   /* ms */
#ifndef CONFIG_YOS_DDA
    WIFI_ADVERTISEMENT_TIMEOUT       = 20000,  /* ms, 20 seconds */
    WIFI_NEIGHBOR_ALIVE_TIMEOUT      = 120000, /* ms, 2 mins */
    WIFI_NET_SCAN_TIMEOUT            = 300000, /* ms, 5 mins */
#else
    WIFI_ADVERTISEMENT_TIMEOUT       = 4000,  /* ms, 4 seconds */
    WIFI_NEIGHBOR_ALIVE_TIMEOUT      = 24000, /* ms, 24 seconds */
    WIFI_NET_SCAN_TIMEOUT            = 60000, /* ms, 1 mins */
#endif
    WIFI_MIGRATE_WAIT_TIMEOUT        = 5 * WIFI_ADVERTISEMENT_TIMEOUT,
    WIFI_NOTIFICATION_TIMEOUT        = 60000,  /* ms, 1 mins */
    WIFI_ADDR_CACHE_ALIVE_TIMEOUT    = 3,
};

#endif  /* UR_WIFI_HAL_H */
