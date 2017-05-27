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

#ifndef UMESH_TYPES_H
#define UMESH_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#include "hal/base.h"
#include "yos/list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ur_error_s {
   UR_ERROR_NONE          = 0,
   UR_ERROR_FAIL          = 1,
   UR_ERROR_BUSY          = 2,
   UR_ERROR_DROP          = 3,
   UR_ERROR_MEM           = 4,
   UR_ERROR_ROUTE         = 5,
   UR_ERROR_PARSE         = 6,
   UR_ERROR_ADDRESS_QUERY = 7,
} ur_error_t;

typedef enum media_type_s {
    MEDIA_TYPE_WIFI = 0,
    MEDIA_TYPE_BLE  = 1,
    MEDIA_TYPE_15_4 = 2,
} media_type_t;

typedef void *umessage_t;

#ifndef NULL
#define NULL (void *)0
#endif

enum {
    UR_IP6_ADDR_SIZE   = 16,

    IP6_UCAST_ADDR_NUM = 2,
    IP6_MCAST_ADDR_NUM = 1,
};

enum {
    SHORT_ADDR_SIZE = 2,
    EXT_ADDR_SIZE   = 8,

    MESHNETID_SIZE  = 6,
};

typedef struct ur_ip6_addr_s {
    union {
        uint8_t  m8[UR_IP6_ADDR_SIZE];
        uint16_t m16[UR_IP6_ADDR_SIZE / sizeof(uint16_t)];
        uint32_t m32[UR_IP6_ADDR_SIZE / sizeof(uint32_t)];
    };
} __attribute__((packed)) ur_ip6_addr_t;

typedef struct ur_ip6_prefix_s {
    ur_ip6_addr_t prefix;
    uint8_t       length;
} __attribute__((packed)) ur_ip6_prefix_t;

enum {
    UR_IP6_HLEN      = 40,
    UR_UDP_HLEN      = 8,
};

typedef struct ur_netif_ip6_address_s {
    ur_ip6_addr_t                 addr;
    uint8_t                       prefix_length;
    struct ur_netif_ip6_address_s *next;
} ur_netif_ip6_address_t;

typedef struct mac_address_s {
    union {
        uint64_t value;
        uint16_t short_addr;
        uint8_t  addr[EXT_ADDR_SIZE];
    };
    uint8_t len;
} mac_address_t;

typedef struct ur_addr_s {
    mac_address_t addr;
    uint16_t netid;
} ur_addr_t;

typedef struct meshnetid_s {
    uint8_t meshnetid[MESHNETID_SIZE];
    uint8_t len;
} meshnetid_t;

enum {
    MAX_KEY_SIZE      = 16,  // bytes

    INVALID_KEY_INDEX = 0xff,
    MASTER_KEY_INDEX  = 0,
    GROUP_KEY1_INDEX  = 1,
};

typedef struct mesh_key_s {
    uint8_t len;
    uint8_t key[MAX_KEY_SIZE];
} mesh_key_t;

typedef struct frame_s {
    uint8_t  *data;
    uint16_t len;
    uint8_t key_index;
} frame_t;

typedef struct frame_info_s {
    mac_address_t peer;
    uint8_t       channel;
    int8_t        rssi;
    int8_t        key_index;
} frame_info_t;

typedef struct channel_s {
    uint16_t channel;
    uint16_t wifi_channel;
    uint16_t hal_ucast_channel;
    uint16_t hal_bcast_channel;
} channel_t;

typedef ur_error_t (* umesh_raw_data_received)(ur_addr_t *src,
                                               uint8_t *payload,
                                               uint8_t length);

typedef struct frame_stats_s {
    uint32_t in_frames;
    uint32_t out_frames;
} frame_stats_t;

typedef struct ur_link_stats_s {
    uint32_t in_frames;
    uint32_t in_command;
    uint32_t in_data;
    uint32_t in_filterings;
    uint32_t in_drops;

    uint32_t out_frames;
    uint32_t out_command;
    uint32_t out_data;
    uint32_t out_errors;

    bool     sending;
    uint16_t sending_timeouts;
} ur_link_stats_t;

typedef struct ur_message_stats_s {
    uint16_t num;
    uint16_t fails;
} ur_message_stats_t;

typedef struct ur_mem_stats_s {
    uint32_t num;
} ur_mem_stats_t;

#ifdef __cplusplus
}
#endif

#endif  /* UMESH_TYPES_H */
