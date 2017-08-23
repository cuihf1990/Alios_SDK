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

#ifndef YOS_KV_H
#define YOS_KV_H

#include <yos/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief add another KV pair .
 *
 * @param[in] @key  the key of the KV pair.
 * @param[in] @value  the value of the KV pair.
 * @param[in] @len  the length of the @value.
 * @param[in] @sync  1: save the KV pair to flash right now,
 *                   0: do not save the pair this time.
 *
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_set(const char *key, const void *value, int len, int sync);

/**
 * @brief get the KV value stored in @buffer by its key @key.
 *
 * @param[in] @key, the key of the KV you want to get.
 * @param[out] @buffer, the memory to store KV value.
 * @param[out] @buff_len, the real lenght of value.
 *
 * @note: the @buffer_len should be large enough to store the value,
 *            otherwise @buffer would be NULL.
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_get(const char *key, void *buffer, int *buffer_len);

/**
 * @brief delete the KV pair by its key @key.
 *
 * @param[in] key , the key of the KV pair you want to delete.
 *
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_del(const char *key);

#ifdef __cplusplus
}
#endif

#endif /* YOS_KV_H */

