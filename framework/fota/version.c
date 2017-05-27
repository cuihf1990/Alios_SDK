/*
* Copyright (C) 2016 YunOS Project. All rights reserved.
*/
/*
 * YOS EXPORT API:
 *      get_yos_product_name
 *      get_yos_chip_version
*/
#include <string.h>

#ifndef YOS_EXPORT
#define YOS_EXPORT
#endif

#ifndef YOS_WEAK
#define YOS_WEAK __attribute__((weak))
#endif

#define SYSINFO_PRODUCT_MODEL ""
#define SYSINFO_PRODUCT_INTERNAL_TYPE ""
#define SYSINFO_OS_VERSION ""
#define SYSINFO_OS_INNERVERSION ""
 

YOS_EXPORT YOS_WEAK const char   *get_yos_product_model(void)
{
    char *ret = SYSINFO_PRODUCT_MODEL;
    return (const char *)ret;
}


YOS_EXPORT YOS_WEAK const char   *get_yos_product_internal_type(void)
{
    char *ret = SYSINFO_PRODUCT_INTERNAL_TYPE;
    return (const char *)ret;
}

YOS_EXPORT YOS_WEAK const char   *get_yos_os_version(void)
{
    char *ret = "v2.0.0.0";
    return (const char *)ret;
}


