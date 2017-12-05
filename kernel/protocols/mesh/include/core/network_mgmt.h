/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UR_NETWORK_MGMT_H
#define UR_NETWORK_MGMT_H

#include "core/topology.h"

enum {
    DISCOVERY_RETRY_TIMES = 4,
    ACTIVE_DISCOVER_INTERVAL = 300000,  // 5 mins
};

typedef void (* discovered_handler_t)(neighbor_t *nbr);

ur_error_t handle_discovery_request(message_t *message);
ur_error_t handle_discovery_response(message_t *message);
void umesh_network_mgmt_init(void);
void umesh_network_mgmt_register_callback(discovered_handler_t handler);

#endif  /* UR_NETWORK_MGMT_H */

