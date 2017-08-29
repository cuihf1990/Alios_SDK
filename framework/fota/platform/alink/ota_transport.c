/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <cJSON.h>
#include <alink_export.h>
#include <device.h>

#include "ota_transport.h"
#include "alink_protocol.h"
#include "ota_log.h"

#define POST_OTA_STATUS_METHOD "ota/postDeviceUpgradeStatus"
#define POST_OTA_RESULT_METHOD "device.updateVersion"
#define POST_OTA_STATUS_DATA "{\"version\":\"%s\",\"step\":\"%d\",\"stepPercent\":\"%d\"}"
#define POST_OTA_RESULT_DATA "{\"uuid\" :\"%s\",\"version\":\"%s;APP2.0;OTA1.0\",\"description\":\"%s\"}"

/*
 *  "md5":"6B21342306D0F619AF97006B7025D18A",
    "resourceUrl":"http://otalink.alicdn.com/ALINKTEST_LIVING_LIGHT_ALINK_TEST/v2.0.0.1/uthash-master.zip",
    "size":"265694",
    "uuid":"35C858CFCD6A1FF9F734D749033A29A0",
    "version":"v2.0.0.1","zip":"0"
*/

int8_t parse_ota_requset(const char *request, int *buf_len, ota_request_params *request_parmas)
{
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
        cJSON_free(info);

        cJSON *uuid = cJSON_GetObjectItem(root, "uuid");
        if (!uuid) {
            OTA_LOG_E("resourceUrl get error.");
            goto parse_failed;
        }
        strncpy(response_parmas->device_uuid, uuid->valuestring,
                sizeof response_parmas->device_uuid);
        OTA_LOG_D(" response_parmas->device_uuid %s", response_parmas->device_uuid);

        cJSON *resourceUrl = cJSON_GetObjectItem(root, "resourceUrl");
        if (!resourceUrl) {
            OTA_LOG_E("resourceUrl get error.");
            goto parse_failed;
        }
        strncpy(response_parmas->download_url, resourceUrl->valuestring,
                sizeof response_parmas->download_url);
        OTA_LOG_D(" response_parmas->download_url %s",
                  response_parmas->download_url);

        cJSON *md5 = cJSON_GetObjectItem(root, "md5");
        if (!md5) {
            OTA_LOG_E("md5 get error.");
            goto parse_failed;
        }

        memset(response_parmas->md5, 0, sizeof response_parmas->md5);
        strncpy(response_parmas->md5, md5->valuestring, sizeof response_parmas->md5);

        OTA_LOG_E("md5 %s", response_parmas->md5);
        cJSON *size = cJSON_GetObjectItem(root, "size");
        if (!size) {
            OTA_LOG_E("size get error.");
            goto parse_failed;
        }

        response_parmas->frimware_size = size->valueint;

        cJSON *version = cJSON_GetObjectItem(root, "version");
        if (!version) {
            OTA_LOG_E("version get error.");
            goto parse_failed;
        }
        strncpy(response_parmas->primary_version, version->valuestring,
                sizeof response_parmas->primary_version);
        OTA_LOG_D(" response_parmas->primary_version %s",
                  response_parmas->primary_version);

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


int8_t parse_ota_cancel_response(const char *response, int buf_len, ota_response_params *response_parmas)
{
    cJSON *root = cJSON_Parse(response);
    if (!root) {
        OTA_LOG_E("Error before: [%s]\n", cJSON_GetErrorPtr());
        goto parse_failed;
    } else {
        char *info = cJSON_Print(root);
        OTA_LOG_D("root is %s", info);
        cJSON_free(info);

        cJSON *uuid = cJSON_GetObjectItem(root, "uuid");
        if (!uuid) {
            OTA_LOG_E("uuid get error.");
            goto parse_failed;
        }

        strncpy(response_parmas->device_uuid, uuid->valuestring,
                sizeof response_parmas->device_uuid);
        OTA_LOG_D(" response_parmas->device_uuid %s", response_parmas->device_uuid);
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


int8_t platform_ota_status_post(int status, int percent)
{
    int ret = -1;
    char buff[256];
    memset(&buff, 0x00, sizeof(buff));
    if (percent < 0 || percent > 100) {
        OTA_LOG_E("percent error !(status = %d, percent = %d)\n", status, percent);
        percent = 0;
    }
    const char *ota_version = (const char *)platform_ota_get_version();
    snprintf(buff, sizeof(buff), POST_OTA_STATUS_DATA, ota_version, status, percent);
    //OTA_LOG_D("%s",buff);
    ret = yos_cloud_report(POST_OTA_STATUS_METHOD, buff, NULL, NULL);
    OTA_LOG_D("alink_ota_status_post: %s, ret=%d", buff, ret);
    return ret;
}


int8_t platform_ota_result_post(void)
{
    int ret = -1;
    char buff[256] = {0};
    char *alink_version = NULL;

    alink_version = (char *)yos_malloc(64);
    //assert(alink_version, NULL);
    memset(alink_version, 0, 64);
    alink_get_sdk_version(alink_version, 64);

    snprintf(buff, sizeof buff, POST_OTA_RESULT_DATA, (char *)ota_get_id(), (const char *)platform_get_main_version(),
             alink_version);
    yos_free(alink_version);
    ret = yos_cloud_report(POST_OTA_RESULT_METHOD, buff, NULL, NULL);
    OTA_LOG_D("alink_ota_status_post: %s, ret=%d\n", buff, ret);
    return ret;
}

void platform_ota_set_version(char *version)
{
    config_set_ota_version(version);
}

const char *platform_ota_get_version()
{
    return config_get_ota_version();
}

const char *platform_get_main_version()
{
    return devinfo_get_version();
}

const char *platform_get_dev_version()
{
    return config_get_dev_version();
}

void platform_set_dev_version(const char *dev_version)
{
    config_set_dev_version((char *)dev_version);
}

int8_t ota_pub_request(ota_request_params * request_parmas)
{
    return 0;
}

int8_t ota_sub_request_reply(yos_cloud_cb_t msgCallback)
{
    return 0;
}

int8_t ota_sub_upgrade(yos_cloud_cb_t msgCallback)
{
    return yos_cloud_register_callback(ALINK_UPGRADE_DEVICE, msgCallback);
}


int8_t ota_cancel_upgrade(yos_cloud_cb_t msgCallback)
{
    return yos_cloud_register_callback(ALINK_CANCEL_UPGRADE_DEVICE, msgCallback);
}

extern char *config_get_main_uuid(void);

char *ota_get_id(void)
{
    return config_get_main_uuid();
}

void free_global_topic()
{
}



