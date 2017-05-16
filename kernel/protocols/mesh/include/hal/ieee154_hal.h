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

#ifndef UR_IEEE_154_HAL_H
#define UR_IEEE_154_HAL_H

enum {
    IEEE154_DISCOVERY_TIMEOUT          = 400,    /* ms */
    IEEE154_ATTACH_REQUEST_TIMEOUT     = 1000,   /* ms */
    IEEE154_SID_REQUEST_TIMEOUT        = 3000,   /* ms */
    IEEE154_LINK_REQUEST_MOBILE_TIMEOUT = 1000,     /* ms */
    IEEE154_LINK_REQUEST_TIMEOUT        = 30000,   /* ms */
    IEEE154_ADVERTISEMENT_TIMEOUT      = 20000,  /* ms, 20 seconds */
    IEEE154_NEIGHBOR_ALIVE_TIMEOUT     = 120000, /* ms, 2  mins */
    IEEE154_MIGRATE_WAIT_TIMEOUT       = 8 * IEEE154_ADVERTISEMENT_TIMEOUT,
    IEEE154_NOTIFICATION_TIMEOUT       = 60000,  /* ms, 1 mins */
    IEEE154_ADDR_CACHE_ALIVE_TIMEOUT   = 3,
};

#endif  /* UR_IEEE_154_HAL_H */
