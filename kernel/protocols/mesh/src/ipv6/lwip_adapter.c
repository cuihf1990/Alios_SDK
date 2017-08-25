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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "lwip/opt.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/ip6.h"
#include "lwip/ip.h"

#include "umesh.h"
#include "umesh_hal.h"
#include "umesh_utils.h"
#include "ipv6/lwip_adapter.h"
#include "core/mesh_mgmt.h"

typedef struct lwip_adapter_state_s {
    struct netif          adpif;
    ur_adapter_callback_t adapter_cb;
    const char            interface_name[3];
} lwip_adapter_state_t;

static lwip_adapter_state_t g_la_state = {.interface_name = "ur"};

/* Receive IP frame from umesh and pass up to LwIP */
ur_error_t ur_adapter_input(struct pbuf *buf)
{
    err_t       error = ERR_ARG;
    if (g_la_state.adpif.input) {
        pbuf_ref(buf);
        error = g_la_state.adpif.input(buf, &g_la_state.adpif);
    }
    return error == ERR_OK ? UR_ERROR_NONE : UR_ERROR_FAIL;
}

static err_t ur_adapter_ipv4_output(struct netif *netif, struct pbuf *p,
                                    const ip4_addr_t *ip4addr)
{
    ur_error_t error;
    uint16_t sid;

    if (ip4_addr_isany(ip4addr) || ip4_addr_isbroadcast(ip4addr, netif)) {
        sid = 0xffff;
    } else {
        sid = ntohl(ip4addr->addr) & 0xffff;
    }

    error = umesh_ipv4_output(p, sid);

    /* error mapping */
    switch (error) {
        case UR_ERROR_NONE:
            return ERR_OK;
            break;
        case UR_ERROR_FAIL:
            return ERR_VAL;
            break;
        default:
            return ERR_VAL;
            break;
    }
    return ERR_OK;
}

#if LWIP_IPV6
static err_t ur_adapter_ipv6_output(struct netif *netif, struct pbuf *p,
                                    const ip6_addr_t *ip6addr)
{
    ur_error_t error;

    error = umesh_ipv6_output(p, (ur_ip6_addr_t *)ip6addr);

    /* error mapping */
    switch (error) {
        case UR_ERROR_NONE:
            return ERR_OK;
            break;
        case UR_ERROR_FAIL:
            return ERR_VAL;
            break;
        default:
            return ERR_VAL;
            break;
    }
    return ERR_OK;
}
#endif

static err_t ur_adapter_if_init(struct netif *netif)
{
    netif->name[0] = g_la_state.interface_name[0];
    netif->name[1] = g_la_state.interface_name[1];
    netif->num = g_la_state.interface_name[2] - '0';
#if LWIP_IPV6
    netif->output_ip6 = ur_adapter_ipv6_output;
#endif
    netif->output = ur_adapter_ipv4_output;
    netif->mtu = 127;
    netif->flags = NETIF_FLAG_LINK_UP | NETIF_FLAG_UP | NETIF_FLAG_BROADCAST ;
    return ERR_OK;
}

ur_error_t ur_adapter_interface_init(void)
{
    g_la_state.adapter_cb.input = ur_adapter_input;
    g_la_state.adapter_cb.interface_up = ur_adapter_interface_up;
    g_la_state.adapter_cb.interface_down = ur_adapter_interface_down;
    g_la_state.adapter_cb.interface_update = ur_adapter_interface_update;
    umesh_register_callback(&g_la_state.adapter_cb);
    return UR_ERROR_NONE;
}

static void update_interface_ipaddr(void)
{
#if LWIP_IPV6
    const ur_netif_ip6_address_t *ip6_addr;
    uint8_t                      index = 0;
    uint8_t                      addr_index;

    ip6_addr = umesh_get_ucast_addr();
    while (ip6_addr) {
        ip6_addr_t addr6;
        netif_ip6_addr_set_state(&g_la_state.adpif, index, IP6_ADDR_INVALID);
        IP6_ADDR(&addr6, ip6_addr->addr.m32[0], ip6_addr->addr.m32[1],
                 ip6_addr->addr.m32[2], ip6_addr->addr.m32[3]);
        ip6_addr_copy(*(ip_2_ip6(&g_la_state.adpif.ip6_addr[index])), addr6);
        netif_ip6_addr_set_state(&g_la_state.adpif, index, IP6_ADDR_VALID);
        ip6_addr = ip6_addr->next;
        index++;
    }

    ip6_addr = umesh_get_mcast_addr();
    while (ip6_addr) {
        ip6_addr_t addr6;
        netif_ip6_addr_set_state(&g_la_state.adpif, index, IP6_ADDR_INVALID);
        memset(&addr6, 0, sizeof(addr6));
        for (addr_index = 0; addr_index < ip6_addr->prefix_length / 32; addr_index++) {
            addr6.addr[addr_index] = ip6_addr->addr.m32[addr_index];
        }
        ip6_addr_copy(*(ip_2_ip6(&g_la_state.adpif.ip6_addr[index])), addr6);
        netif_ip6_addr_set_state(&g_la_state.adpif, index, IP6_ADDR_VALID);
        ip6_addr = ip6_addr->next;
        index++;
    }

    g_la_state.adpif.ip6_autoconfig_enabled = 1;
#endif
}

ur_error_t ur_adapter_interface_up(void)
{
    const mac_address_t *mac_addr;
    struct netif        *interface;
    ip4_addr_t          ipaddr, netmask, gw;

    interface = netif_find(g_la_state.interface_name);

    if (interface == NULL) {
        mac_addr = umesh_get_mac_address();
        g_la_state.adpif.hwaddr_len = mac_addr->len;
        memcpy(g_la_state.adpif.hwaddr, mac_addr->addr, 6);

        uint16_t sid = umesh_mm_get_local_sid();
        IP4_ADDR(&gw, 192, 168, 0, 1);
        IP4_ADDR(&ipaddr, 192, 168, sid >> 8, sid & 0xff);
        IP4_ADDR(&netmask, 255, 255, 0, 0);
        netif_add(&g_la_state.adpif, &ipaddr, &netmask, &gw, NULL,
                  ur_adapter_if_init, tcpip_input);
        interface = &g_la_state.adpif;
    }

    assert(interface == &g_la_state.adpif);

    if (netif_is_up(interface) == 0) {
        netif_set_up(&g_la_state.adpif);
    }
    update_interface_ipaddr();
    return UR_ERROR_NONE;
}

ur_error_t ur_adapter_interface_down(void)
{
    if (netif_is_up(&g_la_state.adpif)) {
        netif_set_down(&g_la_state.adpif);
    }

    return UR_ERROR_NONE;
}

ur_error_t ur_adapter_interface_update(void)
{
    update_interface_ipaddr();
    return UR_ERROR_NONE;
}
#if LWIP_IPV6
struct netif *ur_adapter_ip6_route(const ip6_addr_t *src,
                                   const ip6_addr_t *dest)
{
    return &g_la_state.adpif;
}

bool ur_adapter_is_mcast_subscribed(const ip6_addr_t *addr)
{
    return umesh_is_mcast_subscribed((const ur_ip6_addr_t *)addr);
}

struct netif *lwip_hook_ip6_route(const ip6_addr_t *src, const ip6_addr_t *dest)
{
    return ur_adapter_ip6_route(src, dest);
}

bool lwip_hook_mesh_is_mcast_subscribed(const ip6_addr_t *dest)
{
    return ur_adapter_is_mcast_subscribed(dest);
}
#endif
