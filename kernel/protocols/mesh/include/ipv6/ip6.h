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

#ifndef UR_IP6_H
#define UR_IP6_H

#include <stdint.h>

#include "hal/interface_context.h"

enum {
    UR_IP6_MTU = 1500,
};

enum {
    UR_IPPROTO_UDP    = 17,
    UR_IPPROTO_ICMPV6 = 58,
};

enum {
    UR_ICMP6_TYPE_EREQ = 128,
    UR_ICMP6_TYPE_EREP = 129,
};

typedef struct ur_ip6_header_s {
    uint32_t      v_tc_fl;
    uint16_t      len;
    uint8_t       next_header;
    uint8_t       hop_lim;
    ur_ip6_addr_t src;
    ur_ip6_addr_t dest;
} __attribute__((packed)) ur_ip6_header_t;

typedef struct ur_icmp6_header_s {
    uint8_t  type;
    uint8_t  code;
    uint16_t chksum;
    uint32_t data;
} __attribute__((packed)) ur_icmp6_header_t;

typedef struct ur_udp_header_s {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t chksum;
} __attribute__((packed)) ur_udp_header_t;

typedef void (* raw_data_handler_t)(const uint8_t *payload, uint16_t length);

int echo_socket(raw_data_handler_t handler);
int autotest_udp_socket(raw_data_handler_t handler, uint16_t port);
int ip6_sendto(int socket, const uint8_t *payload, uint16_t length,
               ur_ip6_addr_t *dest, uint16_t port);
int ip6_recv(int socket, void *payload, uint16_t length);

ur_error_t string_to_ip6_addr(const char *buf, ur_ip6_addr_t *target);

bool ur_is_mcast(const ur_ip6_addr_t *addr);
bool ur_is_unique_local(const ur_ip6_addr_t *addr);

#endif  /* UR_IP6_H */
