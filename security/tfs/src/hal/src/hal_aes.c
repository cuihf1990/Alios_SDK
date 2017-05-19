/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <string.h>
#include "pal.h"
#include "log.h"
#include "hal_aes.h"

#ifdef TFS_TEE
#include "tee_aes.h"
#endif

#define TAG_HAL_RSA "TFS_HAL_AES"

int32_t hal_aes128_cbc_enc(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           uint8_t *output)
{
    return tee_aes_cbc_encrypt(key, iv, input_len, input, output);
}

int32_t hal_aes128_cbc_dec(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           uint8_t *output)
{
    return tee_aes_cbc_decrypt(key, iv, input_len, input, output);
}

