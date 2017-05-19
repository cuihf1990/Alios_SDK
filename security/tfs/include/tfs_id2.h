/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#ifndef __TFS_ID2_H
#define __TFS_ID2_H

#include <stdint.h>

/* ID2 key started with letter Y */
#define TFS_ID2_LEN    17
/* for 3DES sign len is 16 + 8, for rsa sign len is 128 */
#define TFS_ID2_SIGN_SIZE   (128)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief get ID2 informaiton
 *
 * @param[out] id2: the mem must >= TFS_ID2_LEN.
 * @param[out] len: TFS_ID2_LEN.
 * @return: 0~OK, other~ERROR.
 * @note None.
 */
int tfs_get_ID2(uint8_t *id2, uint32_t *len);


/*
 * @brief sign data with ID2 key
 *
 * @param[in] in: input data.
 * @param[in] in_len: the length of intput data, which must <= 512 bytes.
 * @param[out] sign: signature for input data.
 * @param[out] sign_len: the length of signature.
 * @return: 0~OK, other~ERROR
 * @note None.
 */
int tfs_id2_sign(const uint8_t *in, uint32_t in_len,
                 uint8_t *sign, uint32_t *sign_len);

/*
 * @brief verify signature with ID2 key
 *
 * @param[in] in: input data.
 * @param[in] in_len: the length of intput data, which must <= 512 bytes.
 * @param[in] sign: signature for input data.
 * @param[in] sign_len: the length of signature.
 * @return: 0~OK, other~ERROR
 * @note None.
 */
int tfs_id2_verify(const uint8_t *in, uint32_t in_len,
                   uint8_t *sign, uint32_t sign_len);

/*
 * @brief encrypt data with ID2 key
 *
 * @param[in] in: input data.
 * @param[in] in_len: the length of intput data, which must <= 117 bytes.
 * @param[out] out: encrypted data.
 * @param[out] out_len: the length of encrypted data.
 * @return: 0~OK, other~ERROR
 * @note None.
 */
int tfs_id2_encrypt(uint8_t *in, uint32_t in_len,
                    uint8_t *out, uint32_t *out_len);

/*
 * @brief decrypt cipher text with ID2 key
 *
 * @param[in] in: input data.
 * @param[in] in_len: the length of intput data, which mush <= 128 bytes.
 * @param[out] out: decrypted data.
 * @param[out] out_len: the length of decrypted data.
 * @return: 0~OK, other~ERROR
 * @note None.
 */
int tfs_id2_decrypt(uint8_t *in, uint32_t in_len,
                    uint8_t *out, uint32_t *out_len);

/*
 * @brief get auth code
 *
 * @param[out] auth_code: mode~sid~seed~signature.
 * @param[out] len: auth code length
 * @return: 0~OK, other~ERROR.
 * @note None.
 */
int tfs_get_auth_code(uint8_t *auth_code, uint32_t *len);

/**
 * @brief get activate status
 *
 * @return: 0~activated, other~not activated or read activate info fail.
 * @note None.
 */
int tfs_is_device_activated(void);

/*
 * @brief activate device
 *
 * @return: 0~OK, other~ERROR.
 * @note None.
 */
int tfs_activate_device(void);

/*
 * @brief get auth code with timestamp
 *
 * @param[in]  timestamp: timestamp from caller, the number of milliseconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
 * @param[out] auth_code: model~sid~timestamp~signature.
 * @param[out] auth_len: auth code length.
 * @return: 0~OK, other~ERROR.
 * @note None.
*/

int tfs_id2_get_auth_code(uint64_t timestamp, uint8_t *auth_code, uint32_t *auth_len);


/*
 * @brief get auth code with digest and timestamp
 *
 * @param[in]  timestamp: timestamp from caller, the number of milliseconds since the Epoch, 1970-01-01 00:00:00 +0000 (UTC).
 * @param[in]  digest: data digest
 * @param[in]  digest_len: data digest length
 * @param[out] auth_code: model~timestamp~signature.
 * @param[out] auth_len: auth code length.
 * @return: 0~OK, other~ERROR.
 * @note None.
*/
int tfs_id2_get_digest_auth_code(uint64_t timestamp, uint8_t *digest, uint32_t digest_len, uint8_t *auth_code, uint32_t *auth_len);


#ifdef __cplusplus
}
#endif

#endif
