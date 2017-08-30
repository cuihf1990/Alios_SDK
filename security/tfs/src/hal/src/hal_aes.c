#include <string.h>
#include "pal.h"
#include "log.h"
#include "hal_aes.h"

#if defined(TFS_TEE)
#include "tee_aes.h"
#endif

#define TAG_HAL_AES "TFS_HAL_AES"

int32_t hal_aes128_cbc_enc(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           uint8_t *output)
{
    LOGD(TAG_HAL_AES, "[%s]: enter.\n", __func__);
#if defined(TFS_TEE)
    return tee_aes_cbc_encrypt(key, iv, input_len, input, output);
#else
    return -1;
#endif
}

int32_t hal_aes128_cbc_dec(const uint8_t *key,
                           const uint8_t *iv,
                           int32_t input_len,
                           const uint8_t *input,
                           uint8_t *output)
{
    LOGD(TAG_HAL_AES, "[%s]: enter.\n", __func__);
#if defined(TFS_TEE)
    return tee_aes_cbc_decrypt(key, iv, input_len, input, output);
#else
    return -1;
#endif
}

