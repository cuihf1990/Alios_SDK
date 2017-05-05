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


#ifndef UR_MEMORY_H
#define UR_MEMORY_H

typedef struct ur_mem_stats_s {
    uint32_t num;
} ur_mem_stats_t;

void *ur_mem_alloc(uint16_t size);
void ur_mem_free(void *mem, uint16_t size);
const ur_mem_stats_t *ur_mem_get_stats(void);

#endif  /* UR_MEMORY_H */
