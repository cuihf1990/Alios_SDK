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

#ifndef _MD5_H_
#define _MD5_H_
#include <stdint.h>
#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif
typedef struct {
    uint32_t state[4];                  /* state (ABCD) */
    uint32_t count[2];                  /* number of bits, modulo 2^64 (lsb first) */
    uint8_t buffer[64];                 /* input buffer */
} MD5_CTX;

void MD5_Init(MD5_CTX *ctx);
void MD5_Update(MD5_CTX *ctx, const uint8_t *msg, int len);
void MD5_Final(uint8_t *digest, MD5_CTX *ctx);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif
