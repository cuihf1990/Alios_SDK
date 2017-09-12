/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <aos/aos.h>
#include <vfs.h>
#include <kvmgr.h>

#ifdef MESH_GATEWAY_SERVICE
#include "gateway_service.h"
#endif

extern void ota_service_init(void);

int aos_framework_init(void)
{
#ifdef MESH_GATEWAY_SERVICE
    gateway_service_init();
#endif

    ota_service_init();

    return 0;
}

