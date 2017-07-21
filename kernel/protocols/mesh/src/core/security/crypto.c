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

#include <ali_crypto.h>
#include <yos/kernel.h>

#include "stdio.h"

#include "umesh_types.h"
#include "core/crypto.h"
#include "core/keys_mgr.h"

#define KEY_SIZE 16

typedef void *umesh_aes_ctx_t;

uint8_t g_umesh_iv[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t g_umesh_key[KEY_SIZE] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x00,
};

static ur_error_t umesh_aes128_cbc_encrypt_decrypt(umesh_aes_ctx_t *aes,
                                                   const void *src,
                                                   uint16_t size,
                                                   void *dst)
{
    ali_crypto_result result;
    uint32_t dlen = 1024;

    if (aes == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_finish(src, size, dst, &dlen, SYM_NOPAD, aes);

    if (result != ALI_CRYPTO_SUCCESS) {
        return UR_ERROR_FAIL;
    }

    return UR_ERROR_NONE;
}

ur_error_t umesh_aes128_cbc_encrypt(const void *src,
                                    uint16_t size, void *dst)
{
    ur_error_t error;
    umesh_aes_ctx_t *aes;
    uint32_t aes_ctx_size;
    ali_crypto_result result;

    if (src == NULL || dst == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_get_ctx_size(AES_CTR, &aes_ctx_size);
    if (result != ALI_CRYPTO_SUCCESS) {
        return UR_ERROR_FAIL;
    }

    aes = yos_malloc(aes_ctx_size);
    if (aes == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_init(AES_CTR, true,
                 g_umesh_key, NULL, KEY_SIZE, g_umesh_iv, aes);
    if (result != ALI_CRYPTO_SUCCESS) {
        yos_free(aes);
        return UR_ERROR_FAIL;
    }

    error = umesh_aes128_cbc_encrypt_decrypt(aes, src, size, dst);
    yos_free(aes);

    return error;
}

ur_error_t umesh_aes128_cbc_decrypt(const void *src,
                                    uint16_t size, void *dst)
{
    ur_error_t error;
    umesh_aes_ctx_t *aes;
    uint32_t aes_ctx_size;
    ali_crypto_result result;

    if (src == NULL || dst == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_get_ctx_size(AES_CTR, &aes_ctx_size);
    if (result != ALI_CRYPTO_SUCCESS) {
        return UR_ERROR_FAIL;
    }

    aes = yos_malloc(aes_ctx_size);
    if (aes == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_init(AES_CTR, false,
                 g_umesh_key, NULL, KEY_SIZE, g_umesh_iv, aes);
    if (result != ALI_CRYPTO_SUCCESS) {
        yos_free(aes);
        return UR_ERROR_FAIL;
    }

    error = umesh_aes128_cbc_encrypt_decrypt(aes, src, size, dst);
    yos_free(aes);

    return error;
}
