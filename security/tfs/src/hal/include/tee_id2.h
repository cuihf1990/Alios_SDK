/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef _TEE_ID2_H_
#define _TEE_ID2_H_

#include <stdint.h>

/* rsa padding type */
#define RSA_PAD_NO_PAD             (0x0)
#define RSA_PAD_PKCS1_V1_5         (0x1)

/* rsa sign type */
#define RSA_SIGN_PKCS1_V1_5_MD5    (0x0)
#define RSA_SIGN_PKCS1_V1_5_SHA1   (0x1)

#ifdef __cplusplus
extern "C" {
#endif

/*
 * get ID2 ID
 *
 * @id2[out]: id2 id buffer
 * @len[inout]: in  - id2 buffer length
 *              out - actual id2 id length
 * @return: 0~OK, other~ERROR.
 */
int tee_get_ID2(uint8_t *id2, uint32_t *len);

/*
 * get ID2 public key
 *
 * @pub[out]: public key buffer
 * @len[inout]: in  - public key buffer length
 *              out - actual public key length
 * @return: 0~OK, other~ERROR.
 */
int tee_ID2_get_pub_key(uint8_t *pub, uint32_t *len);

/*
 * ID2 Private key sign
 *
 * @in[in]:  in buffer
 * @in_len[in]: in buffer length
 * @sign[out]: sign buffer
 * @sign_len[inout]: in  - sign buffer length
 *                   out - actual sign length
 * @sign_type[in]: rsa sign type
 * @return: 0~OK, other~ERROR.
 */
int tee_ID2_sign(const uint8_t *in, uint32_t in_len,
                 uint8_t *sign, uint32_t *sign_len, uint32_t sign_type);

/*
 * ID2 public key verify
 *
 * @in[in]:  in buffer
 * @in_len[in]: in buffer length
 * @sign[out]: sign buffer
 * @sign_len[in]: sign buffer length
 * @sign_type[in]: rsa sign type
 * @return: 0~OK, other~ERROR.
 */
int tee_ID2_verify(const uint8_t *in, uint32_t in_len,
                   uint8_t *sign, uint32_t sign_len, uint32_t sign_type);


/*
 * ID2 public key encrypt
 *
 * @in[in]:  in buffer
 * @in_len[in]: in buffer length
 * @out[out]: out buffer
 * @out_len[inout]: in  - out buffer length
 *                  out - actual out length
 * @padding[in]: rsa padding type
 * @return: 0~OK, other~ERROR.
 */
int tee_ID2_encrypt(const uint8_t *in, uint32_t in_len,
                    uint8_t *out, uint32_t *out_len, uint32_t padding);

/*
 * ID2 private key decrypt
 *
 * @in[in]:  in buffer
 * @in_len[in]: in buffer length
 * @out[out]: out buffer
 * @out_len[inout]: in  - out buffer length
 *                  out - actual out length
 * @padding[in]: rsa padding type
 * @return: 0~OK, other~ERROR.
 */
int tee_ID2_decrypt(const uint8_t *in, uint32_t in_len,
                    uint8_t *out, uint32_t *out_len, uint32_t padding);

int tee_RSA_sign(uint8_t ID, const uint8_t *in, uint32_t in_len,
                 uint8_t *sign, uint32_t *sign_len, uint8_t type);
int tee_RSA_verify(uint8_t ID, const uint8_t *in, uint32_t in_len,
                   uint8_t *sign, uint32_t sign_len, uint8_t type);
int tee_RSA_public_encrypt(uint8_t ID, const uint8_t *in, uint32_t in_len,
                           uint8_t *out, uint32_t *out_len, uint8_t padding);
int tee_RSA_private_decrypt(uint8_t ID, uint8_t *in, uint32_t in_len,
                            uint8_t *out, uint32_t *out_len, uint8_t padding);

#ifdef __cplusplus
}
#endif

#endif /* _TEE_ID2_H_ */
