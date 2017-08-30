/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef __ID2_H
#define __ID2_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


int get_ID2(uint8_t *id2, uint32_t *len);
int id2_sign(const uint8_t *in, uint32_t in_len,
             uint8_t *sign, uint32_t *sign_len);
int id2_verify(const uint8_t *in, uint32_t in_len,
               uint8_t *sign, uint32_t sign_len);
int id2_encrypt(uint8_t *in, uint32_t in_len,
                uint8_t *out, uint32_t *out_len);
int id2_decrypt(uint8_t *in, uint32_t in_len,
                uint8_t *out, uint32_t *out_len);

int get_auth_code(uint8_t *auth_code, uint32_t *len);
int is_id2_activated(void);
int activate_device(void);

int id2_get_auth_code(uint64_t timestamp, uint8_t *auth_code,
                      uint32_t *auth_len);
int id2_get_digest_auth_code(uint64_t timestamp, uint8_t *digest,
                             uint32_t digest_len, uint8_t *auth_code, uint32_t *auth_len);

#ifdef __cplusplus
}
#endif

#endif // __ID2_H
