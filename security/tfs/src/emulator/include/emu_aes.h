/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#ifndef _EMU_AES128_H
#define _EMU_AES128_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t emu_aes128_cbc_enc(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           uint8_t *output);

int32_t emu_aes128_cbc_dec(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           uint8_t *output);

#ifdef __cplusplus
}
#endif

#endif
