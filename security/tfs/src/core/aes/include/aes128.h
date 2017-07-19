/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#ifndef __AES128_H
#define __AES128_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int aes128_cbc_enc(const uint8_t *key,
                   const uint8_t *iv,
                   int32_t input_len,
                   const uint8_t *input,
                   int32_t *output_len,
                   uint8_t *output,
                   uint8_t padding);

int aes128_cbc_dec(const uint8_t *key,
                   const uint8_t *iv,
                   int32_t input_len,
                   const uint8_t *input,
                   int32_t *output_len,
                   uint8_t *output,
                   uint8_t padding);

#ifdef __cplusplus
}
#endif

#endif // __AES128_H
