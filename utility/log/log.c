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

#include <stdarg.h>
#include <stdio.h>
#include <yos/log.h>

unsigned int yos_log_level = YOS_LL_V_DEBUG | YOS_LL_V_INFO | YOS_LL_V_WARN | YOS_LL_V_ERROR | YOS_LL_V_FATAL;

__attribute__((weak)) int csp_printf(const char *fmt, ...)
{
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vprintf(fmt, args);
    va_end(args);

    fflush(stdout);

    return ret;
}

void yos_set_log_level(yos_log_level_t log_level)
{
    unsigned int value = 0;

    switch (log_level) {
        case YOS_LL_NONE:
            value |= YOS_LL_V_NONE;
            break;
        case YOS_LL_DEBUG:
            value |= YOS_LL_V_DEBUG;
        case YOS_LL_INFO:
            value |= YOS_LL_V_INFO;
        case YOS_LL_WARN:
            value |= YOS_LL_V_WARN;
        case YOS_LL_ERROR:
            value |= YOS_LL_V_ERROR;
        case YOS_LL_FATAL:
            value |= YOS_LL_V_FATAL;
            break;
        default:
            break;
    }

    yos_log_level = value;
}

