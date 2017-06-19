/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <string.h>
#include "pal.h"
#include "log.h"

#define TAG_PAL_DEVICE "PAL_DEVICE"

#ifndef TFS_ONLINE
#define PRODUCT_NAME "dhtestproduct4"
#else
#define PRODUCT_NAME "dic_id2_test"
#endif

int pal_collect_device_info(struct device_info *pInfo)
{
    LOGD(TAG_PAL_DEVICE, "[%s]: enter.\n", __func__);

    if (pInfo == NULL) {
        LOGE(TAG_PAL_DEVICE, "[%s]: para pInfo is null.\n", __func__);
        return -1;
    }

    memset(pInfo, 0, sizeof(struct device_info));

    pInfo->product_name = PRODUCT_NAME;
    LOGD(TAG_PAL_DEVICE, "[%s]: product_name = %s\n", __func__,
         pInfo->product_name);

    pInfo->imei = "123456789012345";
    LOGD(TAG_PAL_DEVICE, "[%s]: imei = %s\n", __func__, pInfo->imei);

    pInfo->hardware_id = "123456789012345678";
    LOGD(TAG_PAL_DEVICE, "[%s]: hardware_id = %s\n", __func__, pInfo->hardware_id);

    pInfo->mac = "4c:b1:6c:90:46:f2";
    LOGD(TAG_PAL_DEVICE, "[%s]: mac = %s\n", __func__, pInfo->mac);

    pInfo->bt_mac = "4c:b1:6c:90:46:f3";
    LOGD(TAG_PAL_DEVICE, "[%s]: bt_mac = %s\n", __func__, pInfo->bt_mac);

    pInfo->build_time = "20170527";
    LOGD(TAG_PAL_DEVICE, "[%s]: build_time = %s\n", __func__, pInfo->build_time);

    pInfo->os_version = "UBuntu";
    LOGD(TAG_PAL_DEVICE, "[%s]: os_version = %s\n", __func__, pInfo->os_version);

    pInfo->dm_pixels = "4096*1280";
    LOGD(TAG_PAL_DEVICE, "[%s]: dm_pixels = %s\n", __func__, pInfo->dm_pixels);

    pInfo->dm_dpi = "1024";
    LOGD(TAG_PAL_DEVICE, "[%s]: dm_dpi = %s\n", __func__, pInfo->dm_dpi);

    pInfo->cup_info = "16";
    LOGD(TAG_PAL_DEVICE, "[%s]: cup_info = %s\n", __func__, pInfo->cup_info);

    pInfo->storage_total = "256G";
    LOGD(TAG_PAL_DEVICE, "[%s]: storage_total = %s\n", __func__,
         pInfo->storage_total);

    pInfo->camera_resolution = "4096*4096";
    LOGD(TAG_PAL_DEVICE, "[%s]: camera_resolution = %s\n", __func__,
         pInfo->camera_resolution);

    return 0;
}
