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

#ifndef UR_CONFIGS_H
#define UR_CONFIGS_H

#include "umesh_types.h"

typedef struct prev_netinfo_s {
    uint16_t meshnetid;
    uint16_t path_cost;
} prev_netinfo_t;

typedef struct ur_configs_s {
    uint32_t magic_number;
    uint8_t  version;
    uint8_t  main_version;
    prev_netinfo_t prev_netinfo;
} ur_configs_t;

ur_error_t ur_configs_read(ur_configs_t *config);
ur_error_t ur_configs_write(ur_configs_t *config);

#endif  /* UR_CONFIGS_H */
