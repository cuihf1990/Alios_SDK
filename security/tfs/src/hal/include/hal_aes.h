/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#ifndef HAL_AES_H
#define HAL_AES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


int32_t hal_aes128_cbc_enc(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           uint8_t *output);

int32_t hal_aes128_cbc_dec(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           uint8_t *output);

#ifdef __cplusplus
}
#endif

#endif
