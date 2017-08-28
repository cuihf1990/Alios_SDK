/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "ota_transport.h"

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

int8_t ota_cancel_upgrade(message_arrived *msgCallback)
{
    return 0;
}


int8_t ota_sub_upgrade(message_arrived *msgCallback)
{
    return 0;
}

int8_t ota_pub_request(ota_request_params *request_parmas)
{
    return 0;
}

int8_t ota_sub_request_reply(message_arrived *msgCallback)
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

void platform_ota_set_version(char *version) {};

const char *platform_ota_get_version()
{
    return 0;
}

const char *platform_get_main_version()
{
    return 0;
}

const char *platform_get_dev_version()
{
    return 0;
}

void platform_set_dev_version(const char *dev_version) {};
