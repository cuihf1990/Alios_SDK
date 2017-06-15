#include <yos/kernel.h>
#include <yos/framework.h>
#include <vfs.h>
#include <kvmgr.h>

#ifdef CONFIG_YOS_CLI
#include "yos_cli.h"
#endif

int yos_framework_init(void)
{
    vfs_init();
    vfs_device_init();
    yos_kv_init();

    yos_loop_init();

#ifdef CONFIG_YOS_CLI
    yos_cli_init();
#endif

    return 0;
}

