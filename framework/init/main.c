#include <yos/kernel.h>
#include <yos/framework.h>
#include <vfs.h>
#include <kvmgr.h>

#include "yos/cli.h"

int yos_framework_init(void)
{
    vfs_init();
    vfs_device_init();
    yos_kv_init();

    yos_loop_init();

    yos_cli_init();

    return 0;
}

