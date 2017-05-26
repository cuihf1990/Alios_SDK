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

#ifndef YOS_LOG_IMPL_H
#define YOS_LOG_IMPL_H

#if defined(__cplusplus)
extern "C"
{
#endif

extern unsigned int yos_log_level;
static inline unsigned int yos_log_get_level(void)
{
    return yos_log_level;
}

enum log_level_bit {
    YOS_LL_V_NONE_BIT = -1,
    YOS_LL_V_FATAL_BIT,
    YOS_LL_V_ERROR_BIT,
    YOS_LL_V_WARN_BIT,
    YOS_LL_V_INFO_BIT,
    YOS_LL_V_DEBUG_BIT,
    YOS_LL_V_MAX_BIT
};

#define YOS_LOG_LEVEL yos_log_get_level()

#define YOS_LL_V_NONE  0
#define YOS_LL_V_ALL  0xFF
#define YOS_LL_V_FATAL (1 << YOS_LL_V_FATAL_BIT)
#define YOS_LL_V_ERROR (1 << YOS_LL_V_ERROR_BIT)
#define YOS_LL_V_WARN  (1 << YOS_LL_V_WARN_BIT)
#define YOS_LL_V_INFO  (1 << YOS_LL_V_INFO_BIT)
#define YOS_LL_V_DEBUG (1 << YOS_LL_V_DEBUG_BIT)

/*
 * color def.
 * see http://stackoverflow.com/questions/3585846/color-text-in-terminal-applications-in-unix
 */
#define COL_DEF "\x1B[0m"   //white
#define COL_RED "\x1B[31m"  //red
#define COL_GRE "\x1B[32m"  //green
#define COL_BLU "\x1B[34m"  //blue
#define COL_YEL "\x1B[33m"  //yellow
#define COL_WHE "\x1B[37m"  //white
#define COL_CYN "\x1B[36m"
#define COL_MAG "\x1B[35m"

#define CONFIG_LOGMACRO_DETAILS
#ifdef CONFIG_LOGMACRO_DETAILS
#define log_print(CON, MOD, COLOR, LVL, FMT, ...) \
    do { \
        if (CON) { \
            csp_printf(COLOR"<%s> %s [%s#%d] : ", LVL, MOD, __FUNCTION__, __LINE__); \
            csp_printf(FMT COL_DEF"\r\n", ##__VA_ARGS__); \
        } \
    } while (0)

#else
#define log_print(CON, MOD, COLOR, LVL, FMT, ...) \
    do { \
        if (CON) { \
            csp_printf("<%s> "FMT"\n", LVL, ##__VA_ARGS__); \
        } \
    } while (0)

#endif

#define void_func(fmt, ...)

#ifndef os_printf
#ifndef csp_printf
int csp_printf(const char *fmt, ...);
#endif
#endif

#undef LOGF
#undef LOGE
#undef LOGW
#undef LOGI
#undef LOGD
#undef LOG

#define LOG_IMPL(fmt, ...) \
            log_print(1, "YoC", COL_DEF, "V", fmt, ##__VA_ARGS__)

#ifdef CONFIG_LOGMACRO_SILENT
#define LOGF_IMPL(mod, fmt, ...) void_func(fmt, ##__VA_ARGS__)
#define LOGE_IMPL(mod, fmt, ...) void_func(fmt, ##__VA_ARGS__)
#define LOGW_IMPL(mod, fmt, ...) void_func(fmt, ##__VA_ARGS__)
#define LOGI_IMPL(mod, fmt, ...) void_func(fmt, ##__VA_ARGS__)
#define LOGD_IMPL(mod, fmt, ...) void_func(fmt, ##__VA_ARGS__)

#else
#define LOGF_IMPL(mod, fmt, ...) \
            log_print(YOS_LOG_LEVEL & YOS_LL_V_FATAL, mod, COL_RED, "F", fmt, ##__VA_ARGS__)
#define LOGE_IMPL(mod, fmt, ...) \
            log_print(YOS_LOG_LEVEL & YOS_LL_V_ERROR, mod, COL_YEL, "E", fmt, ##__VA_ARGS__)
#define LOGW_IMPL(mod, fmt, ...) \
            log_print(YOS_LOG_LEVEL & YOS_LL_V_WARN, mod, COL_BLU, "W", fmt, ##__VA_ARGS__)
#define LOGI_IMPL(mod, fmt, ...) \
            log_print(YOS_LOG_LEVEL & YOS_LL_V_INFO, mod, COL_GRE, "I", fmt, ##__VA_ARGS__)
#define LOGD_IMPL(mod, fmt, ...) \
            log_print(YOS_LOG_LEVEL & YOS_LL_V_DEBUG, mod, COL_WHE, "D", fmt, ##__VA_ARGS__)

#endif /* CONFIG_LOGMACRO_SILENT */

#if defined(__cplusplus)
}
#endif

#endif /* YOS_LOG_IMPL_H */

