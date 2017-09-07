/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>
#include <stdio.h>
#include <aos/aos.h>

#include <aos/log.h>
#include <aos/kernel.h>

#ifndef YOS_EXPORT
#define YOS_EXPORT
#endif

#ifndef YOS_WEAK
#define YOS_WEAK __attribute__((weak))
#endif

#define TAG "YOS_VERSION"

YOS_EXPORT YOS_WEAK const char   *get_aos_product_model(void)
{
    return (const char *)SYSINFO_PRODUCT_MODEL;
}

YOS_EXPORT YOS_WEAK const char   *get_aos_device_name(void)
{
    return (const char *)SYSINFO_DEVICE_NAME;
}

#define OS_MAX_VERSION_LEN 64    
char  os_version[OS_MAX_VERSION_LEN];

YOS_EXPORT YOS_WEAK const char   *get_aos_kernel_version(void)
{
    return (const char *)yos_version_get();
}

YOS_EXPORT YOS_WEAK const char   *get_aos_app_version(void)
{
    return (const char *)SYSINFO_APP_VERSION;
}

YOS_EXPORT YOS_WEAK const char   *get_aos_os_version(void)
{
    #ifdef SYSINFO_OS_BINS
    snprintf(os_version, OS_MAX_VERSION_LEN, "%s_%s", get_aos_kernel_version(), get_aos_app_version());
    return os_version;
    #else  
    return (const char *)SYSINFO_OS_VERSION;
    #endif
}

YOS_EXPORT YOS_WEAK void dump_sys_info(void)
{
    LOGI(TAG, "app_version: %s", get_aos_app_version());
#ifdef SYSINFO_OS_BINS
    LOGI(TAG, "kernel_version: %s", get_aos_kernel_version());
#endif
    LOGI(TAG, "product_model: %s", get_aos_product_model());
    LOGI(TAG, "device_name: %s", get_aos_device_name());
}

