/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>

#include <assert.h>

#include "umesh.h"
#include "umesh_utils.h"
#include "core/mesh_mgmt.h"
#include "core/lowpower_mgmt.h"
#include "utilities/logging.h"

typedef enum {
    PARENT_SCHEDULE = 0,
    SCHEDULE = 1,
} schedule_type_t;

typedef struct lowpower_state_s {
    time_slot_t schedule;  // indicates time slot when wake up again
    ur_timer_t parent_wakeup_timer;
    ur_timer_t wakeup_timer;
    ur_timer_t goto_sleep_timer;
    mm_cb_t callback;
} lowpower_state_t;
lowpower_state_t g_lowpower_state;

static void handle_wakeup_timer(void *args);

static uint8_t get_next_slot_num(const mac_address_t *mac, uint8_t slot_num)
{
    uint8_t next_slot_num = 1;
    struct key_t {
        uint8_t mac[EXT_ADDR_SIZE];
        uint32_t slot_num;
    } key;

    memcpy(key.mac, mac->addr, EXT_ADDR_SIZE);
    key.slot_num = (uint32_t)slot_num;
    next_slot_num = (umesh_get_hashword((uint32_t *)&key, sizeof(key) / 4, 0) % SLOTS_SIZE) + 1;
    return next_slot_num;
}

static void update_schedule_timer(const mac_address_t *mac, schedule_type_t type,
                                  void (* handle_timer)(void *args))
{
    uint8_t slot;
    uint8_t next_slot;
    int16_t offset;

    assert(type == SCHEDULE);
    slot = g_lowpower_state.schedule.slot_num;

    next_slot = get_next_slot_num(mac, slot);
    offset = next_slot + SLOTS_SIZE - slot - 1;
    g_lowpower_state.wakeup_timer = ur_start_timer(offset * SLOT_INTERVAL, handle_timer, NULL);
    g_lowpower_state.schedule.slot_num = next_slot;
    g_lowpower_state.schedule.offset = 0;
}

static void handle_goto_sleep_timer(void *args)
{
    update_schedule_timer(umesh_get_mac_address(MEDIA_TYPE_DFL), SCHEDULE,
                          handle_wakeup_timer);

    MESH_LOG_DEBUG("radio goto sleep at %d, wakeup at slot %d, offset %d",
                  umesh_now_ms(), g_lowpower_state.schedule.slot_num, g_lowpower_state.schedule.offset);

    umesh_pal_sleep();
}

static void handle_wakeup_timer(void *args)
{
    MESH_LOG_DEBUG("radio wakeup at %d", umesh_now_ms());
    umesh_pal_wakeup();
    g_lowpower_state.goto_sleep_timer = ur_start_timer(SLOT_INTERVAL,
                                                       handle_goto_sleep_timer, NULL);
}

static ur_error_t mesh_interface_up(void)
{
    MESH_LOG_DEBUG("lowpower interface up");
    return UR_ERROR_NONE;
}

static ur_error_t mesh_interface_down(void)
{
    MESH_LOG_DEBUG("lowpower interface up");
    return UR_ERROR_NONE;
}

void lowpower_init(void)
{
    g_lowpower_state.callback.interface_up = mesh_interface_up;
    g_lowpower_state.callback.interface_down = mesh_interface_down;
    umesh_mm_register_callback(&g_lowpower_state.callback);
}

void lowpower_start(void)
{
    g_lowpower_state.schedule.slot_num = (((uint8_t)umesh_get_random()) % SLOTS_SIZE + 1);
    g_lowpower_state.schedule.offset = 0;
    update_schedule_timer(umesh_get_mac_address(MEDIA_TYPE_DFL), SCHEDULE,
                          handle_wakeup_timer);
    umesh_pal_sleep();
}

void lowpower_stop(void)
{
    ur_stop_timer(g_lowpower_state.parent_wakeup_timer, NULL);
    ur_stop_timer(g_lowpower_state.wakeup_timer, NULL);
    ur_stop_timer(g_lowpower_state.goto_sleep_timer, NULL);
}
