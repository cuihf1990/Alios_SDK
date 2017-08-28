#include <stdio.h>
#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/network.h>
#include <yos/cli.h>
#ifdef CONFIG_YOS_DDA
#include <dda.h>
#endif
#include "netmgr.h"
#include "umesh.h"
#include "umesh_hal.h"

static void app_delayed_action(void *arg)
{
    netmgr_init();
    netmgr_start(false);
}

static void app_main_entry(void *arg)
{
    yos_post_delayed_action(1000, app_delayed_action, arg);
    yos_loop_run();
}

int application_start(int argc, char **argv)
{
    const char *mode = argc > 1 ? argv[1] : "";

    if (strcmp(mode, "--mesh-node") == 0) {
#ifdef CONFIG_YOS_DDA
        dda_enable(atoi(argv[argc-1]));
        dda_service_init();
        dda_service_start();
#endif
    }
    else if (strcmp(mode, "--mesh-master") == 0) {
        yos_cli_stop();
#ifdef CONFIG_YOS_DDM
        ddm_run(argc, argv);
#endif
    }
    else {
        yos_task_new("meshappmain", app_main_entry, NULL, 8192);
    }

    return 0;
}

