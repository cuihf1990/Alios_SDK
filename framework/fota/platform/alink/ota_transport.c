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

#include "alink_export.h"
#include "ota_transport.h"
#include "ota_log.h"


/*
 *  "md5":"6B21342306D0F619AF97006B7025D18A",
    "resourceUrl":"http://otalink.alicdn.com/ALINKTEST_LIVING_LIGHT_ALINK_TEST/v2.0.0.1/uthash-master.zip",
    "size":"265694",
    "uuid":"35C858CFCD6A1FF9F734D749033A29A0",
    "version":"v2.0.0.1","zip":"0"
*/

int8_t parse_ota_requset(const char* request, int *buf_len, ota_request_params * request_parmas)
{
    return 0;
}

int8_t parse_ota_response(const char* response, int buf_len,ota_response_params * response_parmas)
{
    cJSON *root = cJSON_Parse(response);
    if (!root) {
        OTA_LOG_E("Error before: [%s]\n", cJSON_GetErrorPtr());
        goto parse_failed;
    } else {
        char* info = cJSON_Print(root);
        OTA_LOG_D("root is %s", info);
        cJSON_free(info);

        cJSON *uuid = cJSON_GetObjectItem(root, "uuid");
        if (!uuid) {
            OTA_LOG_E("resourceUrl back.");
            goto parse_failed;
        }
        strncpy(response_parmas->device_uuid, uuid->valuestring,
                       sizeof response_parmas->device_uuid);
        OTA_LOG_D(" response_parmas->device_uuid %s", response_parmas->device_uuid);

        cJSON *resourceUrl = cJSON_GetObjectItem(root, "resourceUrl");
        if (!resourceUrl) {
            OTA_LOG_E("resourceUrl back.");
            goto parse_failed;
        }
        strncpy(response_parmas->download_url, resourceUrl->valuestring,
                sizeof response_parmas->download_url);
        OTA_LOG_D(" response_parmas->download_url %s",
                response_parmas->download_url);

        cJSON *md5 = cJSON_GetObjectItem(root, "md5");
        if (!md5) {
            OTA_LOG_E("md5 back.");
            goto parse_failed;
        }

        strncpy(response_parmas->md5, md5->valuestring,
                sizeof response_parmas->md5);

        cJSON *size = cJSON_GetObjectItem(root, "size");
        if (!md5) {
            OTA_LOG_E("size back.");
            goto parse_failed;
        }

        response_parmas->frimware_size = size->valueint;

        cJSON *version = cJSON_GetObjectItem(root, "version");
        if (!resourceUrl) {
            OTA_LOG_E("resourceUrl back.");
            goto parse_failed;
        }
        strncpy(response_parmas->primary_version, version->valuestring,
                sizeof response_parmas->primary_version);
        OTA_LOG_D(" response_parmas->primary_version %s",
                response_parmas->primary_version);

    }

    OTA_LOG_D("parse_json success");
    goto parse_success;

parse_failed: if (root) {
        cJSON_Delete(root);
    }
    return -1;
parse_success: if (root) {
        cJSON_Delete(root);
    }
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

int8_t ota_sub_upgrade(message_arrived *msgCallback)
{
    return alink_register_callback(ALINK_UPGRADE_DEVICE,msgCallback);
}


int8_t ota_cancel_upgrade(message_arrived *msgCallback)
{
    return alink_register_callback(ALINK_CANCEL_UPGRADE_DEVICE,msgCallback);
}

extern char *config_get_main_uuid(void);

char* ota_get_id(void) {
   return config_get_main_uuid();
}

void free_global_topic()
{
}





