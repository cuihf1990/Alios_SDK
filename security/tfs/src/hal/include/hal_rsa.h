#ifndef _HAL_RSA_H
#define _HAL_RSA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int hal_RSA_sign(uint8_t ID, const uint8_t *in, uint32_t in_len,
                 uint8_t *sign, uint32_t *sign_len, uint8_t type);
int hal_RSA_verify(uint8_t ID, const uint8_t *in, uint32_t in_len,
                   uint8_t *sign, uint32_t sign_len, uint8_t type);
int hal_RSA_public_encrypt(uint8_t ID, const uint8_t *in, uint32_t in_len,
                           uint8_t *out, uint32_t *out_len, uint8_t padding);
int hal_RSA_private_decrypt(uint8_t ID, uint8_t *in, uint32_t in_len,
                            uint8_t *out, uint32_t *out_len, uint8_t padding);

#ifdef __cplusplus
}
#endif

#endif
