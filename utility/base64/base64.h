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

#ifndef _BASE64_H_
#define _BASE64_H_

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
/**
 * @brief base64 encode
 *
 * @param[in] input: input byte stream
 * @param[in] input_len: input stream length in byte
 * @param[out] output: base64 encoded string
 * @param[in/out] output_len: [in] for output buffer size, [out] for
 *                  base64 encoded string length
 * @Note output buffer is not NULL-terminated
 *
 * @retval  output buffer on success, otherwise NULL will return
 */
unsigned char *base64_encode(const unsigned char *input, int input_len,
                             unsigned char *output, int *output_len);

/**
 * @brief base64 decode
 *
 * @param[in] input: input byte stream
 * @param[in] input_len: input stream length in byte
 * @param[out] output: base64 decoded string
 * @param[in/out] output_len: [in] for output buffer size, [out] for
 *                  base64 decoded string length
 * @Note output buffer is not NULL-terminated
 * @retval  output buffer on success, otherwise NULL will return
 */
unsigned char *base64_decode(const unsigned char *input, int input_len,
                             unsigned char *output, int *output_len);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif

