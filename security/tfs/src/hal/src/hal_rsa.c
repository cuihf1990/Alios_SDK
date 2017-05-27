/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <string.h>
#include "pal.h"
#include "log.h"
#include "hal_rsa.h"

#define PUB_KEY_MAX   300
#define SIGN_IN_MAX   512
#define SIGN_OUT_MAX  256
#define CRYPT_IN_MAX  512
#define CRYPT_OUT_MAX 512

#if defined(TFS_TEE) || defined(TFS_TEE_MOBILE)
#include "tee_id2.h"
#elif defined(TFS_SW)
#include "sm_id2.h"
#else
#include "cmd.h"
#endif

#define TAG_HAL_RSA "TFS_HAL_RSA"

int hal_RSA_sign(uint8_t ID, const uint8_t *in, uint32_t in_len,
                 uint8_t *sign, uint32_t *sign_len, uint8_t type)
{
    int ret = 0;

    LOGD(TAG_HAL_RSA, "[%s]: enter.\n", __func__);
#if defined(TFS_TEE)
    *sign_len = 128;
    ret = tee_RSA_sign(ID, in, in_len, sign, sign_len, type);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:tee env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_TEE_MOBILE)
    *sign_len = 128;
    ret =  tee_ID2_sign(in, in_len, sign, sign_len, type);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:tee mobile env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_SW)
    *sign_len = 128;
    ret =  tee_RSA_sign(ID, in, in_len, sign, sign_len, type);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:sw env error!\n", __func__);
        return -1;
    }
#else
    uint32_t _in_len = 0;
    uint8_t *_in = NULL;
    uint32_t out_len = 0;
    uint8_t *out = (uint8_t*)pal_memory_malloc(SIGN_OUT_MAX + 11);
    uint8_t *arg = NULL;
    memset(out, 0, SIGN_OUT_MAX + 11);

    if (in_len > SIGN_IN_MAX) {
        LOGE(TAG_HAL_RSA, "[%s]: input data error!\n", __func__);
        pal_memory_free(out);
        return -1;
    }

    _in_len = in_len + 13;
    _in = (uint8_t *)pal_memory_malloc(_in_len);
    if (_in == NULL) {
        LOGE(TAG_HAL_RSA, "[%s]: malloc error!\n", __func__);
        pal_memory_free(out);
        return -1;
    }

    arg = (uint8_t *)pal_memory_malloc(in_len + 5);
    if (arg == NULL) {
        LOGE(TAG_HAL_RSA, "[%s]: malloc error!\n", __func__);
        pal_memory_free(_in);
        pal_memory_free(out);
        return -1;
    }
    arg[0] = ID;
    arg[1] = (in_len >> 8) & 0XFF;
    arg[2] = in_len & 0XFF;
    memcpy(&arg[3], in, in_len);
    arg[3 + in_len] = type;
    arg[4 + in_len] = TFS_PKCS1_PADDING;

    fill_package(_in, CMD_RSA_SIGN, arg, in_len + 5);
    pal_memory_free(arg);

    ret = hal_cmd(CMD_RSA_SIGN, _in, _in_len, out, &out_len);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]: hal_cmd error!\n", __func__);
        pal_memory_free(out);
        pal_memory_free(_in);
        return -1;
    }
    pal_memory_free(_in);

    if (out[out_len - 3] != RES_OK) {
        LOGE(TAG_HAL_RSA, "[%s]: response error!\n", __func__);
        pal_memory_free(out);
        return -1;
    }

    *sign_len = (out[6] & 0XFF) << 8;
    *sign_len |= out[7] & 0XFF;
    if (*sign_len > SIGN_OUT_MAX) {
        LOGE(TAG_HAL_RSA, "[%s]: pub-key too long!\n", __func__);
        pal_memory_free(out);
        return -1;
    }

    memcpy(sign, out + 8, *sign_len);
    pal_memory_free(out);

#endif
    return 0;
}

int hal_RSA_verify(uint8_t ID, const uint8_t *in, uint32_t in_len,
                   uint8_t *sign, uint32_t sign_len, uint8_t type)
{
    int ret = 0;

    LOGD(TAG_HAL_RSA, "[%s]: enter.\n", __func__);
#if defined(TFS_TEE)
    ret = tee_RSA_verify(ID, in, in_len, sign, sign_len, type);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:tee env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_TEE_MOBILE)
    ret =  tee_ID2_verify(in, in_len, sign, sign_len, type);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:tee mobile env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_SW)
    ret =  tee_RSA_verify(ID, in, in_len, sign, sign_len, type);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:sw env error!\n", __func__);
        return -1;
    }
#else
    uint32_t _in_len = 0;
    uint8_t *_in = NULL;
    uint32_t out_len = 0;
    uint8_t out[9] = {0};
    uint8_t *arg = NULL;

    if (in_len > SIGN_IN_MAX || sign_len > SIGN_OUT_MAX) {
        LOGE(TAG_HAL_RSA, "[%s]: input data error!\n", __func__);
        return -1;
    }

    _in_len = in_len + sign_len + 15;
    _in = (uint8_t *)pal_memory_malloc(_in_len);
    if (_in == NULL) {
        LOGE(TAG_HAL_RSA, "[%s]: malloc error!\n", __func__);
        return -1;
    }

    arg = (uint8_t *)pal_memory_malloc(in_len + sign_len + 7);
    if (arg == NULL) {
        LOGE(TAG_HAL_RSA, "[%s]: malloc error!\n", __func__);
        pal_memory_free(_in);
        return -1;
    }
    arg[0] = ID;
    arg[1] = (in_len >> 8) & 0XFF;
    arg[2] = in_len & 0XFF;
    memcpy(&arg[3], in, in_len);
    arg[3 + in_len] = (sign_len >> 8) & 0XFF;
    arg[4 + in_len] = sign_len & 0XFF;
    memcpy(&arg[5 + in_len], sign, sign_len);
    arg[5 + in_len + sign_len] = type;
    arg[6 + in_len + sign_len] = TFS_PKCS1_PADDING;

    fill_package(_in, CMD_RSA_VERIFY, arg, in_len + sign_len + 7);
    pal_memory_free(arg);

    ret = hal_cmd(CMD_RSA_VERIFY, _in, _in_len, out, &out_len);
    if (ret != 0 || out_len != 9) {
        LOGE(TAG_HAL_RSA, "[%s]: hal_cmd error!\n", __func__);
        pal_memory_free(_in);
        return -1;
    }
    pal_memory_free(_in);

    if (out[out_len - 3] != RES_OK) {
        LOGE(TAG_HAL_RSA, "[%s]: response error!\n", __func__);
        return -1;
    }

#endif
    return 0;
}

int hal_RSA_public_encrypt(uint8_t ID, const uint8_t *in, uint32_t in_len,
                           uint8_t *out, uint32_t *out_len, uint8_t padding)
{
    int ret = 0;

    LOGD(TAG_HAL_RSA, "[%s]: enter.\n", __func__);
#if defined(TFS_TEE)
    *out_len = 128;
    ret = tee_RSA_public_encrypt(ID, in, in_len, out, out_len, padding);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:tee env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_TEE_MOBILE)
    *out_len = 128;
    ret =  tee_ID2_encrypt(in, in_len, out, out_len, padding);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:tee mobile env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_SW)
    *out_len = 128;
    ret =  tee_RSA_public_encrypt(ID, in, in_len, out, out_len, padding);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:sw env error!\n", __func__);
        return -1;
    }
#else

    uint32_t _in_len = 0;
    uint8_t *_in = NULL;
    uint32_t _out_len = 0;
    uint8_t _out[CRYPT_OUT_MAX + 11] = {0};
    uint8_t *arg = NULL;

    if (in_len > CRYPT_IN_MAX) {
        LOGE(TAG_HAL_RSA, "[%s]: input data error!\n", __func__);
        return -1;
    }

    _in_len = in_len + 12;
    _in = (uint8_t *)pal_memory_malloc(_in_len);
    if (_in == NULL) {
        LOGE(TAG_HAL_RSA, "[%s]: malloc error!\n", __func__);
        return -1;
    }

    arg = (uint8_t *)pal_memory_malloc(in_len + 4);
    if (arg == NULL) {
        LOGE(TAG_HAL_RSA, "[%s]: malloc error!\n", __func__);
        pal_memory_free(_in);
        return -1;
    }
    arg[0] = ID;
    arg[1] = (in_len >> 8) & 0XFF;
    arg[2] = in_len & 0XFF;
    memcpy(&arg[3], in, in_len);
    arg[3 + in_len] = padding;
    fill_package(_in, CMD_RSA_ENCRYPT, arg, in_len + 4);
    pal_memory_free(arg);

    ret = hal_cmd(CMD_RSA_ENCRYPT, _in, _in_len, _out, &_out_len);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]: hal_cmd error!\n", __func__);
        pal_memory_free(_in);
        return -1;
    }
    pal_memory_free(_in);

    if (_out[_out_len - 3] != RES_OK) {
        LOGE(TAG_HAL_RSA, "[%s]: response error!\n", __func__);
        return -1;
    }

    *out_len = (_out[6] & 0XFF) << 8;
    *out_len |= _out[7] & 0XFF;
    LOGD(TAG_HAL_RSA, "out_len: %d\n", (int)(*out_len));
    if (*out_len > CRYPT_OUT_MAX) {
        LOGE(TAG_HAL_RSA, "[%s]: input data error!\n", __func__);
        return -1;
    }

    memcpy(out, _out + 8, *out_len);

#endif
    return 0;
}

int hal_RSA_private_decrypt(uint8_t ID, uint8_t *in, uint32_t in_len,
                            uint8_t *out, uint32_t *out_len, uint8_t padding)
{
    int ret = 0;

    LOGD(TAG_HAL_RSA, "[%s]: enter.\n", __func__);
#if defined(TFS_TEE)
    ret = tee_RSA_private_decrypt(ID, in, in_len, out, out_len, padding);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:tee env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_TEE_MOBILE)
    ret =  tee_ID2_decrypt(in, in_len, out, out_len, padding);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:tee mobile env error!\n", __func__);
        return -1;
    }
#elif defined(TFS_SW)
    ret = tee_RSA_private_decrypt(ID, in, in_len, out, out_len, padding);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]:sw env error!\n", __func__);
        return -1;
    }
#else

    uint32_t _in_len = 0;
    uint8_t *_in = NULL;
    uint32_t _out_len = 0;
    uint8_t _out[CRYPT_OUT_MAX + 11] = {0};
    uint8_t *arg = NULL;

    if (in_len > CRYPT_IN_MAX) {
        LOGE(TAG_HAL_RSA, "[%s]: input data error!\n", __func__);
        return -1;
    }

    _in_len = in_len + 12;
    _in = (uint8_t *)pal_memory_malloc(_in_len);
    if (_in == NULL) {
        LOGE(TAG_HAL_RSA, "[%s]: malloc error!\n", __func__);
        return -1;
    }

    arg = (uint8_t *)pal_memory_malloc(in_len + 4);
    if (arg == NULL) {
        LOGE(TAG_HAL_RSA, "[%s]: malloc error!\n", __func__);
        pal_memory_free(_in);
        return -1;
    }
    arg[0] = ID;
    arg[1] = (in_len >> 8) & 0XFF;
    arg[2] = in_len & 0XFF;
    memcpy(&arg[3], in, in_len);
    arg[3 + in_len] = padding;
    fill_package(_in, CMD_RSA_DECRYPT, arg, in_len + 4);
    pal_memory_free(arg);

    ret = hal_cmd(CMD_RSA_DECRYPT, _in, _in_len, _out, &_out_len);
    if (ret != 0) {
        LOGE(TAG_HAL_RSA, "[%s]: hal_cmd error!\n", __func__);
        pal_memory_free(_in);
        return -1;
    }
    pal_memory_free(_in);

    if (_out[_out_len - 3] != RES_OK) {
        LOGE(TAG_HAL_RSA, "[%s]: response error!\n", __func__);
        return -1;
    }

    *out_len = (_out[6] & 0XFF) << 8;
    *out_len |= _out[7] & 0XFF;
    if (*out_len > CRYPT_OUT_MAX) {
        LOGE(TAG_HAL_RSA, "[%s]: input data error!\n", __func__);
        return -1;
    }

    memcpy(out, _out + 8, *out_len);

#endif
    return 0;
}
