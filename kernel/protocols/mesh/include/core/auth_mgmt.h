/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UR_AUTH_MGMT_H
#define UR_AUTH_MGMT_H

#include "core/topology.h"
#include "hal/interface_context.h"
#include "umesh_types.h"
#include "tfs.h"

enum {
    TFS_CHALLENGE_LEN = 32,
    TFS_AUTH_CODE_LEN = 100,
};

typedef struct auth_context_s {
    bool is_auth_enable;
    bool is_auth_busy;
    bool is_auth_success;

    uint8_t id2_challenge[TFS_CHALLENGE_LEN];
    uint8_t local_id2[TFS_ID2_LEN+1];
    uint32_t local_id2_len;
    uint8_t peer_id2[TFS_ID2_LEN+1];
    uint8_t auth_code[TFS_AUTH_CODE_LEN];
    uint32_t auth_code_len;
    ur_addr_t peer;
    bool peer_auth_result;

    neighbor_t *auth_candidate;
    auth_state_t auth_state;
    ur_timer_t auth_timer;
    uint8_t auth_retry_times;
    auth_handler_t auth_handler;
} auth_context_t;

bool is_auth_enabled(void);
bool is_auth_busy(void);
void auth_enable(void);
void auth_disable(void);
ur_error_t auth_init(void);
neighbor_t *get_auth_candidate(void);
auth_state_t get_auth_state(void);
void set_auth_state(auth_state_t state);
bool get_auth_result(void);

ur_error_t handle_auth_request(message_t *message);
ur_error_t handle_auth_response(message_t *message);
ur_error_t handle_auth_relay(message_t *message);
ur_error_t handle_auth_ack(message_t *message);

ur_error_t send_auth_request(network_context_t *netowrk);
ur_error_t send_auth_response(network_context_t *network, ur_addr_t *dest);
ur_error_t send_auth_relay(network_context_t *network, ur_addr_t *dest);
ur_error_t send_auth_ack(network_context_t *netowrk);

ur_error_t auth_start(neighbor_t *nbr);
ur_error_t nm_start_auth(auth_handler_t handler);
ur_error_t nm_stop_auth(void);

#endif  /* UR_AUTH_MGMT_H */
