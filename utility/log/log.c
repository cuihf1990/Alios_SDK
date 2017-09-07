/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdarg.h>
#include <stdio.h>
#include <yos/log.h>

unsigned int aos_log_level = YOS_LL_V_DEBUG | YOS_LL_V_INFO | YOS_LL_V_WARN | YOS_LL_V_ERROR | YOS_LL_V_FATAL;

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

void aos_set_log_level(aos_log_level_t log_level)
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

    aos_log_level = value;
}

