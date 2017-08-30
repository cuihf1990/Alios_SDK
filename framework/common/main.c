/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <yos/kernel.h>
#include <yos/yos.h>
#include <vfs.h>
#include <kvmgr.h>
#include <vflash.h>

#include "yos/cli.h"

#ifdef MESH_GATEWAY_SERVICE
#include "gateway_service.h"
#endif

extern void ota_service_init(void);

int yos_framework_init(void)
{
#ifdef MESH_GATEWAY_SERVICE
    gateway_service_init();
#endif

    ota_service_init();

    return 0;
}

