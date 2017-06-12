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

#ifndef _OTA_UTIL_H_
#define _OTA_UTIL_H_

#ifdef __cplusplus
extern "C"{
#endif
#include <stdint.h>

typedef enum {
    OTA_FAILED_REBOOT = -7,
    OTA_REBOOT_FAILED = -6,
    OTA_UPGRADE_FAILED = -5,
    OTA_CHECK_FAILED = -4,
    OTA_DECOMPRESS_FAILED = -3,
    OTA_DOWNLOAD_FAILED = -2,
    OTA_INIT_FAILED = -1,
    OTA_INIT = 0,
    OTA_DOWNLOAD = 1,
    OTA_DECOMPRESS = 2,
    OTA_CHECK = 3,
    OTA_UPGRADE = 4,
    OTA_REBOOT = 5,
    OTA_REBOOT_SUCCESS = 6,
    OTA_CANCEL = 7,
    OTA_MAX
} OTA_STATUS_T;

typedef enum {
    OTA_DOWNLOAD_RECV_FAIL = -6,
    OTA_DOWNLOAD_SEND_FAIL = -5,
    OTA_DOWNLOAD_SOCKET_FAIL = -4,
    OTA_DOWNLOAD_IP_FAIL = -3,
    OTA_DOWNLOAD_URL_FAIL = -2,
    OTA_DOWNLOAD_FAIL = -1,
    OTA_DOWNLOAD_CONTINUE = 0,
    OTA_DOWNLOAD_CANCEL = 1,
    OTA_DOWNLOAD_FINISH = 2
}OTA_DOWNLOAD_T;

typedef enum {
    OTA_UPDATE_WAY_BEGIN,
    OTA_INTERACTION,
    OTA_SILENT,
    OTA_FORCE,
    OTA_UPDATE_WAY_END
} OTA_ENUM_UPDATE_WAY;

#define MAX_VERSION_LEN 64

typedef struct {
        OTA_STATUS_T status;
        OTA_ENUM_UPDATE_WAY update_way;
        void *mutex;
        char ota_version[MAX_VERSION_LEN];
        char firmware_version[MAX_VERSION_LEN];
} ota_info_t;


void ota_status_init(void);

void ota_set_status(OTA_STATUS_T status);

OTA_STATUS_T ota_get_status(void);

int8_t ota_status_post(int percent);

int8_t ota_result_post(void);

const char *ota_get_dev_version(void);

const char *ota_get_version();

void ota_set_version(const char *ota_version);

void ota_set_dev_version(const char *dev_version);

const char *ota_get_product_type(void);

const char *ota_get_system_version(void);

const char *ota_get_product_internal_type(void);
#ifdef __cplusplus
}
#endif

#endif
