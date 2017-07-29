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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "yos/log.h"

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

#ifdef CONFIG_YOS_DDA
    dda_log(buffer);
#endif
    if (!g_cli_silent) {
        csp_printf("%s", buffer);
    }

    ur_mem_free(buffer, MAX_LOG_SIZE);
}
#endif

ur_error_t ur_cli_output(const char *buf, uint16_t length)
{
#ifdef CONFIG_YOS_DDA
    dda_cli_log((char *)buf);
#endif
    if (!g_cli_silent) {
        csp_printf("%s", buf);
    }

    return UR_ERROR_NONE;
}

