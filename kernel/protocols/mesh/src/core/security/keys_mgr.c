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

#include "umesh_hal.h"
#include "core/keys_mgr.h"
#include "hal/interfaces.h"
#include "hal/interface_context.h"

static uint8_t g_master_key[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

static uint8_t g_group_key[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

ur_error_t set_master_key(const uint8_t *key, uint8_t length)
{
    slist_t *hals;
    hal_context_t *hal;

    if (key != NULL && length == sizeof(g_master_key)) {
        memcpy(g_master_key, key, length);
    }

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        hal_ur_mesh_set_key(hal->module, MASTER_KEY_INDEX, g_group_key, sizeof(g_group_key));
    }

    return UR_ERROR_NONE;
}

const uint8_t *get_master_key(uint8_t *length)
{
    return g_master_key;
}

ur_error_t set_group_key(uint8_t *payload, uint8_t length)
{
    slist_t *hals;
    hal_context_t *hal;
    ur_error_t error = UR_ERROR_NONE;

    if (payload != NULL && length == sizeof(g_group_key)) {
        memcpy(g_group_key, payload, length);
    }

    hals = get_hal_contexts();
    slist_for_each_entry(hals, hal, hal_context_t, next) {
        hal_ur_mesh_set_key(hal->module, GROUP_KEY1_INDEX, g_group_key, sizeof(g_group_key));
    }

    return error;
}

const uint8_t *get_group_key(void)
{
    return g_group_key;
}
