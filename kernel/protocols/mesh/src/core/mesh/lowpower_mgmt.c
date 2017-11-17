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
    bool enabled;
    uint8_t slot_num;  // indicates time slot when wake up again
    uint32_t timestamp; // indicates timestamp when wake up again
    ur_timer_t parent_wakeup_timer;
    ur_timer_t wakeup_timer;
    ur_timer_t parent_sleep_timer;
    ur_timer_t sleep_timer;
    mm_cb_t callback;
} lowpower_state_t;
lowpower_state_t g_lowpower_state;

static void handle_parent_wakeup_timer(void *args);
static void handle_wakeup_timer(void *args);

static uint8_t get_next_slot_num(const uint8_t *mac, uint8_t slot_num)
{
    uint8_t next_slot_num = 1;
    struct key_t {
        uint8_t mac[EXT_ADDR_SIZE];
        uint32_t slot_num;
    } key;

    memcpy(key.mac, mac, EXT_ADDR_SIZE);
    key.slot_num = (uint32_t)slot_num;
    next_slot_num = (umesh_get_hashword((uint32_t *)&key, sizeof(key) / 4, 0) % SLOTS_SIZE) + 1;
    return next_slot_num;
}

static void update_schedule_timer(schedule_type_t type)
{
    uint8_t slot;
    uint8_t next_slot;
    int32_t offset;
    neighbor_t *attach;
    const uint8_t *mac;

    assert(type == SCHEDULE || type == PARENT_SCHEDULE);
    schedule_type_t *schedule_type = ur_mem_alloc(sizeof(schedule_type_t));
    *schedule_type = type;
    if (type == SCHEDULE) {
        mac = (umesh_get_mac_address(MEDIA_TYPE_DFL))->addr;
        slot = g_lowpower_state.slot_num;
        next_slot = get_next_slot_num(mac, slot);
        offset = (next_slot + SLOTS_SIZE - slot - 1) * SLOT_INTERVAL;
        g_lowpower_state.wakeup_timer = ur_start_timer(offset, handle_wakeup_timer, schedule_type);
        g_lowpower_state.slot_num = next_slot;
        g_lowpower_state.timestamp = umesh_now_ms();
        MESH_LOG_DEBUG("wakeup at own schedule, slot %d, at %d",
                        g_lowpower_state.slot_num, g_lowpower_state.timestamp);
    } else {
        attach = umesh_mm_get_attach_node();
        slot = attach->time_slot.slot_num;
        next_slot = get_next_slot_num(attach->mac, slot);
        offset = (next_slot + SLOTS_SIZE - slot - 1) * SLOT_INTERVAL;
        offset -= attach->time_slot.offset;
        g_lowpower_state.parent_wakeup_timer = ur_start_timer(offset, handle_parent_wakeup_timer, schedule_type);
        attach->time_slot.slot_num = next_slot;
        attach->time_slot.offset = 0;
        MESH_LOG_DEBUG("wakeup at parent schedule, slot %d, at %d",
                        attach->time_slot.slot_num, umesh_now_ms() + offset);
    }
}

static void sleep_timer_handler(void *args)
{
    schedule_type_t type = *(schedule_type_t *)args;

    ur_mem_free(args, sizeof(schedule_type_t));
    if (g_lowpower_state.enabled == true) {
        update_schedule_timer(type);
        if ((umesh_get_mode() & MODE_RX_ON) == 0 && g_lowpower_state.sleep_timer == NULL &&
            g_lowpower_state.parent_sleep_timer == NULL) {
            MESH_LOG_DEBUG("radio goto sleep at %d", umesh_now_ms());
            umesh_pal_sleep();
        }
    }
}

static void handle_parent_sleep_timer(void *args)
{
    g_lowpower_state.parent_sleep_timer = NULL;
    sleep_timer_handler(args);
}

static void handle_sleep_timer(void *args)
{
    g_lowpower_state.sleep_timer = NULL;
    sleep_timer_handler(args);
}

static void wakeup_timer_handler(void *args)
{
    schedule_type_t type = *(schedule_type_t *)args;

    if ((umesh_get_mode() & MODE_RX_ON) == 0) {
        MESH_LOG_DEBUG("radio wakeup at %d", umesh_now_ms());
        umesh_pal_wakeup();
    }

    if (type == SCHEDULE) {
        g_lowpower_state.sleep_timer = ur_start_timer(SLOT_INTERVAL,
                                                      handle_sleep_timer, args);
    } else {
        g_lowpower_state.parent_sleep_timer = ur_start_timer(SLOT_INTERVAL,
                                                             handle_parent_sleep_timer, args);
    }
}

static void handle_parent_wakeup_timer(void *args)
{
    wakeup_timer_handler(args);
}

static void handle_wakeup_timer(void *args)
{
    wakeup_timer_handler(args);
}

static ur_error_t mesh_interface_up(void)
{
    uint8_t state = umesh_get_device_state();

    if ((umesh_mm_get_mode() & MODE_RX_ON) == 0 &&
        state != DEVICE_STATE_LEADER && state != DEVICE_STATE_SUPER_ROUTER) {
        MESH_LOG_DEBUG("lowpower interface up");
        update_schedule_timer(PARENT_SCHEDULE);
    }
    return UR_ERROR_NONE;
}

static ur_error_t mesh_interface_down(void)
{
    MESH_LOG_DEBUG("lowpower interface down");
    ur_stop_timer(&g_lowpower_state.parent_wakeup_timer, NULL);
    return UR_ERROR_NONE;
}

void lowpower_init(void)
{
    g_lowpower_state.enabled = false;
    g_lowpower_state.callback.interface_up = mesh_interface_up;
    g_lowpower_state.callback.interface_down = mesh_interface_down;
    umesh_mm_register_callback(&g_lowpower_state.callback);
}

void lowpower_start(void)
{
    g_lowpower_state.enabled = true;
    g_lowpower_state.slot_num = (((uint8_t)umesh_get_random()) % SLOTS_SIZE + 1);
    g_lowpower_state.timestamp = umesh_now_ms();
    update_schedule_timer(SCHEDULE);
}

void lowpower_stop(void)
{
    g_lowpower_state.enabled = false;
    ur_stop_timer(&g_lowpower_state.parent_wakeup_timer, NULL);
    ur_stop_timer(&g_lowpower_state.wakeup_timer, NULL);
}

void lowpower_update_time_slot(neighbor_t *nbr, uint8_t *tlvs, uint16_t length)
{
    mm_time_slot_tv_t *time_slot;

    time_slot = (mm_time_slot_tv_t *)umesh_mm_get_tv(tlvs, length, TYPE_TIME_SLOT);
    if (time_slot) {
        nbr->time_slot.slot_num = time_slot->slot_num;
        nbr->time_slot.offset = time_slot->offset;
    }
}

uint16_t lowpower_set_time_slot(uint8_t *data)
{
    mm_time_slot_tv_t *time_slot;

    time_slot = (mm_time_slot_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)time_slot, TYPE_TIME_SLOT);
    time_slot->slot_num = g_lowpower_state.slot_num;
    time_slot->offset = umesh_now_ms() - g_lowpower_state.timestamp;
    return sizeof(mm_time_slot_tv_t);
}
