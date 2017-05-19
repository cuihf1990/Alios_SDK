/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#ifndef _TFS_DEVICE_INFO_H
#define _TFS_DEVICE_INFO_H

struct product_info
{
    const char *product_name;
    /* type need to modify  */
    const char *imei;
    const char *hardware_id;
    const char *mac;
    const char *bt_mac;
    const char *build_time;
    const char *os_version;
    const char *dm_pixels;
    const char *dm_dpi;
    const char *cup_info;
    const char *storage_total;
    const char *camera_resolution;
};

/*
 * @return: -1~ERROR, 0~OK
 */
int collection(struct product_info *pInfo);

#endif
