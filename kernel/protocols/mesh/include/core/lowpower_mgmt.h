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
void lowpower_update_time_slot(neighbor_t *nbr, uint8_t *tlvs, uint16_t length);
uint16_t lowpower_set_time_slot(uint8_t *data);

#endif
