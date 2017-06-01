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


