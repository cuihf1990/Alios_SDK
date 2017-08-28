/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UR_NETWORK_MGMT_H
#define UR_NETWORK_MGMT_H

#include "core/topology.h"

enum {
    DISCOVERY_RETRY_TIMES = 4,
};

ur_error_t handle_discovery_request(message_t *message);
ur_error_t handle_discovery_response(message_t *message);
ur_error_t nm_start_discovery(discovered_handler_t handler);
ur_error_t nm_stop_discovery(void);

#endif  /* UR_NETWORK_MGMT_H */

