#include "yos/framework.h"
#include "yos/kernel.h"

#include "umesh_utils.h"

ur_timer_t ur_start_timer(uint32_t dt, timer_handler_t handler, void *args)
{
    yos_post_delayed_action(dt, handler, args);
    return handler;
}

void ur_stop_timer(ur_timer_t *timer, void *args)
{
    timer_handler_t handler;

    if (*timer != NULL) {
        handler = (timer_handler_t)(*timer);
        yos_cancel_delayed_action(-1, handler, args);
        *timer = NULL;
    }
}

uint32_t ur_get_now(void)
{
    uint64_t now;

    now = yos_now();
    now = now / 1000000;
    return (uint32_t)now;
}
