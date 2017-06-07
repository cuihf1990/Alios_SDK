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

#ifndef UR_LOGGING_H
#define UR_LOGGING_H

typedef enum {
    UR_LOG_LEVEL_CRIT,
    UR_LOG_LEVEL_WARN,
    UR_LOG_LEVEL_INFO,
    UR_LOG_LEVEL_DEBUG,
} ur_log_level_t;

#define lvl2str(l) ({ \
    const char *_s = "unknown"; \
    switch (l) { \
    case UR_LOG_LEVEL_CRIT: _s = "critical"; break; \
    case UR_LOG_LEVEL_WARN: _s = "warning"; break; \
    case UR_LOG_LEVEL_INFO: _s = "information"; break; \
    case UR_LOG_LEVEL_DEBUG: _s = "debug"; break; \
    } \
    _s; })

#define str2lvl(s) ({ \
    int _l; \
    if (strcmp(s, "critical") == 0) _l = UR_LOG_LEVEL_CRIT; \
    else if (strcmp(s, "warning") == 0) _l = UR_LOG_LEVEL_WARN; \
    else if (strcmp(s, "information") == 0) _l = UR_LOG_LEVEL_INFO; \
    else if (strcmp(s, "debug") == 0) _l = UR_LOG_LEVEL_DEBUG; \
    else _l = UR_LOG_LEVEL_INFO; \
    _l; })

typedef enum {
    UR_LOG_REGION_API,
    UR_LOG_REGION_ROUTE,
    UR_LOG_REGION_MM,
    UR_LOG_REGION_ARP,
    UR_LOG_REGION_6LOWPAN,
    UR_LOG_REGION_IP6,
    UR_LOG_REGION_HAL,
    UR_LOG_REGION_CLI,
} ur_log_region_t;

void ur_log(ur_log_level_t level, ur_log_region_t region,
            const char *format, ...);

#define EXT_ADDR_FMT "%02x%02x%02x%02x%02x%02x%02x%02x"
#define EXT_ADDR_DATA(addr) \
                      addr[0],addr[1],addr[2],addr[3],\
                      addr[4],addr[5],addr[6],addr[7]

#define IP6_ADDR_FMT "%x:%x:%x:%x:%x:%x:%x:%x"
#define IP6_ADDR_DATA(dest) \
                        ur_swap16(dest.m16[0]), \
                        ur_swap16(dest.m16[1]), \
                        ur_swap16(dest.m16[2]), \
                        ur_swap16(dest.m16[3]), \
                        ur_swap16(dest.m16[4]), \
                        ur_swap16(dest.m16[5]), \
                        ur_swap16(dest.m16[6]), \
                        ur_swap16(dest.m16[7])

#define format_ip6_str(ip6_addr, buf, len) \
    snprintf(buf, len, IP6_ADDR_FMT, IP6_ADDR_DATA(ip6_addr))

ur_log_level_t ur_log_get_level(void);
void ur_log_set_level(ur_log_level_t);
#endif  /* UR_LOGGING_H */
