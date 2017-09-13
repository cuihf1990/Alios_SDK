/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UR_MM_H
#define UR_MM_H

#include "core/mesh_mgmt_tlvs.h"
#include "core/mesh_forwarder.h"
#include "core/topology.h"
#include "ip/ip6.h"
#include "hal/interface_context.h"

typedef enum device_state_s {
    DEVICE_STATE_DISABLED     = 0,
    DEVICE_STATE_DETACHED     = 1,
    DEVICE_STATE_ATTACHED     = 2,
    DEVICE_STATE_LEAF         = 3,
    DEVICE_STATE_LEADER       = 4,
    DEVICE_STATE_SUPER_ROUTER = 5,
    DEVICE_STATE_ROUTER       = 6,
} mm_device_state_t;

static inline uint16_t get_subnetsize_from_netinfo(mm_netinfo_tv_t *netinfo)
{
    return (netinfo->subnet_size_1 << 8) | netinfo->subnet_size_2;
}

static inline void set_subnetsize_to_netinfo(mm_netinfo_tv_t *netinfo,
                                             uint16_t subnetsize)
{
    netinfo->subnet_size_1 = (uint8_t)((subnetsize >> 8) & 0x7);
    netinfo->subnet_size_2 = (uint8_t)(subnetsize & 0xff);
}

typedef enum mm_command_s {
    COMMAND_COMMAND_MASK            = 0x7f,
    COMMAND_ADVERTISEMENT           = 0x1,
    COMMAND_DISCOVERY_REQUEST       = 0x2,
    COMMAND_DISCOVERY_RESPONSE      = 0x3,
    COMMAND_ATTACH_REQUEST          = 0x4,
    COMMAND_ATTACH_RESPONSE         = 0x5,
    COMMAND_SID_REQUEST             = 0x8,
    COMMAND_SID_RESPONSE            = 0x9,
    COMMAND_ADDRESS_QUERY           = 0xd,
    COMMAND_ADDRESS_QUERY_RESPONSE  = 0xe,
    COMMAND_ADDRESS_NOTIFICATION    = 0xf,
    COMMAND_LINK_REQUEST            = 0x10,
    COMMAND_LINK_ACCEPT             = 0x11,
    COMMAND_LINK_ACCEPT_AND_REQUEST = 0x12,
    COMMAND_ADDRESS_UNREACHABLE     = 0x14,
    COMMAND_ADDRESS_ERROR           = 0x15,
    COMMAND_ROUTING_INFO_UPDATE     = 0x16,

    // diags command
    COMMAND_TRACE_ROUTE_REQUEST     = 0x71,
    COMMAND_TRACE_ROUTE_RESPONSE    = 0x72,
} mm_command_t;

enum {
    ATTACH_REQUEST_RETRY_TIMES   = 2,
    ATTACH_SID_RETRY_TIMES       = 2,
    BECOME_LEADER_TIMEOUT        = 3,
    MIGRATE_TIMEOUT              = 2,
    DETACHED_MIGRATE_TIMEOUT     = 1,

#ifndef CONFIG_AOS_DDA
    ATTACH_CANDIDATE_TIMEOUT     = 30,     /* 30 * ADVERTISEMENT_TIMEOUT */
#else
    ATTACH_CANDIDATE_TIMEOUT     = 0,
#endif
};

enum {
    PATH_COST_WEIGHT       = 256,
    SIZE_WEIGHT            = 1,
    PATH_COST_SWITCH_HYST = 384,
};

typedef struct mesh_mgmt_header_s {
    uint8_t command;
} mm_header_t;

typedef struct mesh_network_info_s {
    uint8_t       stable_version;
    uint8_t       temp_version;
    uint16_t      netid;
    uint16_t      size;
    ur_ip6_addr_t mcast;
    uint16_t      timeout;
} mesh_network_info_t;

enum {
    ROUTING_ID_DEFAULT = 0,
};

typedef ur_error_t (* interface_up_t)(void);
typedef ur_error_t (* interface_down_t)(void);

typedef struct mm_cb_s {
    interface_up_t interface_up;
    interface_down_t interface_down;
} mm_cb_t;

ur_error_t umesh_mm_init(node_mode_t mode);
ur_error_t umesh_mm_start(mm_cb_t *cb);
ur_error_t umesh_mm_stop(void);

bool umesh_mm_migration_check(network_context_t *network, neighbor_t *nbr,
                              mm_netinfo_tv_t *netinfo);
void umesh_mm_start_net_scan_timer(void);
uint8_t umesh_mm_get_prev_channel(void);
void umesh_mm_set_prev_channel(void);

uint16_t            umesh_mm_get_local_sid(void);
neighbor_t          *umesh_mm_get_attach_node(network_context_t *network);
neighbor_t          *umesh_mm_get_attach_candidate(network_context_t *network);
uint8_t             *umesh_mm_get_local_ueid(void);
mm_device_state_t   umesh_mm_get_device_state(void);
attach_state_t      umesh_mm_get_attach_state(void);
uint16_t            umesh_mm_get_meshnetid(network_context_t *network);
uint16_t            umesh_mm_get_meshnetsize(void);
const mac_address_t *umesh_mm_get_mac_address(void);
node_mode_t         umesh_mm_get_mode(void);
int8_t              umesh_mm_compare_mode(node_mode_t local, node_mode_t other);
ur_error_t          umesh_mm_set_mode(node_mode_t mode);
uint16_t            umesh_mm_get_channel(network_context_t *network);
void                umesh_mm_set_channel(network_context_t *network,
                                         uint16_t channel);
ur_error_t          umesh_mm_set_seclevel(int8_t level);
int8_t              umesh_mm_get_seclevel(void);

void umesh_mm_get_extnetid(umesh_extnetid_t *extnetid);
ur_error_t umesh_mm_set_extnetid(const umesh_extnetid_t *extnetid);

uint8_t umesh_mm_get_leader_mode(void);
uint8_t umesh_mm_get_reboot_flag(void);

void       umesh_mm_init_tlv_base(mm_tlv_t *tlv, uint8_t type, uint8_t length);
void       umesh_mm_init_tv_base(mm_tv_t *tlv, uint8_t type);
mm_tv_t    *umesh_mm_get_tv(const uint8_t *data, const uint16_t length,
                            uint8_t type);
ur_error_t umesh_mm_handle_frame_received(message_t *message);

void become_leader(void);
void become_detached(void);

uint16_t tlvs_set_value(network_context_t *network,
                        uint8_t *buf, const uint8_t *tlvs,
                        uint8_t tlvs_length);
int16_t tlvs_calc_length(const uint8_t *tlvs, uint8_t tlvs_length);

void get_leader_addr(ur_addr_t *addr);
void set_mesh_short_addr(ur_addr_t *addr, uint16_t netid, uint16_t sid);
void set_mesh_ext_addr(ur_addr_t *addr, uint16_t netid, uint8_t *value);

static inline uint16_t mm_get_main_netid(network_context_t *network)
{
    return get_main_netid(umesh_mm_get_meshnetid(network));
}

static inline uint16_t mm_get_sub_netid(network_context_t *network)
{
    return get_sub_netid(umesh_mm_get_meshnetid(network));
}

static inline bool is_bcast_sid(ur_addr_t *addr)
{
    if (addr->addr.len == SHORT_ADDR_SIZE && addr->addr.short_addr == BCAST_SID) {
        return true;
    }
    return false;
}

uint8_t set_mm_header_type(message_info_t *info, uint8_t *data, uint8_t type);
uint8_t set_mm_netinfo_tv(network_context_t *network, uint8_t *data);
uint8_t set_mm_channel_tv(network_context_t *network, uint8_t *data);
uint8_t set_mm_ueid_tv(uint8_t *data, uint8_t type, uint8_t *ueid);
uint8_t set_mm_path_cost_tv(network_context_t *network, uint8_t *data);
uint8_t set_mm_node_id_tv(uint8_t *data, uint8_t type, ur_node_id_t *node);
uint8_t set_mm_sid_tv(uint8_t *data, uint8_t type, uint16_t sid);
uint8_t set_mm_mode_tv(uint8_t *data);
uint8_t set_mm_allocated_node_type_tv(uint8_t *data, uint8_t type);
uint8_t set_mm_mcast_tv(uint8_t *data);

#endif  /* UR_MM_H */
