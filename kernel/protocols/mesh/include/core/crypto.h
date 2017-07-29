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

#ifndef UMESH_CRYPTO_H
#define UMESH_CRYPTO_H

ur_error_t umesh_aes128_cbc_encrypt(const void *src,
                                    uint16_t size, void *dst);
ur_error_t umesh_aes128_cbc_decrypt(const void *src,
                                    uint16_t size, void *dst);

#endif  /* UMESH_CRYPTO_H */
