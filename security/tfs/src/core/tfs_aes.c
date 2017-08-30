#include "aes128.h"

int tfs_aes128_cbc_enc(const uint8_t *key,
                       const uint8_t *iv,
                       int32_t input_len,
                       const uint8_t *input,
                       int32_t *output_len,
                       uint8_t *output,
                       uint8_t padding)
{
    return aes128_cbc_enc(key, iv, input_len, input, output_len, output, padding);
}

int tfs_aes128_cbc_dec(const uint8_t *key,
                       const uint8_t *iv,
                       int32_t input_len,
                       const uint8_t *input,
                       int32_t *output_len,
                       uint8_t *output,
                       uint8_t padding)
{
    return aes128_cbc_dec(key, iv, input_len, input, output_len, output, padding);
}
