#ifndef __TFS_AES_H
#define __TFS_AES_H

#include <stdint.h>

/* AES padding type*/
#define TFS_AES_PKCS7_PADDING 0X01
#define TFS_AES_ZERO_PADDING  0X02
#define TFS_AES_NO_PADDING    0X03

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief aes128 cbc encryption
 *
 * @param[in] key: key for aes128, key length should be 16 bytes.
 * @param[in] iv: initial vector for cbc, iv length should be 16 bytes.
 * @param[in] input_len:input data length, when padding type is nopadding, it should be multiple of 16, which must <= 2MB.
 * @param[in] input: input data for encryption.
 * @param[out] out_len:output data length.
 * @param[out] output: output data for encryption .
 * @param[in] padding: padding type, support pkcs7 padding, zero padding and no padding now.
 * @return: 0~OK, other~ERROR.
 * @note None.
 */
int32_t tfs_aes128_cbc_enc(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           int32_t *output_len,
                           uint8_t *output,
                           uint8_t padding);



/**
 * @brief aes128 cbc decryption
 *
 * @param[in] key: key for aes128, key length should be 16 bytes.
 * @param[in] iv: initial vector for cbc, iv length should be 16 bytes.
 * @param[in] input_len:input data length, it should be multiple of 16, which must <= 2MB.
 * @param[in] input: input data for decryption.
 * @param[out] out_len:output data length.
 * @param[out] output: output data for decryption .
 * @param[in] padding: padding type, support pkcs7 padding, zero padding and no padding now.
 * @return: 0~OK, other~ERROR.
 * @note None.
 */
int32_t tfs_aes128_cbc_dec(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           int32_t *output_len,
                           uint8_t *output,
                           uint8_t padding);

#ifdef __cplusplus
}
#endif

#endif
