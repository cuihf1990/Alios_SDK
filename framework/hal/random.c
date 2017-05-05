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

#include <stdlib.h>
#include <string.h>
#include <hal/hal.h>

int __attribute__((weak)) hal_arch_random_seed(uint32_t seed)
{
    srand(seed);

    return 0;
}

int __attribute__((weak)) hal_arch_random_generate(uint8_t *buf, size_t len)
{
    uint32_t i;
    uint32_t tmp;

    for (i = 0; i < len; i+=sizeof(uint32_t)) {
        tmp = rand();
        memcpy((void *)(buf+i), (void *)&tmp, sizeof(uint32_t) > (len - i) ? len - i : sizeof(uint32_t));
    }

    return 0;
}

int hal_random_seed(uint32_t seed)
{
    return hal_arch_random_seed(seed);
}

int hal_random_generate_buf(uint8_t*buf, size_t len)
{
    return hal_arch_random_generate(buf, len);
}

int hal_random_generate(void)
{
    uint32_t data;

    hal_arch_random_generate((uint8_t *)&data, sizeof(uint32_t));
    return data;
}


