#include <stdio.h>
#include <yos/cloud.h>

static yos_cloud_cb_t cbs[MAX_EVENT_TYPE];
static int (*report_backend)(const char *method, const char *json_buffer);

int yos_cloud_register_callback(int cb_type, yos_cloud_cb_t cb)
{
    if (cb_type >= MAX_EVENT_TYPE)
        return -1;

    cbs[cb_type] = cb;
    return 0;
}

int yos_cloud_report(const char *method,
                     const char *json_buffer,
                     void (*done_cb)(void *),
                     void *arg)
{
    if (report_backend == NULL)
        return -1;

    return report_backend(method, json_buffer);
}

void yos_cloud_register_backend(int (*report)(const char *method, const char *json_buffer))
{
    report_backend = report;
}

void yos_cloud_trigger(int cb_type, const char *json_buffer)
{
    if (cb_type >= MAX_EVENT_TYPE)
        return;

    cbs[cb_type](cb_type, json_buffer);
}

int yos_cloud_init(void)
{
}
