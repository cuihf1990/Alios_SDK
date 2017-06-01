/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <string.h>
#if defined(TFS_OPENSSL)
#include "openssl/des.h"
#else
#include "des.h"
#endif
#include "keys_3des.h"
#include "pal.h"
#include "log.h"
#include "emu_3des.h"

#define MD5_SIZE      16
#define SIGN_IN_MAX   512
#define SIGN_OUT_MAX  256

#define TAG_EMU_3DES "EMU_3DES"

int emu_3DES_sign(uint8_t ID, const uint8_t *in, uint32_t in_len,
                  uint8_t *sign, uint32_t *sign_len, uint8_t mode)
{
    uint8_t sign_out_buf[SIGN_OUT_MAX];
    uint8_t md5_buf[MD5_SIZE];
    uint32_t len;

    pal_md5_sum(in, in_len, md5_buf);

    emu_3DES_encrypt(ID, md5_buf, MD5_SIZE, sign_out_buf, &len, mode);

    if (len > SIGN_OUT_MAX)
        return -1;
    memcpy(sign, sign_out_buf, len);
    *sign_len = len;

    return 0;
}

int emu_3DES_verify(uint8_t ID, const uint8_t *in, uint32_t in_len,
                    uint8_t *sign, uint32_t sign_len, uint8_t mode)
{
    uint8_t sign_out_buf[SIGN_OUT_MAX];
    uint32_t len;
    int ret;

    ret = emu_3DES_sign(ID, in, in_len, sign_out_buf, &len, mode);
    if (ret != 0) {
        LOGE(TAG_EMU_3DES, "[%s]: emu_3DES_sign error!\n", __func__);
        return -1;
    }
    if (len != sign_len)
        return -1;
    return memcmp(sign, sign_out_buf, sign_len);
}

int emu_3DES_encrypt(uint8_t ID, const uint8_t *in, uint32_t in_len,
                     uint8_t *out, uint32_t *out_len, uint8_t mode)
{
    uint32_t len;
    uint32_t padding;
    uint8_t *_in;
#if defined(TFS_OPENSSL)
    int i = 0;
    DES_key_schedule ks1, ks2, ks3;
    DES_cblock ivec = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#else
    int32_t ret;
#endif

    if (in == NULL || in_len <= 0 || out == NULL || out_len == NULL
        || (mode != TFS_3DES_ECB && mode != TFS_3DES_CBC)) {
        LOGE(TAG_EMU_3DES, "[%s]: wrong para!\n", __func__);
        return -1;
    }

#if defined(TFS_OPENSSL)
    DES_set_key_unchecked((C_Block *)g_3des_key, &ks1);
    DES_set_key_unchecked((C_Block *)(g_3des_key + 8), &ks2);
    DES_set_key_unchecked((C_Block *)(g_3des_key + 16), &ks3);
#endif

    padding = 8 - (in_len % 8);
    len = in_len + padding;
    _in = (uint8_t *)pal_memory_malloc(len);
    if (_in == NULL) {
        LOGE(TAG_EMU_3DES, "[%s]: malloc error!\n", __func__);
        return -1;
    }
    memcpy(_in, in, in_len);
    memset(_in + in_len, (uint8_t)padding, padding);
    *out_len = len;

#if defined(TFS_OPENSSL)
    for (i = 0; i < len; i += 8) {
        if (mode == TFS_3DES_ECB) {
            DES_ecb3_encrypt((C_Block *)(_in + i), (C_Block *)(out + i), &ks1, &ks2, &ks3, DES_ENCRYPT);
        } else if (mode == TFS_3DES_CBC) {
            DES_ede3_cbc_encrypt(_in + i, out + i, 8, &ks1, &ks2, &ks3, &ivec, DES_ENCRYPT);
        }
    }
#else
    ret = des3_en((unsigned char *)g_3des_key, DES3_KEY_SIZE, mode - 1, _in, len, out, 0);
#endif
    pal_memory_free(_in);

#if defined(TFS_OPENSSL)
    return 0;
#else
    return ret;
#endif
}

int emu_3DES_decrypt(uint8_t ID, uint8_t *in, uint32_t in_len,
                     uint8_t *out, uint32_t *out_len, uint8_t mode)
{
    uint8_t *_out;
    uint32_t len;
#if defined(TFS_OPENSSL)
    int i = 0;
    DES_key_schedule ks1, ks2, ks3;
    DES_cblock ivec = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#else
    int32_t ret;
#endif

    if (in == NULL || in_len <= 0 || out == NULL || out_len == NULL
        || (mode != TFS_3DES_ECB && mode != TFS_3DES_CBC)) {
        LOGE(TAG_EMU_3DES, "[%s]: wrong para!\n", __func__);
        return -1;
    }

#if defined(TFS_OPENSSL)
    DES_set_key_unchecked((C_Block *)g_3des_key, &ks1);
    DES_set_key_unchecked((C_Block *)(g_3des_key + 8), &ks2);
    DES_set_key_unchecked((C_Block *)(g_3des_key + 16), &ks3);
#endif

    _out = (uint8_t *)pal_memory_malloc(in_len);
    if (_out == NULL) {
        LOGE(TAG_EMU_3DES, "[%s]: malloc error!\n", __func__);
        return -1;
    }

#if defined(TFS_OPENSSL)
    for (i = 0; i < in_len; i += 8) {
        if (mode == TFS_3DES_ECB) {
            DES_ecb3_encrypt((C_Block *)(in + i), (C_Block *)(_out + i), &ks1, &ks2, &ks3, DES_DECRYPT);
        } else if (mode == TFS_3DES_CBC) {
            DES_ede3_cbc_encrypt(in + i, _out + i, 8, &ks1, &ks2, &ks3, &ivec, DES_DECRYPT);
        }
    }
#else
    ret = des3_de((unsigned char *)g_3des_key, DES3_KEY_SIZE, mode - 1, in, in_len, _out, 0);
#endif

    len = in_len - _out[in_len - 1];
    memcpy(out, _out, len);
    pal_memory_free(_out);
    *out_len = len;

#if defined(TFS_OPENSSL)
    return 0;
#else
    return ret;
#endif
}
