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
#include <stdio.h>
#include <string.h>
#include <tfs.h>

#include "ota_util.h"
#include "ota_log.h"

#define PACKET_VER_SIZE 64

typedef struct {
	OTA_STATUS_T status;
	OTA_ENUM_UPDATE_WAY update_way;
	char packet_ver[PACKET_VER_SIZE];
    void* cb_para;
} ota_info_t;

static ota_info_t g_ota_info_storage = {
        .status = E_OTA_IDLE,
        .update_way = E_OTA_SILENT,
};
static ota_info_t *g_ota_info = &g_ota_info_storage;

OTA_STATUS_T ota_get_status(void)
{
    return g_ota_info->status;
}

void ota_set_status(OTA_STATUS_T status)
{
    g_ota_info->status = status;
}

char* ota_get_id2(void) {


    static char id2[TFS_ID2_LEN+1] = {0};
    if (!strlen(id2)) {
        uint32_t len = TFS_ID2_LEN;
        if ( tfs_get_ID2((unsigned char *)id2, &len) < 0 ) {
            OTA_LOG_E("get_ID2 failed!");
        }
    }

    OTA_LOG_D("id2: %s", id2);
    return id2;
}



