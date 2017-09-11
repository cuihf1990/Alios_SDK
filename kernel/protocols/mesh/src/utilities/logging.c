/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "aos/aos.h"

#include "umesh_utils.h"
#include "tools/cli.h"

extern int dda_log(char *str);
extern int dda_cli_log(char *str);

#ifdef DEBUG
#define MAX_LOG_SIZE 256

static ur_log_level_t g_log_level = UR_LOG_LEVEL_INFO;
ur_log_level_t ur_log_get_level(void)
{
    return g_log_level;
}

void ur_log_set_level(ur_log_level_t lvl)
{
    g_log_level = lvl;
}

void __attribute__((weak)) ur_log(ur_log_level_t level, ur_log_region_t region,
                                  const char *format, ...)
{
    va_list args;
    char *buffer = (char *)ur_mem_alloc(MAX_LOG_SIZE);

    if (buffer == NULL) {
        return;
    }

    if (g_log_level < level) {
        ur_mem_free(buffer, MAX_LOG_SIZE);
        return;
    }

    va_start(args, format);
    vsnprintf(buffer, MAX_LOG_SIZE, format, args);
    va_end(args);

#ifdef CONFIG_AOS_DDA
    dda_log(buffer);
#endif
    if (!g_cli_silent) {
        long long ms = aos_now_ms();
        csp_printf("[%4d.%03d] %s", (int)(ms/1000), (int)(ms%1000), buffer);
    }

    ur_mem_free(buffer, MAX_LOG_SIZE);
}
#endif
