/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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


#ifndef __MSDP_COMMON_H__
#define __MSDP_COMMON_H__

#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include "msdp.h"

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

int msdp_set_attr_each_cb(const char *attr_name, int name_len, int type,
                          va_list *ap);

int msdp_get_attr_each_cb(const char *attr_name, int name_len, int type,
                          va_list *ap);

int msdp_set_status_array(char *params);

int msdp_set_device_status_handler(char *params, void *cb, void *exec_cb);

int msdp_get_device_status_handler(char *params, void *cb, void *exec_cb,
                                   char **json_out);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif
