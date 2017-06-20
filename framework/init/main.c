#include <yos/kernel.h>
#include <yos/framework.h>
#include <vfs.h>
#include <kvmgr.h>
#include <vflash.h>

#include "yos/cli.h"

#ifdef MESH_GATEWAY_SERVICE
#include "gateway_service.h"
#endif

static void register_devices()
{
    int i;
    for(i = 0; i < 10; i++)
        vflash_register_partition(i);
}

extern void ota_service_init(void);

int yos_framework_init(void)
{
    vfs_init();
    vfs_device_init();

    register_devices();
    yos_kv_init();

    yos_loop_init();

    yos_cli_init();

#ifdef MESH_GATEWAY_SERVICE
    gateway_service_init();
#endif

    ota_service_init();

    return 0;
}

