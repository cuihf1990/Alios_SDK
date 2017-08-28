/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>
#include <yos/log.h>
#include<stdio.h>

#ifndef YOS_EXPORT
#define YOS_EXPORT
#endif

#ifndef YOS_WEAK
#define YOS_WEAK __attribute__((weak))
#endif

#define TAG "YOS_VERSION"

YOS_EXPORT YOS_WEAK const char   *get_yos_product_model(void)
{
    return (const char *)SYSINFO_PRODUCT_MODEL;
}

YOS_EXPORT YOS_WEAK const char   *get_yos_device_name(void)
{
    return (const char *)SYSINFO_DEVICE_NAME;
}

YOS_EXPORT YOS_WEAK const char   *get_yos_os_version(void)
{
    return (const char *)SYSINFO_OS_VERSION;
}

#ifdef SYSINFO_OS_BINS
YOS_EXPORT YOS_WEAK const char   *get_yos_kernel_version(void)
{
    return (const char *)SYSINFO_KERNEL_VERSION;
}


YOS_EXPORT YOS_WEAK const char   *get_yos_app_version(void)
{
    return (const char *)SYSINFO_APP_VERSION;
}
#endif

YOS_EXPORT YOS_WEAK void dump_sys_info(void)
{
    LOGI(TAG, "os_version: %s", get_yos_os_version());
    #ifdef SYSINFO_OS_BINS
    LOGI(TAG, "kernel_version: %s", get_yos_kernel_version());
    LOGI(TAG, "app_version: %s", get_yos_app_version());
    #endif
    LOGI(TAG, "product_model: %s", get_yos_product_model());
    LOGI(TAG, "device_name: %s", get_yos_device_name());
}

