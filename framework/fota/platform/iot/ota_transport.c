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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cJSON.h>

#include "ota_update_manifest.h"
#include "ota_log.h"
#include "ota_transport_platform.h"


#define METHOD_UPGRADE "/ota/device/upgrade/%s"
#define METHOD_CHECK "/ota/device/check/%s"
#define METHOD_CHECK_REPLY "/ota/device/check_reply/%s"

#define POSTOTA_CHECK "{\"id\":\"%s\",\"params\":{\"version\":\"%s\"}}"
#define POST_DATA_BUFFER_SIZE (512)
#define POST_METHOD_BUFFER_SIZE (256)
#define DOWNLOAD_URL_LEN  256

static char *g_check_topic;
static char *g_reply_topic;
static char *g_upgrad_topic;
char  post_data_buffer[POST_DATA_BUFFER_SIZE];

int8_t parse_ota_requset(const char *request, int *buf_len, ota_request_params *request_parmas)
{
    if (0 != request_parmas) {
        snprintf(post_data_buffer, POST_DATA_BUFFER_SIZE,
                 POSTOTA_CHECK, request_parmas->device_uuid, request_parmas->primary_version);
        request = post_data_buffer;
        *buf_len = strlen(post_data_buffer);
        OTA_LOG_D("post_data_buffer:%s, len :%d", post_data_buffer, *buf_len);
    } else {
        OTA_LOG_E("update_manifest_read_cb,  error!");
    }
    return 0;
}

int8_t parse_ota_response(const char *response, int buf_len, ota_response_params *response_parmas)
{
    cJSON *root = cJSON_Parse(response);
    if (!root) {
        OTA_LOG_E("Error before: [%s]\n", cJSON_GetErrorPtr());
        goto parse_failed;
    } else {
        char *info = cJSON_Print(root);
        OTA_LOG_D("root is %s", info);
        free(info);
        cJSON *json_obj = cJSON_GetObjectItem(root, "data");
        if (!json_obj) {
            OTA_LOG_E("data back.");
            goto parse_failed;
        }
        cJSON *resourceUrl = cJSON_GetObjectItem(json_obj, "url");
        if (!resourceUrl) {
            OTA_LOG_E("resourceUrl back.");
            goto parse_failed;
        }
        strncpy(response_parmas->download_url, resourceUrl->valuestring,
                sizeof response_parmas->download_url);
        OTA_LOG_D(" response_parmas->download_url %s",
                  response_parmas->download_url);

        cJSON *md5 = cJSON_GetObjectItem(json_obj, "md5");
        if (!md5) {
            OTA_LOG_E("md5 back.");
            goto parse_failed;
        }

        strncpy(response_parmas->md5, md5->valuestring,
                sizeof response_parmas->md5);

        cJSON *size = cJSON_GetObjectItem(json_obj, "size");
        if (!md5) {
            OTA_LOG_E("size back.");
            goto parse_failed;
        }

        response_parmas->frimware_size = size->valueint;

    }

    OTA_LOG_D("parse_json success");
    goto parse_success;

parse_failed:
    if (root) {
        cJSON_Delete(root);
    }
    return -1;
parse_success:
    if (root) {
        cJSON_Delete(root);
    }
    return 0;
}

int8_t ota_pub_request(ota_request_params *request_parmas)
{
    char *req_buf = NULL;
    int len = 0;

    g_check_topic =  yos_zalloc(POST_METHOD_BUFFER_SIZE) ;

    parse_ota_requset(req_buf, &len, request_parmas);

    snprintf(g_check_topic, POST_METHOD_BUFFER_SIZE, METHOD_CHECK, ota_get_iotId());
    return ota_pub_platform(g_check_topic, post_data_buffer, len);
}

int8_t ota_sub_upgrade(message_arrived *msgCallback)
{
    g_upgrad_topic =  yos_zalloc(POST_METHOD_BUFFER_SIZE);

    snprintf(g_upgrad_topic, POST_METHOD_BUFFER_SIZE, METHOD_UPGRADE, ota_get_iotId());
    return ota_sub_platform(g_upgrad_topic, 0, msgCallback);
}

int8_t ota_sub_request_reply(message_arrived *msgCallback)
{
    g_reply_topic =  yos_zalloc(POST_METHOD_BUFFER_SIZE);

    snprintf(g_reply_topic, POST_METHOD_BUFFER_SIZE, METHOD_CHECK_REPLY, ota_get_iotId());
    return ota_sub_platform(g_reply_topic, 0, msgCallback);
}

void free_global_topic()
{
    if (g_check_topic) {
        free(g_check_topic);
        g_check_topic = NULL;
    }

    if (g_reply_topic) {
        free(g_reply_topic);
        g_reply_topic = NULL;
    }

    if (g_upgrad_topic) {
        free(g_upgrad_topic);
        g_upgrad_topic = NULL;
    }

}





