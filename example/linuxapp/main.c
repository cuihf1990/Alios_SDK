#include <stdio.h>
#include <yos/kernel.h>
#include <yos/framework.h>

static void app_delayed_action(void *arg)
{
    int *pflag = arg;
    printf("%s - %s\n", __func__, g_active_task->task_name);
    if (*pflag == 0) {
        yos_post_delayed_action(1000, app_delayed_action, arg);
    }
    else if (*pflag == 1) {
        yos_schedule_call(app_delayed_action, arg);
    }
    else {
        yloop_terminate();
    }
    (*pflag) ++;
}

static void app_main_entry(void *arg)
{
    static int flag;
    printf("%s:%d - %s\n", __func__, __LINE__, g_active_task->task_name);
    yos_post_delayed_action(1000, app_delayed_action, &flag);
    yos_event_loop_run();
    printf("%s:%d - %s\n", __func__, __LINE__, g_active_task->task_name);
    yos_event_loop_run();
    printf("%s:%d - %s\n", __func__, __LINE__, g_active_task->task_name);
}

static void action_after_terminated(void *arg)
{
    printf("%s - %s\n", __func__, g_active_task->task_name);
}

static void app_second_entry(void *arg)
{
    static int flag;
    printf("%s - %s\n", __func__, g_active_task->task_name);
    yloop_init();
    local_event_service_init();
    yos_post_delayed_action(1000, app_delayed_action, &flag);
    yos_event_loop_run();
    yloop_destroy();
    printf("%s - %s quit\n", __func__, g_active_task->task_name);

    yos_schedule_call(action_after_terminated, NULL);
}

int application_start(void)
{
    ktask_t *task;
    printf("%s\n", __func__);
    yos_task_new("appmain", app_main_entry, NULL, 8192);
    yos_task_new("appsecond", app_second_entry, NULL, 8192);
    return 0;
}

