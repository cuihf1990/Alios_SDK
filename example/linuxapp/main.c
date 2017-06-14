#include <stdio.h>
#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/network.h>
#include <strings.h>
#include <netmgr.h>
#include "aos_cli.h"

struct cookie {
    int flag;
};

static void app_delayed_action(void *arg)
{
    struct cookie *cookie = arg;
    struct hostent *hent = gethostbyname("www.taobao.com");
    printf("%s - %s\r\n", __func__, yos_task_name());
    if(hent) {
        printf("%s - %s\r\n", __func__, hent->h_name);
    }
    if (cookie->flag != 0) {
        yos_post_delayed_action(3000, app_delayed_action, arg);
    }
    else {
        yos_schedule_call(app_delayed_action, arg);
    }
    cookie->flag ++;
}

static void app_main_entry(void *arg)
{
    yos_post_delayed_action(1000, app_delayed_action, arg);
    yos_loop_run();
}

int application_start(void)
{
    struct cookie *cookie = yos_malloc(sizeof(*cookie));
    bzero(cookie, sizeof(*cookie));

    cli_init();

    netmgr_init();

    netmgr_start();

    yos_task_new("appmain", app_main_entry, cookie, 8192);
    return 0;
}

