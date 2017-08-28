#include <yos/framework.h>

#include "hal/soc/soc.h"
#include "helloworld.h"

static void app_delayed_action(void *arg)
{
    printf("%s:%d %s\r\n", __func__, __LINE__, yos_task_name());
    yos_post_delayed_action(5000, app_delayed_action, NULL);
}

int application_start(int argc, char *argv[])
{
    yos_post_delayed_action(1000, app_delayed_action, NULL);
    yos_loop_run();
}

