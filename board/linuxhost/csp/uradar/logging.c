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
#include <stdlib.h>

#include "logging.h"

extern int agent_log(char *str);
extern int dda_log(char *str);

enum {
    MAX_LOG_SIZE = 1500,
};

ur_log_level_t g_level = UR_LOG_LEVEL_INFO;

void ur_log(ur_log_level_t level, ur_log_region_t region,
            const char *format, ...)
{
    va_list args;
    char *buffer;

    if (g_level < level) {
        return;
    }

    buffer = (char *)malloc(MAX_LOG_SIZE);
    if (buffer == NULL) {
        return;
    }

    va_start(args, format);
    vsnprintf(buffer, MAX_LOG_SIZE, format, args);
#ifdef CONFIG_YOC_DDA
    dda_log(buffer);
#else
    agent_log(buffer);
#endif
    va_end(args);
    free(buffer);
}
