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

/**
 * @file hal/crypto.h
 * @brief CRYPTO HAL API for YOC
 * @version since 5.5.0
 */

#ifndef HAL_CRYPTO_H
#define HAL_CRYPTO_H

#define AES_PRIV_DATA_MAXSIZE (512)

typedef struct {
    int   priv_data_size;
    int   hexdump_result;

    int (*aes_init)(void * ctx);
    int (*aes_destroy)(void * ctx);
    int (*aes_cbc_encrypt)(
              void *ctx,
              const unsigned char *src, const unsigned int src_len,
              const unsigned char *key, const int key_size,
              const unsigned char *ivec,
              unsigned char *dst, unsigned int *dst_len);
    int (*aes_cbc_decrypt)(
              void *ctx,
              const unsigned char *src, const unsigned int src_len,
              const unsigned char *key, const int key_size,
              const unsigned char *ivec,
              unsigned char *dst, unsigned int *dst_len);

} hal_aes_module_t;

/* for application */
int hal_aes_unittest(void);
int hal_aes_init(void **priv_data);
int hal_aes_cbc_encrypt(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_len,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len);
int hal_aes_cbc_decrypt(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_len,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len);
int hal_aes_destroy(void *priv_data);

/* for platform */
int hal_aes_register(hal_aes_module_t *mod);

#endif /* HAL_CRYPTO_H */

