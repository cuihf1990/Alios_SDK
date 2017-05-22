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

#ifndef _OTA_CONSTANTS_H_
#define _OTA_CONSTANTS_H_

#ifdef __cplusplus
extern "C"{
#endif

typedef enum {
        E_OTA_UPDATE_WAY_BEGIN,
        E_OTA_INTERACTION,
        E_OTA_SILENT,
        E_OTA_FORCE,
        E_OTA_UPDATE_WAY_END
} OTA_ENUM_UPDATE_WAY;


enum FILE_TYPE { MANIFEST = 0, BOOTLOADER, IMAGE, MODULE_APP ,UNDEFINED };

/* update manifest request params */
#define MF_REQ_URL                   "http://10.101.111.160"
#define MF_REQ_REAL_URL              "osupdateservice.yunos.com"
#define MF_REQ_FALSE_URL             "www.baidu.com"
#define MF_REQ_PATH                  "/update/manifest?"
#define MF_REQ_IP                    "10.101.111.160"
#define MF_REQ_DEFAULT_PORT          80

/* update manifest response sections */
#define MF_RESP_FOTA_INFO            "FOTA_Info"
#define MF_RESP_FOTA_IMAGE           "FOTA_Image"
#define MF_RESP_DOWNLOADURL          "DownloadUrl"
#define MF_RESP_OSSDOWNLOADURL       "OssDownloadUrl"
#define MF_RESP_TARGETOSVERSION      "SystemVersion"
#define MF_RESP_ERROR                "Error"
#define MF_RESP_NOERROR_VALUE        "NoError"
#define MF_RESP_NOUPDATE_VALUE		 "NoUpdateAvailable"

/* parse update packet */
#define HEADER_LENGTH_BYTE           13  // (8+4+1)
#define FILE_TYPE_BYTE               1
#define FILE_LENGTH_BYTE             4
#define WGET_BUF_SIZE                512
#define PARSE_ERROR                  -1
#define LEFTOVER_SIZE                4

/* action name for listening cmns push msg */
#define CMNS_PUSH_MSG_NAME           "com.aliyun.cloudapp.FOTA_UPDATE"

/* error handling */
#define MAX_RETRY_NUM                5

/* manifest response secions */
#define MANIFEST_RESP_DOWNLOAD_URL          (0)
#define MANIFEST_RESP_OSS_DOWNLOAD_URL      (1)
#define MANIFEST_RESP_SYSTEM_VERSION        (2)
#define MANIFEST_RESP_COUNT                 (3)

/* event_track head setions */
#define EVENT_TRACK_CURRENT_OS_VERSION      (0)
#define EVENT_TRACK_TARGET_OS_VERSION       (1)
#define EVENT_TRACK_QUERY_ID                (2)
#define EVENT_TRACK_UUID                    (3)
#define EVENT_TRACK_COUNT                   (4)

/* report event */
#define EVENT_DEVICE_TYPE            "YoS"

#ifdef OTA_CHANNEL_CMNS
#define EVENT_NAME_DL_SUCCESS        "os_download_succ"
#define EVENT_NAME_DL_FAILED         "os_download_failed"
#define EVENT_NAME_UD_READY          "update_ready"
#define EVENT_NAME_UD_SUCCESS        "os_recovery_install_status"
#define EVENT_NAME_UD_FAILED         "os_recovery_install_status"
#define EVENT_NAME_UPDATE_RESULT     "os_recovery_install_status"
#endif

#define EVENT_TYPE_SUC               "1000"
#define EVENT_TYPE_FAILED            "2000"

#define DL_SUCCESS                   (0)
#define DL_FAILED                    (1)
#define UPDATE_SUCCESS               (0)
#define UPDATE_FAILED                (1)

#define P_APPKEY                     "fotasebsMkxSqdcW"
#define APPKEY                       "23229061"
#define SECRET                       "$#MH6NVN*#"
#define PACKAGE_NAME                 "fota.yunos.com"
// #define PUSH_EVENT_NAME              "com.aliyun.cloudapp.FOTA_UPDATE"

#if defined(OTA_MODE_RECOVERY)
#ifdef CONFIG_FOTA_MEMORY_SIZE
#define BUCKET_SIZE                   CONFIG_FOTA_MEMORY_SIZE
#else
#define BUCKET_SIZE                   (16384)
#endif
#define ONCE_CONSUME                  (4096)
#define ONCE_PRODUCE                  (8192)
#else
#ifdef CONFIG_FOTA_MEMORY_SIZE
#define BUCKET_SIZE                   CONFIG_FOTA_MEMORY_SIZE
#else
#define BUCKET_SIZE                   (4096)
#endif
#define ONCE_CONSUME                  (4096)
#define ONCE_PRODUCE                  (1024)
#endif

#define OTA_HEDAER_MAX_LEN 512
#define OTA_READ_BUF_LEN 2048
#define OTA_READ_TEMP_LEN 512

#ifdef __cplusplus
}
#endif

#endif
