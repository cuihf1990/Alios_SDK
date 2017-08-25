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

#ifndef UR_MM_TLVS_H
#define UR_MM_TLVS_H

#include "ipv6/ip6.h"

/* mesh managememt TLV types. */
enum {
    TYPE_LENGTH_FIXED_FLAG = 0x80,
};

typedef enum {
    TYPE_VERSION           = 0x80,  /* version TLV */
    TYPE_SRC_UEID          = 0x81,  /* source UEID TLV */
    TYPE_DEST_UEID         = 0x82,  /* dest UEID TLV */
    TYPE_ATTACH_NODE_UEID  = 0x83,  /* attach node UEID TLV */
    TYPE_SRC_SID           = 0x84,  /* source sid TLV */
    TYPE_DEST_SID          = 0x85,  /* dest sid TLV */
    TYPE_ALLOCATE_SID      = 0x86,  /* newly allocated sid TLV */
    TYPE_ATTACH_NODE_SID   = 0x87,  /* attach node sid TLV */
    TYPE_MODE              = 0x88,  /* node mode TLV */
    TYPE_NETWORK_SIZE      = 0x89,  /* network size TLV */
    TYPE_PATH_COST         = 0x8a,  /* path cost TLV */
    TYPE_LINK_COST         = 0x8b,  /* link cost TLV */
    TYPE_LOWPAN_CONTEXT    = 0x8c,  /* lowpan context TLV */
    TYPE_ROUTE_TYPE        = 0x8d,  /* route type TLV */
    TYPE_SCAN_MASK         = 0x8e,  /* scan mask TLV */
    TYPE_SRC_MAC_ADDR      = 0x8f,  /* src MAC address TLV */
    TYPE_WEIGHT            = 0x92,  /* mesh partition weight */
    TYPE_NODE_TYPE         = 0x93,  /* node type TLV */
    TYPE_NETWORK_INFO      = 0x94,  /* packet sequence number */
    TYPE_MCAST_ADDR        = 0x95,  /* multicast address */
    TYPE_TARGET_UEID       = 0x96,  /* target ueid TLV */
    TYPE_TARGET_SID        = 0x97,  /* target sid TLV */
    TYPE_REVERSE_RSSI      = 0x98,  /* reverse RSSI */
    TYPE_FORWARD_RSSI      = 0x99,  /* forward RSSI */
    TYPE_SID_TYPE          = 0x9a,  /* SID allocate type */
    TYPE_ADDR_QUERY        = 0x9b,  /* address query type */
    TYPE_NODE_ID           = 0x9c,  /* node id */
    TYPE_ATTACH_NODE_ID    = 0x9d,  /* attach node id */
    TYPE_DEF_HAL_TYPE      = 0x9e,  /* default hal type */
    TYPE_TIMESTAMP         = 0x9f,  /* timestamp */
    TYPE_SYMMETRIC_KEY     = 0xa0,  /* symmetric key */
    TYPE_SSID_INFO         = 0xa1,  /* structured sid info */
    TYPE_STATE_FLAGS       = 0xa2,  /* node state flags */
    TYPE_UCAST_CHANNEL     = 0xa3,  /* ucast channel */
    TYPE_BCAST_CHANNEL     = 0xa4,  /* bcast channel */
    TYPE_MESH_PREFIX       = 0x0,   /* mesh prefix TLV */
    TYPE_TLV_REQUEST       = 0x1,   /* TLV requests TLV */
    TYPE_HEADER_IES_TERMINATOR = 0x7f,  /* header TLVs terminator */
    TYPE_INVALID           = 0xff,
} mm_tlv_type_t;

typedef struct mesh_mgmt_tlv_s {
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) mm_tlv_t;

typedef struct mesh_mgmt_tv_s {
    uint8_t type;
} __attribute__((packed)) mm_tv_t;

typedef struct mesh_mgmt_ueid_tv_s {
    mm_tv_t base;
    uint8_t ueid[8];
} __attribute__((packed)) mm_ueid_tv_t;

typedef struct mesh_mgmt_sid_tv_s {
    mm_tv_t base;
    uint16_t sid;
} __attribute__((packed)) mm_sid_tv_t;

typedef struct mesh_mgmt_mode_tv_s {
    mm_tv_t base;
    uint8_t mode;
} __attribute__((packed)) mm_mode_tv_t;

typedef struct mesh_mgmt_size_tv_s {
    mm_tv_t  base;
    uint16_t size;
} __attribute__((packed)) mm_size_tv_t;

typedef struct mesh_mgmt_scan_mask_tv_s {
    mm_tv_t base;
    uint8_t mask;
} __attribute__((packed)) mm_scan_mask_tv_t;

typedef struct mesh_mgmt_cost_tv_s {
    mm_tv_t  base;
    uint16_t cost;
} __attribute__((packed)) mm_cost_tv_t;

typedef struct mesh_mgmt_mac_addr_tv_s {
    mm_tv_t       base;
    mac_address_t addr;
} __attribute__((packed)) mm_mac_addr_tv_t;

typedef struct mesh_mgmt_weight_tv_s {
    mm_tv_t  base;
    uint16_t weight;
} __attribute__((packed)) mm_weight_tv_t;

typedef struct mesh_mgmt_node_type_tv_s {
    mm_tv_t base;
    uint8_t type;
} __attribute__((packed)) mm_node_type_tv_t;

typedef struct mesh_mgmt_netinfo_tv_s {
    mm_tv_t  base;
    uint8_t  stable_version;
    uint8_t  version;
    uint16_t size: 13;
    uint16_t subnet_size_1: 3;
    uint8_t subnet_size_2;
    uint8_t leader_mode;
} __attribute__((packed)) mm_netinfo_tv_t;

typedef struct mesh_mgmt_ssid_info_tv_s {
    mm_tv_t  base;
    uint16_t child_num;
    uint8_t  free_slots;
} __attribute__((packed)) mm_ssid_info_tv_t;

typedef struct mesh_mgmt_sid_type_tv_s {
    mm_tv_t base;
    uint8_t type;
} __attribute__((packed)) mm_sid_type_tv_t;

typedef struct mesh_mgmt_addr_query_tv_s {
    mm_tv_t base;
    uint8_t query_type;
} __attribute__((packed)) mm_addr_query_tv_t;

typedef struct mesh_mgmt_node_id_tv_s {
    mm_tv_t base;
    uint16_t meshnetid;
    uint16_t sid;
    uint8_t mode;
} __attribute__((packed)) mm_node_id_tv_t;

typedef struct mesh_mgmt_mcast_addr_tv_s {
    mm_tv_t       base;
    ur_ip6_addr_t mcast;
} __attribute__((packed)) mm_mcast_addr_tv_t;

typedef struct mesh_mgmt_rssi_tv_s {
    mm_tv_t base;
    int8_t rssi;
} __attribute__((packed)) mm_rssi_tv_t;

typedef struct mesh_mgmt_hal_type_s {
    mm_tv_t base;
    uint8_t type;
} __attribute__((packed)) mm_hal_type_tv_t;

typedef struct mesh_mgmt_timestamp_tv_s {
    mm_tv_t base;
    uint32_t timestamp;
} __attribute__((packed)) mm_timestamp_tv_t;

typedef struct mesh_mgmt_symmetric_key_tv_s {
    mm_tv_t base;
    uint8_t symmetric_key[16];
} __attribute__((packed)) mm_symmetric_key_tv_t;

typedef struct mesh_mgmt_state_flags_tv_s {
    mm_tv_t base;
    uint8_t flags;
} __attribute__((packed)) mm_state_flags_tv_t;

typedef struct mesh_mgmt_channel_tv_s {
    mm_tv_t base;
    uint8_t channel;
} __attribute__((packed)) mm_channel_tv_t;

typedef struct mesh_mgmt_prefix_tlv_s {
    mm_tlv_t        base;
    ur_ip6_prefix_t prefix;
} __attribute__((packed)) mm_prefix_tlv_t;

typedef struct mesh_mgmt_tlv_request_tlv_s {
    mm_tlv_t base;
} __attribute__((packed)) mm_tlv_request_tlv_t;

#endif  /* UR_MM_TLVS_H */
