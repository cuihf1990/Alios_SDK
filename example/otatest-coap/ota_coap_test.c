/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "aos/aos.h"

//extern void coap_ota();

int application_start(void)
{
    aos_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP, 0);
//	coap_ota();
    aos_loop_run();
    return 0;
}
