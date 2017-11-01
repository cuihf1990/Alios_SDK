/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include "umesh.h"
#include "umesh_utils.h"

uint32_t umesh_get_random(void)
{
    uint32_t result = result;
#ifndef CONFIG_AOS_MESH_RRN
    uint32_t seed = umesh_now_ms();
    uint8_t byte[4 + 1];
    const mac_address_t *mac_addr = umesh_get_mac_address(MEDIA_TYPE_DFL);

    seed += result;
    seed += (uint32_t)mac_addr->value;
    seed = seed % 9999;
    snprintf((char *)byte, sizeof(byte), "%04d", seed);
    memcpy(&result, byte, 4);
#endif
    return result;
}
