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

#include <stdbool.h>

#include "ipv6/ip6.h"
#include "utilities/encoding.h"

bool is_valid_digit(uint8_t ch)
{
    if (ch >= '0' && ch <= '9') {
        return true;
    }

    return false;
}

bool is_valid_upper(uint8_t ch)
{
    if (ch >= 'A' && ch <= 'Z') {
        return true;
    }

    return false;
}

bool is_valid_lower(uint8_t ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return true;
    }

    return false;
}

ur_error_t string_to_ip6_addr(const char *buf, ur_ip6_addr_t *target)
{

    uint32_t addr_index;
    uint32_t zero_blocks;
    uint32_t current_block_index;
    uint32_t current_block_value;
    const char *s;

    zero_blocks = 8;
    for (s = buf; *s != 0; s++) {
        if (*s == ':') {
            zero_blocks--;
        } else if (!(is_valid_digit(*s) || is_valid_lower(*s) || is_valid_upper(*s))) {
            break;
        }
    }

    addr_index = 0;
    current_block_index = 0;
    current_block_value = 0;
    for (s = buf; *s != 0; s++) {
        if (*s == ':') {
            if (target) {
                if (current_block_index & 0x1) {
                    target->m32[addr_index++] |= current_block_value;
                } else {
                    target->m32[addr_index] = current_block_value << 16;
                }
            }
            current_block_index++;
            current_block_value = 0;
            if (current_block_index > 7) {
                return UR_ERROR_PARSE;
            }
            if (s[1] == ':') {
                if (s[2] == ':') {
                    return 0;
                }
                s++;
                while (zero_blocks > 0) {
                    zero_blocks--;
                    if (current_block_index & 0x1) {
                        addr_index++;
                    } else {
                        if (target) {
                            target->m32[addr_index] = 0;
                        }
                    }
                    current_block_index++;
                    if (current_block_index > 7) {
                        return UR_ERROR_PARSE;
                    }
                }
            }
        } else if (is_valid_digit(*s) || is_valid_lower(*s) || is_valid_upper(*s)) {
            current_block_value = (current_block_value << 4) +
            (is_valid_digit(*s) ? *s - '0' :
            10 + (is_valid_lower(*s) ? *s - 'a' : *s - 'A'));
        } else {
            break;
        }
    }

    if (target) {
        if (current_block_index & 0x1) {
            target->m32[addr_index++] |= current_block_value;
        } else {
            target->m32[addr_index] = current_block_value << 16;
        }
    }

    if (target) {
        for (addr_index = 0; addr_index < 4; addr_index++) {
            target->m32[addr_index] = ur_swap32(target->m32[addr_index]);
        }
    }

    if (current_block_index != 7) {
        return UR_ERROR_PARSE;
    }

    return UR_ERROR_NONE;
}

bool ur_is_mcast(const ur_ip6_addr_t *addr)
{
    return addr->m8[0] == 0xff;
}

bool ur_is_unique_local(const ur_ip6_addr_t *addr)
{
    return (addr->m8[0] & 0xfe) == 0xfc;
}
