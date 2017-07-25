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

#include <stdint.h>

#include "umesh_utils.h"

inline uint16_t ur_swap16(uint16_t value)
{
    return
        ((value & (uint16_t)(0x00ffU)) << 8) |
        ((value & (uint16_t)(0xff00U)) >> 8);
}

inline uint32_t ur_swap32(uint32_t value)
{
    return
        ((value & (uint32_t)(0x000000ffUL)) << 24) |
        ((value & (uint32_t)(0x0000ff00UL)) <<  8) |
        ((value & (uint32_t)(0x00ff0000UL)) >>  8) |
        ((value & (uint32_t)(0xff000000UL)) >> 24);
}
