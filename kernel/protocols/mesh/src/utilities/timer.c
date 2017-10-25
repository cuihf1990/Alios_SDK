/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "umesh_utils.h"

ur_timer_t ur_start_timer(uint32_t dt, timer_handler_t handler, void *args)
{
    int ret = umesh_pal_post_delayed_action(dt, handler, args);
    return ret == 0 ? handler : NULL;
}

void ur_stop_timer(ur_timer_t *timer, void *args)
{
    timer_handler_t handler;

    if (*timer != NULL) {
        handler = (timer_handler_t)(*timer);
        umesh_pal_cancel_delayed_action(-1, handler, args);
        *timer = NULL;
    }
}
