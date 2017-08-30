/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>

#include "umesh_hal.h"
#include "core/keys_mgr.h"
#include "core/crypto.h"
#include "hal/interfaces.h"
#include "hal/interface_context.h"

static uint8_t g_symmetric_key[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
};

ur_error_t calculate_one_time_key(uint8_t *key, uint32_t timestamp,
                                  const uint8_t *mac)
{
    ur_error_t error;
    uint8_t timestamp_expand[KEY_SIZE];
    uint8_t index;

    if (key == NULL || mac == NULL) {
        return UR_ERROR_MEM;
    }

    for (index = 0; index < KEY_SIZE / sizeof(uint32_t); index++) {
        memcpy(&timestamp_expand[index * sizeof(uint32_t)],
               (uint8_t *)&timestamp, sizeof(uint32_t));
    }

    for (index = 0; index < KEY_SIZE / EXT_ADDR_SIZE; index++) {
        memcpy(&key[index * EXT_ADDR_SIZE], mac, EXT_ADDR_SIZE);
    }

    error = umesh_aes_encrypt(timestamp_expand, KEY_SIZE,
                              key, KEY_SIZE, key);

    return error;
}

ur_error_t calculate_network_key(void)
{
    uint8_t network_key[KEY_SIZE];
    uint32_t now = ur_get_now();
    uint8_t index = 0;

    for (index = 0; index < KEY_SIZE / sizeof(uint32_t); index++) {
        memcpy(&network_key[index * sizeof(now)],
               (uint8_t *)&now, sizeof(now));
    }

    set_symmetric_key(GROUP_KEY1_INDEX, network_key, KEY_SIZE);

    return UR_ERROR_NONE;
}

ur_error_t set_symmetric_key(uint8_t key_index, uint8_t *payload,
                             uint8_t length)
{
    if (payload != NULL && length == sizeof(g_symmetric_key)) {
        memcpy(g_symmetric_key, payload, length);
        return UR_ERROR_NONE;
    }
    return UR_ERROR_FAIL;
}

const uint8_t *get_symmetric_key(uint8_t key_index)
{
    return g_symmetric_key;
}
