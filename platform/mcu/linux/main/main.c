/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/log.h>
#include <arg_options.h>
#include <vfs.h>
#include <vflash.h>
#include <kvmgr.h>

#define TAG "main"

#ifdef TFS_EMULATE
extern int tfs_emulate_id2_index;
#endif

extern void csp_os_init(void);
extern void csp_os_start(void);
extern void yunos_lwip_init(int enable_tapif);
/* check in gcc sources gcc/gcov-io.h for the prototype */
extern void __gcov_flush(void);
extern void rl_free_line_state(void);
extern void rl_cleanup_after_signal(void);
extern void lpm_init(void);
extern void hw_start_hal(void);
extern void netmgr_init(void);
extern void trace_start(int flag);

static options_t options = { 0 };

static void yos_features_init(void);
static void signal_handler(int signo);
static int  setrlimit_for_vfs(void);

int csp_get_args(const char ***pargv)
{
    *pargv = (const char **)options.argv;
    return options.argc;
}

void yos_features_init(void)
{
#ifdef WITH_LWIP
    if (options.lwip.enable) {
        yunos_lwip_init(options.lwip.tapif);
    }
#endif

#ifdef CONFIG_YOS_LPM
    lpm_init();
#endif
}

void signal_handler(int signo)
{
    LOGD(TAG, "Received signal %d\n", signo);

#ifdef ENABLE_GCOV
    __gcov_flush();
#endif

#ifdef CONFIG_YOS_DDA
    rl_free_line_state ();
    rl_cleanup_after_signal ();
#endif

    exit(0);
}

#define ARCH_MAX_NOFILE 64
int setrlimit_for_vfs(void)
{
    int           ret;
    struct rlimit rlmt;

    getrlimit(RLIMIT_NOFILE, &rlmt);
    if (rlmt.rlim_cur > ARCH_MAX_NOFILE) {
        rlmt.rlim_cur = ARCH_MAX_NOFILE;
        ret = setrlimit(RLIMIT_NOFILE, &rlmt);
        if (ret != 0) {
            LOGE(TAG, "setrlimit error: %s", strerror(errno));
            return ret;
        }
    }
    LOGD(TAG, "Limit max open files to %d", (int)rlmt.rlim_cur);

    return 0;
}

extern int application_start(int argc, char **argv);

static void app_entry(void *arg)
{
    application_start(options.argc, options.argv);
}

static void start_app()
{
    yos_task_new("app", app_entry, NULL, 8192);
}

static void register_devices(void)
{
    int i;
    for (i=0;i<10;i++)
        vflash_register_partition(i);
}

int main(int argc, char **argv)
{
    int ret;

    setvbuf(stdout, NULL, _IONBF, 0);

    options.argc        = argc;
    options.argv        = argv;
    options.lwip.enable = true;
#ifdef TAPIF_DEFAULT_OFF
    options.lwip.tapif  = false;
#else
    options.lwip.tapif  = true;
#endif
    options.log_level   = YOS_LL_WARN;

    signal(SIGINT, signal_handler);
#ifdef CONFIG_YOS_UT
    signal(SIGPIPE, SIG_IGN);
#endif

    ret = setrlimit_for_vfs();
    if (ret != 0) {
        return ret;
    }

    parse_options(&options);

    yos_set_log_level(options.log_level);

#ifdef TFS_EMULATE
    tfs_emulate_id2_index = options.id2_index;
#endif

    csp_os_init();

    yos_features_init();

    vfs_init();
    vfs_device_init();
    register_devices();

    yos_kv_init();

    hw_start_hal();
    netmgr_init();

    yos_loop_init();

    trace_start(options.trace_flag);

    start_app(argc, argv);

    csp_os_start();

    return ret;
}

