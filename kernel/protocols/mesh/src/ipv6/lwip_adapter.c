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

typedef struct lwip_adapter_state_s {
    struct netif          adpif;
    ur_adapter_callback_t adapter_cb;
    const char            interface_name[3];
} lwip_adapter_state_t;

static lwip_adapter_state_t g_la_state = {.interface_name = "ur"};

/* Receive IP frame from umesh and pass up to LwIP */
ur_error_t ur_adapter_input(void *message)
{
    err_t       error = ERR_ARG;
    struct pbuf *buffer;

    buffer = pbuf_alloc(PBUF_RAW, message_get_msglen((message_t *)message),
                        PBUF_POOL);
    if (buffer == NULL) {
        message_free(message);
        return UR_ERROR_FAIL;
    }
    pbuf_copy(buffer, (struct pbuf *)((message_t *)message)->data);
    if (g_la_state.adpif.input) {
        error = g_la_state.adpif.input(buffer, &g_la_state.adpif);
    }
    if (error != ERR_OK) {
        message_free(message);
        pbuf_free(buffer);
        return UR_ERROR_FAIL;
    }
    message_free(message);
    return UR_ERROR_NONE;
}

static err_t ur_adapter_ipv6_output(struct netif *netif, struct pbuf *p,
                                    const ip6_addr_t *ip6addr)
{
    ur_error_t error;
    message_t  message;

    memset(&message, 0, sizeof(message_t));
    message.data = p;
    error = ur_mesh_ipv6_output((umessage_t *)&message, (ur_ip6_addr_t *)ip6addr);

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

static err_t ur_adapter_if_init(struct netif *netif)
{
    netif->name[0] = g_la_state.interface_name[0];
    netif->name[1] = g_la_state.interface_name[1];
    netif->num = g_la_state.interface_name[2] - '0';
    netif->output_ip6 = ur_adapter_ipv6_output;
    netif->mtu = 127;
    netif->flags = NETIF_FLAG_LINK_UP | NETIF_FLAG_UP;
    return ERR_OK;
}

ur_error_t ur_adapter_interface_init(void)
{
    g_la_state.adapter_cb.input = ur_adapter_input;
    g_la_state.adapter_cb.interface_up = ur_adapter_interface_up;
    g_la_state.adapter_cb.interface_down = ur_adapter_interface_down;
    g_la_state.adapter_cb.interface_update = ur_adapter_interface_update;
    ur_mesh_register_callback(&g_la_state.adapter_cb);
    return UR_ERROR_NONE;
}

ur_error_t ur_adapter_interface_deinit(void)
{
    return UR_ERROR_NONE;
}

static void update_interface_ipaddr(void)
{
    ip6_addr_t                   addr6;
    const ur_netif_ip6_address_t *ip6_addr;
    uint8_t                      index = 0;
    uint8_t                      addr_index;

    ip6_addr = ur_mesh_get_ucast_addr();
    while (ip6_addr) {
        netif_ip6_addr_set_state(&g_la_state.adpif, index, IP6_ADDR_INVALID);
        IP6_ADDR(&addr6, ip6_addr->addr.m32[0], ip6_addr->addr.m32[1],
                 ip6_addr->addr.m32[2], ip6_addr->addr.m32[3]);
        ip6_addr_copy(*(ip_2_ip6(&g_la_state.adpif.ip6_addr[index])), addr6);
        netif_ip6_addr_set_state(&g_la_state.adpif, index, IP6_ADDR_VALID);
        ip6_addr = ip6_addr->next;
        index++;
    }

    ip6_addr = ur_mesh_get_mcast_addr();
    while (ip6_addr) {
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
}

ur_error_t ur_adapter_interface_up(void)
{
    const mac_address_t *mac_addr;
    struct netif        *interface;

    interface = netif_find(g_la_state.interface_name);

    if (interface == NULL) {
        mac_addr = ur_mesh_get_mac_address();
        g_la_state.adpif.hwaddr_len = mac_addr->len;
        memcpy(g_la_state.adpif.hwaddr, mac_addr->addr, 6);
        g_la_state.adpif.ip6_autoconfig_enabled = 1;

        netif_add(&g_la_state.adpif, NULL, NULL, NULL, NULL, ur_adapter_if_init,
                  tcpip_input);
        update_interface_ipaddr();
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

struct netif *ur_adapter_ip6_route(const ip6_addr_t *src,
                                   const ip6_addr_t *dest)
{
    return &g_la_state.adpif;
}

bool ur_adapter_is_mcast_subscribed(const ip6_addr_t *addr)
{
    return ur_mesh_is_mcast_subscribed((const ur_ip6_addr_t *)addr);
}

struct netif *lwip_hook_ip6_route(const ip6_addr_t *src, const ip6_addr_t *dest)
{
    return ur_adapter_ip6_route(src, dest);
}

bool lwip_hook_mesh_is_mcast_subscribed(const ip6_addr_t *dest)
{
    return ur_adapter_is_mcast_subscribed(dest);
}
