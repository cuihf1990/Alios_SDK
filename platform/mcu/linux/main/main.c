/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <yos/log.h>
#include <yos/kernel.h>

#include <arg_options.h>


#define TAG "main"

#ifdef TFS_EMULATE
extern int tfs_emulate_id2_index;
#endif

extern void yunos_lwip_init(int enable_tapif);
/* check in gcc sources gcc/gcov-io.h for the prototype */
extern void __gcov_flush(void);
extern void rl_free_line_state(void);
extern void rl_cleanup_after_signal(void);
extern void hw_start_hal(void);
extern void trace_start();
extern void netmgr_init(void);
extern int yos_framework_init(void);
extern int yos_cli_init(void);

static options_t options = { 0 };

static void yos_features_init(void);
static void signal_handler(int signo);
static int  setrlimit_for_vfs(void);
extern int application_start(int argc, char **argv);

static void exit_clean(void)
{
    char fn[64] = {0};
    snprintf(fn, sizeof(fn), "rm -f ./yos_partition_%d_*", getpid());
    system(fn);
}

static void app_entry(void *arg)
{
    int i = 0;

    yos_features_init();

    hw_start_hal();

    vfs_init();
    vfs_device_init();

    for(i = 0; i < 10; i++) {
        vflash_register_partition(i);
    }

#ifdef CONFIG_YOS_CLI
    yos_cli_init();
#endif

    yos_kv_init();
    yos_loop_init();

    yos_framework_init();

#ifdef VCALL_RHINO
    trace_start();    
#endif

    application_start(options.argc, options.argv);
}

static void start_app()
{
    yos_task_new("app", app_entry, NULL, 8192);
}

int csp_get_args(const char ***pargv)
{
    *pargv = (const char **)options.argv;
    return options.argc;
}

void yos_features_init(void)
{
#ifdef CONFIG_NET_LWIP
    if (options.lwip.enable) {
        yunos_lwip_init(options.lwip.tapif);
    }
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

int main(int argc, char **argv)
{
    int ret;

    setvbuf(stdout, NULL, _IONBF, 0);

    options.argc        = argc;
    options.argv        = argv;
    options.lwip.enable = true;
#if defined(TAPIF_DEFAULT_OFF) || !defined(WITH_LWIP)
    options.lwip.tapif  = false;
#else
    options.lwip.tapif  = true;
#endif
    options.log_level   = YOS_LL_WARN;

#if defined(CONFIG_YOS_DDA) || defined(ENABLE_GCOV)
    signal(SIGINT, signal_handler);
#endif
#ifdef CONFIG_YOS_UT
    signal(SIGPIPE, SIG_IGN);
#endif

    atexit(exit_clean);

    yunos_init();

    ret = setrlimit_for_vfs();
    if (ret != 0) {
        return ret;
    }

    parse_options(&options);

    yos_set_log_level(options.log_level);

#ifdef TFS_EMULATE
    tfs_emulate_id2_index = options.id2_index;
#endif

    start_app(argc, argv);

    yunos_start();

    return ret;
}

int board_cli_init(void)
{
    return 0;
}

