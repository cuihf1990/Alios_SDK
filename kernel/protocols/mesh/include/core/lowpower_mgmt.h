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

typedef struct time_slot_s {
    uint8_t slot_num;
    uint16_t offset;
} time_slot_t;

void lowpower_init(void);
void lowpower_update_info(neighbor_t *nbr, uint8_t *tlvs, uint16_t length);
uint16_t lowpower_set_info(uint8_t type, uint8_t *data, void *context);

typedef void (* lowpower_radio_down_t)(void);
typedef void (* lowpower_radio_up_t)(void);

typedef struct lowpower_events_handler_s {
    slist_t next;
    lowpower_radio_down_t radio_down;
    lowpower_radio_up_t radio_up;
} lowpower_events_handler_t;

void lowpower_register_callback(lowpower_events_handler_t *handler);

#endif
