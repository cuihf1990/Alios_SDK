/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>
#include "log.h"

#ifdef TFS_EMULATE
#include "emu_aes.h"
#else
#include "hal_aes.h"
#endif

#define AES_PKCS7_PADDING 0X01
#define AES_ZERO_PADDING  0X02
#define AES_NO_PADDING    0X03

#define TAG_AES128 "AES128"

int aes128_cbc_enc(const uint8_t *key,
                   const uint8_t *iv,
                   int32_t input_len,
                   const uint8_t *input,
                   int32_t *output_len,
                   uint8_t *output,
                   uint8_t padding)
{
    uint8_t last_section[16];
    int32_t last_section_len = 0;
    unsigned char padding_char;
    int ret = 0;
    LOGD(TAG_AES128, "[%s]: enter.\n", __func__);

    if (key == NULL || iv == NULL || input_len <= 0 || input == NULL ||
        output_len == NULL || output == NULL) {
        LOGE(TAG_AES128, "[%s]: input param error!\n", __func__);
        return -1;
    }

    if (padding != AES_PKCS7_PADDING && padding != AES_ZERO_PADDING &&
        padding != AES_NO_PADDING) {
        LOGE(TAG_AES128, "[%s]: not supported padding!\n", __func__);
        return -1;
    }

    if (padding == AES_NO_PADDING && input_len % 16 != 0) {
        LOGE(TAG_AES128,
             "[%s]: input_len should be multiple of 16 when padding type is nopadding.\n",
             __func__);
        return -1;
    }

    if (padding == AES_NO_PADDING || (padding == AES_ZERO_PADDING &&
                                      input_len % 16 == 0)) {
#ifdef TFS_EMULATE
        ret = emu_aes128_cbc_enc(key, iv, input_len, input, output);
#else
        ret = hal_aes128_cbc_enc(key, iv, input_len, input, output);
#endif
        if (ret != 0) {
            LOGE(TAG_AES128, "[%s]: encrypt error!\n", __func__);
            return -1;
        }
        *output_len = input_len;
    } else {
        memset(last_section, 0, 16);
        last_section_len = input_len % 16;

        if (last_section_len != 0) {
            memcpy(last_section, input + input_len - last_section_len, last_section_len);
        }

        if (padding == AES_PKCS7_PADDING) {
            padding_char = (unsigned char)(16 - last_section_len);
        } else if (padding == AES_ZERO_PADDING) {
            padding_char = 0x00;
        }
        memset(last_section + last_section_len, padding_char, 16 - last_section_len);

#ifdef TFS_EMULATE
        ret = emu_aes128_cbc_enc(key, iv, input_len - last_section_len, input, output);
#else
        ret = hal_aes128_cbc_enc(key, iv, input_len - last_section_len, input, output);
#endif
        if (ret != 0) {
            LOGE(TAG_AES128, "[%s]: encrypt1 error!\n", __func__);
            return -1;
        }

#ifdef TFS_EMULATE
        ret = emu_aes128_cbc_enc(key, iv, 16, last_section,
                                 output + input_len - last_section_len);
#else
        ret = hal_aes128_cbc_enc(key, iv, 16, last_section,
                                 output + input_len - last_section_len);
#endif

        if (ret != 0) {
            LOGE(TAG_AES128, "[%s]: encrypt2 error!\n", __func__);
            return -1;
        }
        *output_len = input_len - last_section_len + 16;
    }
    return 0;
}

int aes128_cbc_dec(const uint8_t *key,
                   const uint8_t *iv,
                   int32_t input_len,
                   const uint8_t *input,
                   int32_t *output_len,
                   uint8_t *output,
                   uint8_t padding)
{
    int ret = 0;
    int i = 0;
    uint8_t last_section[16];
    LOGD(TAG_AES128, "[%s]: enter.\n", __func__);

    if (key == NULL || iv == NULL || input_len <= 0 || input == NULL ||
        output_len == NULL || output == NULL) {
        LOGE(TAG_AES128, "[%s]: input param error!\n", __func__);
        return -1;
    }

    if (padding != AES_PKCS7_PADDING && padding != AES_ZERO_PADDING &&
        padding != AES_NO_PADDING) {
        LOGE(TAG_AES128, "[%s]: not supported padding!\n", __func__);
        return -1;
    }

    if (input_len % 16 != 0) {
        LOGE(TAG_AES128, "[%s]: input_len should be multiple of 16.\n", __func__);
        return -1;
    }

    if (padding == AES_NO_PADDING || padding == AES_ZERO_PADDING) {
#ifdef TFS_EMULATE
        ret = emu_aes128_cbc_dec(key, iv, input_len, input, output);
#else
        ret = hal_aes128_cbc_dec(key, iv, input_len, input, output);
#endif

        if (ret != 0) {
            LOGE(TAG_AES128, "[%s]: decrypt error!\n", __func__);
            return -1;
        }

        *output_len = input_len;
    } else if (padding == AES_PKCS7_PADDING) {
#ifdef TFS_EMULATE
        ret = emu_aes128_cbc_dec(key, iv, input_len - 16, input, output);
#else
        ret = hal_aes128_cbc_dec(key, iv, input_len - 16, input, output);
#endif

        if (ret != 0) {
            LOGE(TAG_AES128, "[%s]: decrypt error1!\n", __func__);
            return -1;
        }

#ifdef TFS_EMULATE
        ret = emu_aes128_cbc_dec(key, iv, 16, input + input_len - 16, last_section);
#else
        ret = hal_aes128_cbc_dec(key, iv, 16, input + input_len - 16, last_section);
#endif

        if (ret != 0) {
            LOGE(TAG_AES128, "[%s]: decrypt error2!\n", __func__);
            return -1;
        }

        if ((unsigned char)last_section[15] > 16) {
            LOGE(TAG_AES128, "[%s]: decrypted data error1!\n", __func__);
            return -1;
        }

        for (i = 16 - last_section[15]; i < 16; i ++) {
            if (last_section[i] != last_section[15]) {
                break;
            }
        }
        if (i != 16) {
            LOGE(TAG_AES128, "[%s]: decrypted data error2!\n", __func__);
            return -1;
        }

        if (last_section[15] < 16) {
            memcpy(output + input_len - 16, last_section, 16 - last_section[15]);
        }
        *output_len = input_len - last_section[15];
    }
    return 0;
}
