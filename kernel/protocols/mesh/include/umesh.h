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

#ifndef UMESH_MESH_H
#define UMESH_MESH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "umesh_types.h"

struct pbuf;
struct message_s;

/* for ip layer */
typedef ur_error_t (* adapter_input_t)(struct pbuf *buf);
typedef ur_error_t (* adapter_interface_up_t)(void);
typedef ur_error_t (* adapter_interface_down_t)(void);
typedef ur_error_t (* adapter_interface_update_t)(void);
typedef struct ur_adapter_callback_s {
    adapter_input_t input;
    adapter_interface_up_t interface_up;
    adapter_interface_down_t interface_down;
    adapter_interface_update_t interface_update;
} ur_adapter_callback_t;

ur_error_t umesh_ipv4_output(struct pbuf *buf, uint16_t sid);
ur_error_t umesh_ipv6_output(struct pbuf *buf,
                             const ur_ip6_addr_t *ip6addr);
ur_error_t umesh_register_callback(ur_adapter_callback_t *callback);

/* for mesh layer */
ur_error_t umesh_input(struct message_s *message);

ur_error_t umesh_init(node_mode_t mode);
ur_error_t umesh_start(void);
bool       umesh_is_initialized(void);
ur_error_t umesh_stop(void);

/* per device API */
uint8_t umesh_get_device_state(void);
uint8_t umesh_get_mode(void);
ur_error_t umesh_set_mode(uint8_t mode);
int8_t umesh_get_seclevel(void);
ur_error_t umesh_set_seclevel(int8_t level);

/* per network API */
typedef enum {
    UR_MESH_NET_CORE = 1,
    UR_MESH_NET_SUB = 2,
    UR_MESH_NET_DFL = UR_MESH_NET_SUB,
} umesh_net_index_t;

const mac_address_t *umesh_net_get_mac_address(umesh_net_index_t nettype);
uint16_t umesh_net_get_meshnetid(umesh_net_index_t nettype);
uint16_t umesh_net_get_sid(umesh_net_index_t nettype);
uint16_t umesh_net_get_meshnetsize(umesh_net_index_t nettype);
/* translation macros */
#define umesh_get_meshnetid() umesh_net_get_meshnetid(UR_MESH_NET_DFL)
#define umesh_get_sid() umesh_net_get_sid(UR_MESH_NET_DFL)
#define umesh_get_meshnetsize() umesh_net_get_meshnetsize(UR_MESH_NET_DFL)
#define umesh_get_mac_address() umesh_net_get_mac_address(UR_MESH_NET_DFL)

bool umesh_is_mcast_subscribed(const ur_ip6_addr_t *addr);
const ur_netif_ip6_address_t *umesh_get_ucast_addr(void);
const ur_netif_ip6_address_t *umesh_get_mcast_addr(void);

ur_error_t umesh_resolve_dest(const ur_ip6_addr_t *dest,
                              ur_addr_t *dest_addr);
void umesh_get_channel(channel_t *channel);

void umesh_get_extnetid(umesh_extnetid_t *extnetid);
ur_error_t umesh_set_extnetid(const umesh_extnetid_t *extnetid);

slist_t *umesh_get_hals(void);
slist_t *umesh_get_networks(void);

bool umesh_is_whitelist_enabled(void);
void umesh_enable_whitelist(void);
void umesh_disable_whitelist(void);
const whitelist_entry_t *umesh_get_whitelist_entries(void);
ur_error_t umesh_add_whitelist(const mac_address_t *address);
ur_error_t umesh_add_whitelist_rssi(const mac_address_t *address,
                                    int8_t rssi);
void umesh_remove_whitelist(const mac_address_t *address);
void umesh_clear_whitelist(void);

const ur_link_stats_t *umesh_get_link_stats(media_type_t type);
const frame_stats_t *umesh_get_hal_stats(media_type_t type);
const ur_message_stats_t *umesh_get_message_stats(void);
const ur_mem_stats_t *umesh_get_mem_stats(void);

#ifdef __cplusplus
}
#endif

#endif  /* UMESH_MESH_H */
