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
/*
 * YOS EXPORT API:
 *      get_yos_product_name
 *      get_yos_chip_version
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


