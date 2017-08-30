/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _HAL_3DES_H
#define _HAL_3DES_H

#include <stdint.h>

#define KEY_ID_0 0 // ID2

// MODE TYPE
#define TFS_3DES_ECB 0X01 // default
#define TFS_3DES_CBC 0X02

#ifdef __cplusplus
extern "C" {
#endif

int hal_3DES_sign(uint8_t ID, const uint8_t *in, uint32_t in_len,
                  uint8_t *sign, uint32_t *sign_len, uint8_t mode);
int hal_3DES_verify(uint8_t ID, const uint8_t *in, uint32_t in_len,
                    uint8_t *sign, uint32_t sign_len, uint8_t mode);
int hal_3DES_encrypt(uint8_t ID, const uint8_t *in, uint32_t in_len,
                     uint8_t *out, uint32_t *out_len, uint8_t mode);
int hal_3DES_decrypt(uint8_t ID, uint8_t *in, uint32_t in_len,
                     uint8_t *out, uint32_t *out_len, uint8_t mode);

#ifdef __cplusplus
}
#endif

#endif
