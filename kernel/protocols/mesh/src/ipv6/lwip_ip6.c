/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>

#include <yos/yos.h>

#include "lwip/sockets.h"
#include "lwip/inet.h"

#include "ipv6/ip6.h"
#include "umesh_utils.h"

typedef struct listen_sockets_s {
    int                socket;
    raw_data_handler_t handler;
} listen_sockets_t;

#ifdef WITH_LWIP
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
#endif

#if LWIP_IPV6
int echo_socket(raw_data_handler_t handler)
{
    static listen_sockets_t g_echo;
    g_echo.socket = lwip_socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    g_echo.handler = handler;
#ifdef WITH_LWIP
    aos_poll_read_fd(g_echo.socket, ur_sock_read_cb, &g_echo);
#endif
    return g_echo.socket;
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
#ifdef WITH_LWIP
    aos_poll_read_fd(udp_autotest.socket, ur_sock_read_cb, &udp_autotest);
#endif
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
#else
int echo_socket(raw_data_handler_t handler)
{
    static listen_sockets_t g_echo;
    g_echo.socket = lwip_socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    g_echo.handler = handler;
#ifdef WITH_LWIP
    aos_poll_read_fd(g_echo.socket, ur_sock_read_cb, &g_echo);
#endif
    return g_echo.socket;
}

int autotest_udp_socket(raw_data_handler_t handler, uint16_t port)
{
    static listen_sockets_t udp_autotest;
    struct sockaddr_in      addr;

    udp_autotest.socket = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    lwip_bind(udp_autotest.socket, (const struct sockaddr *)&addr, sizeof(addr));
    udp_autotest.handler = handler;
#ifdef WITH_LWIP
    aos_poll_read_fd(udp_autotest.socket, ur_sock_read_cb, &udp_autotest);
#endif
    return udp_autotest.socket;
}

int ip6_sendto(int socket, const uint8_t *payload, uint16_t length,
               mesh_ip4_addr_t *dest, uint16_t port)
{
    struct sockaddr_in sock_addr;

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_len = sizeof(sock_addr);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = dest->m32;
    return lwip_sendto(socket, payload, length, 0, (struct sockaddr *)&sock_addr,
                       sizeof(sock_addr));
}
#endif

int ip6_recv(int socket, void *payload, uint16_t length)
{
    return lwip_recv(socket, payload, (size_t)length, 0);
}
