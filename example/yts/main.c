/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdlib.h>
#include <string.h>

#include <yts.h>
#include <kvmgr.h>

#ifdef CONFIG_AOS_YTS_ALL
#include <dda.h>
#endif

int application_start(int argc, char **argv)
{
#ifdef CONFIG_AOS_YTS_ALL
    const char *mode = argc > 1 ? argv[1] : "";
    if (strcmp(mode, "--mesh-node") == 0) {
        dda_enable(atoi(argv[argc-1]));
        dda_service_init();
        dda_service_start();
        return 0;
    }
    else if (strcmp(mode, "--mesh-master") == 0) {
        ddm_run(argc, argv);
        return 0;
    }
#endif

    yts_run(argc, argv);
    aos_kv_deinit();
    exit(0);
}

