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

int pal_collect_device_info(struct device_info *pInfo) {
    LOGD(TAG_PAL_DEVICE, "%s: enter.\n", __FUNCTION__);

    if (pInfo == NULL) {
        LOGE(TAG_PAL_DEVICE, "%s: para pInfo is null.\n", __FUNCTION__);
        return -1;
    }

    memset(pInfo, 0, sizeof(struct device_info));

    pInfo->product_name = PRODUCT_NAME;
    LOGD(TAG_PAL_DEVICE, "%s: product_name = %s\n", __FUNCTION__, pInfo->product_name);

    return 0;
}
