/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UR_TOPOLOGY_H
#define UR_TOPOLOGY_H

#include "umesh_types.h"

enum {
    MAX_NEIGHBORS_NUM = 32,
};

enum {
    BCAST_NETID   = 0xffff,
    BCAST_SUB_NETID = 0xff,
    INVALID_NETID = 0xfffe,
};

#define is_bcast_netid(nid) ((nid) == BCAST_NETID)
#define is_bcast_subnetid(nid) ((nid) == BCAST_SUB_NETID)
#define is_invalid_netid(nid) ((nid) == INVALID_NETID)

enum {
    INFINITY_PATH_COST = 0xffff,

    RSSI_THRESHOLD = -75,  // dBm
};

typedef struct link_nbr_stats_s {
    uint16_t link_request;
    uint16_t link_accept;
    uint16_t link_cost;
    int8_t   forward_rssi;
    int8_t   reverse_rssi;
} link_nbr_stats_t;

typedef enum neighbor_state_s {
    STATE_INVALID,
    STATE_CANDIDATE,
    STATE_NEIGHBOR,
    STATE_PARENT,
    STATE_CHILD,
} neighbor_state_t;

enum {
    NBR_SID_CHANGED       = 1 << 0,
    NBR_DISCOVERY_REQUEST = 1 << 1,
    NBR_NETID_CHANGED     = 1 << 2,
    NBR_REBOOT            = 1 << 3,
    NBR_CHANNEL_CHANGED   = 1 << 4,
    NBR_LINK_ESTIMATED    = 1 << 5,
};

typedef enum node_type_s {
    LEAF_NODE      = 0x01,
    ROUTER_NODE    = 0x02,
} sid_node_type_t;

typedef struct ur_node_id_s {
    uint8_t         ueid[EXT_ADDR_SIZE];
    node_mode_t     mode;
    uint16_t        meshnetid;
    uint16_t        sid;
    uint16_t        attach_sid;
    sid_node_type_t type;
    uint8_t         timeout;
} ur_node_id_t;

typedef struct ssid_info_s {
    uint16_t child_num;
    uint8_t  free_slots;
} ssid_info_t;

#ifdef CONFIG_AOS_MESH_LOWPOWER
typedef struct time_slot_s {
    uint8_t slot_num;
    uint16_t offset;
} time_slot_t;
#endif

typedef struct neighbor_s {
    slist_t next;
    uint8_t mac[EXT_ADDR_SIZE];
    uint16_t netid;
    uint16_t sid;
    uint16_t path_cost;
    neighbor_state_t state: 4;
    node_mode_t mode;
    link_nbr_stats_t stats;
    uint8_t flags;
    uint8_t attach_candidate_timeout;
    ssid_info_t ssid_info;
    uint8_t channel;
    uint8_t *one_time_key;
#ifdef CONFIG_AOS_MESH_LOWPOWER
    time_slot_t time_slot;  // indicates time slot when wake up
#endif
    uint32_t last_heard;
} neighbor_t;

#define ueid64(ueid) ({ \
    uint64_t id64 = ueid[7];     \
    id64 <<= 8; id64 |= ueid[6]; \
    id64 <<= 8; id64 |= ueid[5]; \
    id64 <<= 8; id64 |= ueid[4]; \
    id64 <<= 8; id64 |= ueid[3]; \
    id64 <<= 8; id64 |= ueid[2]; \
    id64 <<= 8; id64 |= ueid[1]; \
    id64 <<= 8; id64 |= ueid[0]; \
    id64; \
})
#endif  /* UR_TOPOLOGY_H */
