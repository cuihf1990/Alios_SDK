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
#include <yos_version.h>

#include "ota_transport.h"
#include "ota_platform_os.h"
#include "ota_util.h"
#include "ota_log.h"

static ota_info_t g_ota_info_storage = {
        .update_way = OTA_SILENT,
};
static ota_info_t *g_ota_info = &g_ota_info_storage;

void ota_status_init()
{
    g_ota_info->status = OTA_INIT;
    g_ota_info->mutex = ota_mutex_init();
}

OTA_STATUS_T ota_get_status(void)
{ 
    OTA_STATUS_T status;
    ota_mutex_lock(g_ota_info_storage.mutex);
    status = g_ota_info->status;
    ota_mutex_unlock(g_ota_info_storage.mutex);
    return status;
}

void ota_set_status(OTA_STATUS_T status)
{
    ota_mutex_lock(g_ota_info_storage.mutex);
    g_ota_info->status = status; 
    ota_mutex_unlock(g_ota_info_storage.mutex);
}

//extern int8_t platform_ota_status_post(int status, int percent);

int8_t ota_status_post(int percent)
{
    return platform_ota_status_post(g_ota_info->status, percent);
}

int8_t ota_result_post()
{
    return platform_ota_result_post();
}

const char *ota_get_version()
{
    ota_mutex_lock(g_ota_info_storage.mutex);
    if(strlen(g_ota_info->ota_version) > 0)
        return g_ota_info->ota_version;
    strncpy(g_ota_info->ota_version, 
        (char *)platform_ota_get_version(), sizeof g_ota_info->ota_version);
    ota_mutex_unlock(g_ota_info_storage.mutex);
    return g_ota_info->ota_version;
}

void ota_set_version(const char *ota_version)
{
    if(!ota_version)
    {
        return;
    }

    ota_mutex_lock(g_ota_info_storage.mutex);
    strncpy(g_ota_info->ota_version, ota_version, sizeof g_ota_info->ota_version);
    platform_ota_set_version((char *)ota_version);
    ota_mutex_unlock(g_ota_info_storage.mutex);
}

const char *ota_get_product_type(void)
{
    return NULL;
}


void ota_set_dev_version(const char *dev_version)
{
    platform_set_dev_version(dev_version);
}

const char *ota_get_dev_version(void)
{
    return (const char *)platform_get_dev_version();
}

const char *ota_get_system_version(void)
{
    return (const char *)platform_get_main_version();
}

const char *ota_get_product_internal_type(void)
{
    return NULL;
}

