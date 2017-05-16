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

#include <string.h>

#include <yos/framework.h>

#include "lwip/sockets.h"
#include "lwip/inet.h"

#include "ipv6/ip6.h"
#include "utilities/memory.h"

typedef struct listen_sockets_s {
    int                socket;
    raw_data_handler_t handler;
} listen_sockets_t;

static void ur_sock_read_cb(int fd, void *arg)
{
    listen_sockets_t *lsock = arg;
    int     length = 0;
    uint8_t *buffer;

    buffer = ur_mem_alloc(UR_IP6_MTU + UR_IP6_HLEN);
    if (buffer == NULL) {
        return;
    }

    length = lwip_recv(lsock->socket, buffer, UR_IP6_MTU + UR_IP6_HLEN, 0);
    if (length > 0) {
        lsock->handler(buffer, length);
    }
    ur_mem_free(buffer, UR_IP6_MTU + UR_IP6_HLEN);
}

int echo_socket(raw_data_handler_t handler)
{
    static listen_sockets_t g_echo;
    g_echo.socket = lwip_socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    g_echo.handler = handler;
    yos_poll_read_fd(g_echo.socket, ur_sock_read_cb, &g_echo);
    return g_echo.socket;
}

int autotest_raw_socket(raw_data_handler_t handler)
{
    static listen_sockets_t raw_autotest;

    raw_autotest.socket = lwip_socket(AF_INET6, SOCK_RAW, IPPROTO_UDP);
    raw_autotest.handler = handler;
    yos_poll_read_fd(raw_autotest.socket, ur_sock_read_cb, &raw_autotest);
    return raw_autotest.socket;
}

int autotest_udp_socket(raw_data_handler_t handler, uint16_t port)
{
    static listen_sockets_t udp_autotest;
    struct sockaddr_in6     addr;

    udp_autotest.socket = lwip_socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(port);
    lwip_bind(udp_autotest.socket, (const struct sockaddr *)&addr, sizeof(addr));
    udp_autotest.handler = handler;
    yos_poll_read_fd(udp_autotest.socket, ur_sock_read_cb, &udp_autotest);
    return udp_autotest.socket;
}

int ip6_sendto(int socket, const uint8_t *payload, uint16_t length,
               ur_ip6_addr_t *dest, uint16_t port)
{
    struct sockaddr_in6 sock_addr;
    uint8_t index;

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin6_len = sizeof(sock_addr);
    sock_addr.sin6_family = AF_INET6;
    sock_addr.sin6_port = htons(port);
    for (index = 0; index < UR_IP6_ADDR_SIZE / sizeof(uint32_t); ++index) {
        sock_addr.sin6_addr.un.u32_addr[index] = dest->m32[index];
    }
    return lwip_sendto(socket, payload, length, 0, (struct sockaddr *)&sock_addr,
                       sizeof(sock_addr));
}

int ip6_recv(int socket, void *payload, uint16_t length)
{
    return lwip_recv(socket, payload, (size_t)length, 0);
}
