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

#ifndef YUNOS_MM_DEBUG_H
#define YUNOS_MM_DEBUG_H

#if (YUNOS_CONFIG_MM_DEBUG > 0)

#ifdef __cplusplus
extern "C" {
#endif

#define YOS_MM_SCAN_REGION_MAX 10
typedef struct {
    void *start;
    void *end;
} mm_scan_region_t;

#if (YUNOS_CONFIG_GCC_RETADDR > 0u)
#include <k_mm.h>
#define YOS_UNSIGNED_INT_MSB (1 << (sizeof(unsigned int) * 8 - 1))
void yunos_owner_attach(k_mm_head *mmhead, void *addr, size_t allocator);
#endif

uint32_t yunos_mm_leak_region_init(void *start, void *end);

uint32_t dumpsys_mm_info_func(char *buf, uint32_t len);

uint32_t dump_mmleak();

#ifdef __cplusplus
}
#endif

#endif /* YUNOS_CONFIG_MM_DEBUG */

#endif /* YSH_H */


