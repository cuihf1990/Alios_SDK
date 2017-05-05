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

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <hal/hal.h>

static YOS_DLIST_HEAD(g_wifi_module);

static void free_old_ap_list(hal_wifi_module_t *module)
{
    module->status.ap_num = 0;
    free(module->status.ap_records);
    module->status.ap_records = NULL;
}

void hal_wifi_register_module(hal_wifi_module_t *module)
{
    dlist_add_tail(&module->base.list, &g_wifi_module);

    module->status.mode = HAL_WIFI_MODE_UNKNOWN;
}

int hal_wifi_set_mode(hal_wifi_module_t *m, hal_wifi_mode_t mode)
{
    int             ret;
    hal_wifi_mode_t cur;

    if (mode >= HAL_WIFI_MODE_CONFIG_MAX) {
        return -1;
    }

    cur = m->status.mode;
    if (cur != HAL_WIFI_MODE_UNKNOWN) {
        if (cur == mode)
            return 0;
    }

    if (!(ret = m->set_mode(m ,mode))) {
        m->status.mode = mode;
    }

    return ret;
}

hal_wifi_mode_t hal_wifi_get_mode(hal_wifi_module_t *m)
{
    return m->status.mode;
}

int hal_wifi_is_connected(hal_wifi_module_t *m)
{
    if (m == NULL)
        m = hal_wifi_get_default_module();
    if (m == NULL)
        return 0;
    if (m->status.mode != HAL_WIFI_MODE_STATION)
        return 0;
    return m->status.connected;
}

void hal_wifi_install_event(hal_wifi_module_t *m,
                                      const hal_wifi_event_cb_t *cb)
{
    m->ev_cb = cb;
}

int hal_wifi_connect(hal_wifi_module_t *m, char *ssid, char *password)
{
    int ret = -1;

    if (m->status.mode != HAL_WIFI_MODE_STATION) {
        return ret;
    }

    if (m->status.connected) {
        return ret;
    }

    ret = m->connect_prepare(m, ssid, password);

    return ret;
}

int hal_wifi_disconnect(hal_wifi_module_t *m)
{
    int ret = -1;

    if (m->status.mode != HAL_WIFI_MODE_STATION) {
        return ret;
    }

    if (m->status.connected) {
        ret = m->disconnect(m);
    }

    return ret;
}

int hal_wifi_scan(hal_wifi_module_t *m, void *_not_use_now)
{
    if (m->status.mode != HAL_WIFI_MODE_STATION) {
        return -1;
    }

    /* Free old list */
    free_old_ap_list(m);

    return m->scan(m, _not_use_now);
}

int hal_wifi_smartconfig(hal_wifi_module_t *m,
                                  hal_wifi_getset_cmd_t cmd)
{
    if (m->status.mode != HAL_WIFI_MODE_STATION) {
        return -1;
    }

    return m->getset_ops(m, cmd);
}

int hal_wifi_has_smartconfig(hal_wifi_module_t *m)
{
    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_CHECK_SUPPORT_SMART_CONFIG);
    }

    return 0;
}

int hal_wifi_enter_promiscuous(hal_wifi_module_t *m)
{
    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_PROMISCUOUS_START);
    }

    return -1;
}

int hal_wifi_leave_promiscuous(hal_wifi_module_t *m)
{
    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_PROMISCUOUS_STOP);
    }

    return -1;
}

int hal_wifi_get_channel(hal_wifi_module_t *m)
{
    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_GET_CHANNEL);
    }

    return -1;
}

int hal_wifi_set_channel(hal_wifi_module_t *m, int channel)
{
    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_SET_CHANNEL, channel);
    }

    return -1;
}

int hal_wifi_get_mac(hal_wifi_module_t *m, uint8_t mac[6])
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_GET_MAC_ADDRESS, m->status.mode, mac);
    }

    return -1;
}

int hal_wifi_get_ap_rssi(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_GET_AP_RSSI);
    }

    return -1;
}

int hal_wifi_set_auto_connect(hal_wifi_module_t *m, int auto_connect)
{
    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_SET_AUTO_CONNECT, auto_connect);
    }

    return -1;
}

int hal_wifi_get_broadcast_port(hal_wifi_module_t *m)
{
    if (m == NULL) {
        m = hal_wifi_get_default_module();
    }

    if (m->getset_ops != NULL) {
        return m->getset_ops(m, HAL_WIFI_GET_BROADCAST_PORT);
    }

    return -1;
}

hal_wifi_module_t *hal_wifi_get_default_module(void)
{
    hal_wifi_module_t *m = NULL;

    if (dlist_empty(&g_wifi_module)) {
        return NULL;
    }

    m = dlist_first_entry(&g_wifi_module, hal_wifi_module_t, base.list);

    return m;
}

int hal_wifi_init(void)
{
    int          err = 0;
    dlist_t *t;

    /* do low level init */
    dlist_for_each(t, &g_wifi_module) {
        hal_wifi_module_t *m = (hal_wifi_module_t*)t;
        m->init(m, NULL);
    }

    return err;
}

