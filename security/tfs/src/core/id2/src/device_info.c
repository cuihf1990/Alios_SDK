/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <string.h>
#include "device_info.h"
#include "pal.h"
#include "log.h"

#define TAG_DEVICE_INFO "DEVICE_INFO"

__attribute__((weak)) const char *pal_get_product_name()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_imei()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_hardware_id()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_mac()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_bt_mac()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_build_time()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_os_version()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_dm_pixels()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_dm_dpi()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_cpu_info()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_storage_total()
{
    return NULL;
}
__attribute__((weak)) const char *pal_get_camera_resolution()
{
    return NULL;
}

int collection(struct product_info *pInfo)
{
    LOGD(TAG_DEVICE_INFO, "collection");

    if (pInfo == NULL) {
        LOGE(TAG_DEVICE_INFO, "para pInfo is null");
        return -1;
    }

    memset(pInfo, 0, sizeof(struct product_info));

    if (pal_get_product_name) {
        pInfo->product_name = pal_get_product_name();
        LOGD(TAG_DEVICE_INFO, "product_name = %s\n", pInfo->product_name);
    }
    if (pal_get_imei) {
        pInfo->imei = pal_get_imei();
        LOGD(TAG_DEVICE_INFO, "imei = %s\n", pInfo->imei);
    }
    if (pal_get_hardware_id) {
        pInfo->hardware_id = pal_get_hardware_id();
        LOGD(TAG_DEVICE_INFO, "hardware_id = %s\n", pInfo->hardware_id);
    }
    if (pal_get_mac) {
        pInfo->mac = pal_get_mac();
        LOGD(TAG_DEVICE_INFO, "mac = %s\n", pInfo->mac);
    }
    if (pal_get_bt_mac) {
        pInfo->bt_mac = pal_get_bt_mac();
        LOGD(TAG_DEVICE_INFO, "bt_mac = %s\n", pInfo->bt_mac);
    }
    if (pal_get_build_time) {
        pInfo->build_time = pal_get_build_time();
        LOGD(TAG_DEVICE_INFO, "build_time = %s\n", pInfo->build_time);
    }
    if (pal_get_os_version) {
        pInfo->os_version = pal_get_os_version();
        LOGD(TAG_DEVICE_INFO, "os_version = %s\n", pInfo->os_version);
    }
    if (pal_get_dm_pixels) {
        pInfo->dm_pixels = pal_get_dm_pixels();
        LOGD(TAG_DEVICE_INFO, "dm_pixels = %s\n", pInfo->dm_pixels);
    }
    if (pal_get_dm_dpi) {
        pInfo->dm_dpi = pal_get_dm_dpi();
        LOGD(TAG_DEVICE_INFO, "dm_dpi = %s\n", pInfo->dm_dpi);
    }
    if (pal_get_cpu_info) {
        pInfo->cup_info = pal_get_cpu_info();
        LOGD(TAG_DEVICE_INFO, "cup_info = %s\n", pInfo->cup_info);
    }
    if (pal_get_storage_total) {
        pInfo->storage_total = pal_get_storage_total();
        LOGD(TAG_DEVICE_INFO, "storage_total = %s\n", pInfo->storage_total);
    }
    if (pal_get_camera_resolution) {
        pInfo->camera_resolution = pal_get_camera_resolution();
        LOGD(TAG_DEVICE_INFO, "camera_resolution = %s\n", pInfo->camera_resolution);
    }

    return 0;
}
