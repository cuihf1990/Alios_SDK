/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "ota_transport.h"

void platform_ota_init( void *signal)
{

}
int8_t parse_ota_requset(const char *request, int *buf_len, ota_request_params *request_parmas)
{
    return 0;
}

int8_t parse_ota_response(const char *buf, int buf_len, ota_response_params *response_parmas)
{
    return 0;
}

int8_t parse_ota_cancel_response(const char *response, int buf_len, ota_response_params *response_parmas)
{
    return 0;
}

int8_t ota_cancel_upgrade(yos_cloud_cb_t msgCallback)
{
    return 0;
}


int8_t ota_sub_upgrade(yos_cloud_cb_t msgCallback)
{
    return 0;
}

int8_t ota_pub_request(ota_request_params *request_parmas)
{
    return 0;
}

int8_t ota_sub_request_reply(yos_cloud_cb_t msgCallback)
{
    return 0;
}

void free_global_topic() {};

char *ota_get_id()
{
    return 0;
}

int8_t platform_ota_status_post(int status, int percent)
{
    return 0;
}

int8_t platform_ota_result_post(void)
{
    return 0;
}
