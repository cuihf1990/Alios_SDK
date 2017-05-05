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

/**
 * @file yoc/mesh.h
 * @brief YoC JavaScript APIs
 * @version since 5.6.1
 */

#ifndef YOC_MESH_API_H
#define YOC_MESH_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

enum {
    UR_IP6_HLEN      = 40,
    UR_UDP_HLEN      = 8,
};

enum {
    UR_IP6_ADDR_SIZE = 16,
};

typedef struct ur_ip6_addr_s {
    union {
        uint8_t  m8[UR_IP6_ADDR_SIZE];
        uint16_t m16[UR_IP6_ADDR_SIZE / sizeof(uint16_t)];
        uint32_t m32[UR_IP6_ADDR_SIZE / sizeof(uint32_t)];
    };
} __attribute__((packed)) ur_ip6_addr_t;

typedef struct ur_netif_ip6_address_s {
    ur_ip6_addr_t                 addr;
    uint8_t                       prefix_length;
    struct ur_netif_ip6_address_s *next;
} ur_netif_ip6_address_t;

/**
 * @brief get unicast address
 * @param None
 * @retval NULL interface not ready
 * @retval !=0 success
 */
const ur_netif_ip6_address_t *ur_mesh_get_ucast_addr(void);

/**
 * @brief get multicast address
 * @param None
 * @retval NULL interface not ready
 * @retval !=0 success
 */
const ur_netif_ip6_address_t *ur_mesh_get_mcast_addr(void);

#ifdef __cplusplus
}
#endif

#endif /* YOC_MESH_API_H */


