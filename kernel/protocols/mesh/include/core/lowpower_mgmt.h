/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef UMESH_LOWPOWER_MGMT_H
#define UMESH_LOWPOWER_MGMT_H

#include "core/topology.h"

enum {
    SLOTS_SIZE = 3,
    SLOT_INTERVAL = 600,
};

void lowpower_init(void);
void lowpower_start(void);
void lowpower_stop(void);

#endif
