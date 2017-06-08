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

#include <stddef.h>

#include "umesh_hal.h"

static YOS_DLIST_HEAD(g_mesh_module);

int hal_ur_mesh_init(void)
{
    int ret = 0;
    dlist_t *t;

    /* do low level init */
    dlist_for_each(t, &g_mesh_module) {
        ur_mesh_hal_module_t *m = (ur_mesh_hal_module_t *)t;

        if (m->ur_mesh_hal_init) {
            ret = m->ur_mesh_hal_init(m, NULL);

            if (ret < 0) {
                break;
            }
        }
    }
    return ret;
}

void hal_ur_mesh_register_module(ur_mesh_hal_module_t *m)
{
    dlist_add_tail(&m->base.list, &g_mesh_module);
}

ur_mesh_hal_module_t *hal_ur_mesh_get_default_module(void)
{
    ur_mesh_hal_module_t *m = NULL;

    if (dlist_empty(&g_mesh_module)) {
        return NULL;
    }

    m = dlist_first_entry(&g_mesh_module, ur_mesh_hal_module_t, base.list);
    return m;
}

ur_mesh_hal_module_t *hal_ur_mesh_get_next_module(ur_mesh_hal_module_t *cur)
{
    ur_mesh_hal_module_t *m = NULL;

    if (cur->base.list.next == &g_mesh_module) {
        return NULL;
    }

    m = dlist_first_entry(&cur->base.list, ur_mesh_hal_module_t, base.list);
    return m;
}

int hal_ur_mesh_enable(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_enable != NULL)) {
        return m->ur_mesh_hal_enable(m);
    }

    return -1;
}

int hal_ur_mesh_disable(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_disable != NULL)) {
        return m->ur_mesh_hal_disable(m);
    }

    return -1;
}

int hal_ur_mesh_send_ucast_request(ur_mesh_hal_module_t *m,
                                   frame_t *frame, mac_address_t *dest,
                                   ur_mesh_handle_sent_ucast_t sent, void *context)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_send_ucast_request != NULL)) {
        return m->ur_mesh_hal_send_ucast_request(m, frame, dest, sent, context);
    }

    return -1;
}

int hal_ur_mesh_send_bcast_request(ur_mesh_hal_module_t *m,
                                   frame_t *frame,
                                   ur_mesh_handle_sent_bcast_t sent, void *context)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_send_bcast_request != NULL)) {
        return m->ur_mesh_hal_send_bcast_request(m, frame, sent, context);
    }

    return -1;
}

int hal_ur_mesh_register_receiver(ur_mesh_hal_module_t *m,
                                  ur_mesh_handle_received_frame_t received, void *context)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_register_receiver != NULL)) {
        return m->ur_mesh_hal_register_receiver(m, received, context);
    }

    return -1;
}

int hal_ur_mesh_start_beacons(ur_mesh_hal_module_t *m,
                              frame_t *data, mac_address_t *dest,
                              uint16_t max_interval)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_start_beacons != NULL)) {
        return m->ur_mesh_hal_start_beacons(m, data, dest, max_interval);
    }

    return -1;
}

int hal_ur_mesh_stop_beacons(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_stop_beacons != NULL)) {
        return m->ur_mesh_hal_stop_beacons(m);
    }

    return -1;
}

int hal_ur_mesh_set_bcast_mtu(ur_mesh_hal_module_t *m, uint16_t mtu)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_set_bcast_mtu != NULL)) {
        return m->ur_mesh_hal_set_bcast_mtu(m, mtu);
    }

    return -1;
}

int hal_ur_mesh_get_bcast_mtu(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_bcast_mtu != NULL)) {
        return m->ur_mesh_hal_get_bcast_mtu(m);
    }

    return -1;
}

int hal_ur_mesh_set_ucast_mtu(ur_mesh_hal_module_t *m, uint16_t mtu)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_set_ucast_mtu != NULL)) {
        return m->ur_mesh_hal_set_ucast_mtu(m, mtu);
    }

    return -1;
}

int hal_ur_mesh_get_ucast_mtu(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_ucast_mtu != NULL)) {
        return m->ur_mesh_hal_get_ucast_mtu(m);
    }

    return -1;
}

int hal_ur_mesh_set_bcast_channel(ur_mesh_hal_module_t *m, uint8_t channel)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_set_bcast_channel != NULL)) {
        return m->ur_mesh_hal_set_bcast_channel(m, channel);
    }

    return -1;
}

int hal_ur_mesh_get_bcast_channel(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_bcast_channel != NULL)) {
        return m->ur_mesh_hal_get_bcast_channel(m);
    }

    return -1;
}

int hal_ur_mesh_get_bcast_chnlist(ur_mesh_hal_module_t *m,
                                  const uint8_t **chnlist)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_bcast_chnlist != NULL)) {
        return m->ur_mesh_hal_get_bcast_chnlist(m, chnlist);
    }

    return -1;
}

int hal_ur_mesh_set_ucast_channel(ur_mesh_hal_module_t *m, uint8_t channel)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_set_ucast_channel != NULL)) {
        return m->ur_mesh_hal_set_ucast_channel(m, channel);
    }

    return -1;
}

int hal_ur_mesh_get_ucast_channel(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_ucast_channel != NULL)) {
        return m->ur_mesh_hal_get_ucast_channel(m);
    }

    return -1;
}

int hal_ur_mesh_get_ucast_chnlist(ur_mesh_hal_module_t *m,
                                  const uint8_t **chnlist)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_ucast_chnlist != NULL)) {
        return m->ur_mesh_hal_get_ucast_chnlist(m, chnlist);
    }

    return -1;
}

int hal_ur_mesh_set_txpower(ur_mesh_hal_module_t *m, int8_t txpower)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_set_txpower != NULL)) {
        return m->ur_mesh_hal_set_txpower(m, txpower);
    }

    return -1;
}

int hal_ur_mesh_get_txpower(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_txpower != NULL)) {
        return m->ur_mesh_hal_get_txpower(m);
    }

    return -1;
}

int hal_ur_mesh_set_meshnetid(ur_mesh_hal_module_t *m,
                              const meshnetid_t *meshnetid)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_set_meshnetid != NULL)) {
        return m->ur_mesh_hal_set_meshnetid(m, meshnetid);
    }

    return -1;
}

const meshnetid_t *hal_ur_mesh_get_meshnetid(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_meshnetid != NULL)) {
        return m->ur_mesh_hal_get_meshnetid(m);
    }

    return NULL;
}

int hal_ur_mesh_set_mac_address(ur_mesh_hal_module_t *m,
                                const mac_address_t *addr)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_set_mac_address != NULL)) {
        return m->ur_mesh_hal_set_mac_address(m, addr);
    }

    return -1;
}

const mac_address_t *hal_ur_mesh_get_mac_address(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_mac_address != NULL)) {
        return m->ur_mesh_hal_get_mac_address(m);
    }

    return NULL;
}

int hal_ur_mesh_set_key(struct ur_mesh_hal_module_s *m,
                        uint8_t index, uint8_t *key, uint8_t length)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_set_key != NULL)) {
        return m->ur_mesh_hal_set_key(m, index, key, length);
    }

    return -1;
}

const frame_stats_t *hal_ur_mesh_get_stats(ur_mesh_hal_module_t *m)
{
    if (m == NULL) {
        m = hal_ur_mesh_get_default_module();
    }

    if ((m != NULL) && (m->ur_mesh_hal_get_stats != NULL)) {
        return m->ur_mesh_hal_get_stats(m);
    }

    return NULL;
}
