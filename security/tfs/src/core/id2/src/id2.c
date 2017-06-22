/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <stdint.h>
#include <string.h>
#include "pal.h"
#include "http.h"
#include "log.h"

#ifdef TFS_EMULATE
#include "emu_id2.h"
#include "emu_rsa.h"
#include "emu_3des.h"
#else
#include "hal_id2.h"
#include "hal_rsa.h"
#include "hal_3des.h"
#endif

// SIGN TYPE
#define TFS_MD5    0
#define TFS_SHA1   1
#define TFS_SHA256 2

// PADDING TYPE
#define TFS_PKCS1_PADDING 0X01 // default
#define TFS_NO_PADDING    0X02

// KEY ID, 3~10:reserved
#define KEY_ID_0  0  // ID2-RSA
#define KEY_ID_18 18 // ID2-3DES

#define ID2_LEN       17
#define SIGN_ALGO_RSA  0
#define SIGN_ALGO_3DES 1

#define SIGN_LEN_3DES 24
#define SIGN_LEN_RSA  128
#define SIGN_LEN_MAX  128

#define SIGN_MODEL 1 // 1: SIGN; 0: ENCRYPT

#define ACTIVATE_SIGN_MODEL "1" // 1: SIGN; 0: ENCRYPT
#define ACTIVATE_VERSION "2.0"

#define FILE_ID2_ACTIVATED "activate"
#define ACTIVATE_SUCCESS_MSG "id2 activate success"
#define ACTIVATE_LEN sizeof(ACTIVATE_SUCCESS_MSG)

#define TAG_ID2 "ID2"

#if defined(TFS_TEE) || defined(TFS_TEE_MOBILE) || defined(TFS_SW)
#define MAX_SIGN_LEN             4096
#define MAX_ENCRYPT_LEN          4096
#else
#define MAX_SIGN_LEN             512
#define MAX_ENCRYPT_LEN          512
#endif

#define RSA_KEY_SIZE             128
#define RSA_MAX_SIZE_PER_SECTION (RSA_KEY_SIZE - 11)

#define MAX_DIGEST_LEN 256
#define ACTIVATE_DEVICE_INFO_LEN 512
#define ACTIVATE_DEVICE_ARGU_LEN 1024

int g_sign_algo = -1;

static int     _g_cached_flag = 0;
static uint8_t _g_cached_ID2[ID2_LEN] = {0};

int _replace_spec_char(char *in, char *out);
static int _get_ID2(uint8_t *id2, uint32_t *len);
static int _get_crypt_algo(void);
static int _get_seed(const char *id2, const char *random,
                     const char *sign, uint32_t sign_len, char *seed);

int get_ID2(uint8_t *id2, uint32_t *len)
{
    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (id2 == NULL || len == NULL) {
        LOGE(TAG_ID2, "[%s]: id2 or len is NULL.\n", __func__);
        return -1;
    }

    if (*len < ID2_LEN) {
        LOGE(TAG_ID2, "[%s]: id2 buf too short.\n", __func__);
        return -1;
    }
    return _get_ID2(id2, len);
}

int id2_sign(const uint8_t *in, uint32_t in_len,
             uint8_t *sign, uint32_t *sign_len)
{
    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (in == NULL || in_len == 0 || sign == NULL || sign_len == NULL) {
        LOGE(TAG_ID2, "[%s]: para wrong.\n", __func__);
        return -1;
    }

    if (in_len > MAX_SIGN_LEN) {
        LOGE(TAG_ID2, "[%s]: in_len must not larger than %d.\n", __func__,
             MAX_SIGN_LEN);
        return -1;
    }

    if ((sign >= in && sign < in + in_len) || (in >= sign &&
                                               in < sign + *sign_len)) {
        LOGE(TAG_ID2, "[%s]: sign buffer should not overlap with in buffer.\n",
             __func__);
        return -1;
    }

    _get_crypt_algo();

    switch (g_sign_algo) {
        case SIGN_ALGO_RSA:
#ifdef TFS_ID2_RSA
#ifdef TFS_EMULATE
            return emu_RSA_sign(KEY_ID_0, in, in_len, sign, sign_len, TFS_MD5);
#else
            return hal_RSA_sign(KEY_ID_0, in, in_len, sign, sign_len, TFS_MD5);
#endif
#else
            return -1;
#endif
        case SIGN_ALGO_3DES:
#ifdef TFS_ID2_3DES
#ifdef TFS_EMULATE
            return emu_3DES_sign(KEY_ID_18, in, in_len, sign, sign_len, TFS_3DES_ECB);
#else
            return hal_3DES_sign(KEY_ID_18, in, in_len, sign, sign_len, TFS_3DES_ECB);
#endif
#else
            return -1;
#endif
        default:
            return -1;
    }
}

int id2_verify(const uint8_t *in, uint32_t in_len,
               uint8_t *sign, uint32_t sign_len)
{
    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (in == NULL || in_len == 0 || sign == NULL || sign_len == 0) {
        LOGE(TAG_ID2, "[%s]: para wrong.\n", __func__);
        return -1;
    }

    if (in_len > MAX_SIGN_LEN) {
        LOGE(TAG_ID2, "[%s]: in_len must not larger than %d.\n", __func__,
             MAX_SIGN_LEN);
        return -1;
    }

    _get_crypt_algo();

    switch (g_sign_algo) {
        case SIGN_ALGO_RSA:
#ifdef TFS_ID2_RSA
#ifdef TFS_EMULATE
            return emu_RSA_verify(KEY_ID_0, in, in_len, sign, sign_len, TFS_MD5);
#else
            return hal_RSA_verify(KEY_ID_0, in, in_len, sign, sign_len, TFS_MD5);
#endif
#else
            return -1;
#endif
        case SIGN_ALGO_3DES:
#ifdef TFS_ID2_3DES
#ifdef TFS_EMULATE
            return emu_3DES_verify(KEY_ID_18, in, in_len, sign, sign_len, TFS_3DES_ECB);
#else
            return hal_3DES_verify(KEY_ID_18, in, in_len, sign, sign_len, TFS_3DES_ECB);
#endif
#else
            return -1;
#endif
        default:
            return -1;
    }
}

int id2_encrypt(uint8_t *in, uint32_t in_len,
                uint8_t *out, uint32_t *out_len)
{
    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (in == NULL || in_len == 0 || out == NULL || out_len == NULL) {
        LOGE(TAG_ID2, "[%s]: para wrong.\n", __func__);
        return -1;
    }

    if (in_len > MAX_ENCRYPT_LEN) {
        LOGE(TAG_ID2, "[%s]: input data len must not larger than %d.\n", __func__,
             MAX_ENCRYPT_LEN);
        return -1;
    }

    if ((out >= in && out < in + in_len) || (in >= out && in < out + *out_len)) {
        LOGE(TAG_ID2, "[%s]: out buffer should not overlap with in buffer.\n",
             __func__);
        return -1;
    }

    _get_crypt_algo();

#ifdef TFS_ID2_RSA
    int ret = -1;
    int i = 0;
    int sections = 0;
    int last_section = 0;
    uint32_t len = 0;
    uint32_t len_sum = 0;
    uint32_t section_len = 0;
#endif

    switch (g_sign_algo) {
        case SIGN_ALGO_RSA:
#ifdef TFS_ID2_RSA
            sections = in_len / RSA_MAX_SIZE_PER_SECTION;
            last_section = in_len % RSA_MAX_SIZE_PER_SECTION;
            section_len = RSA_MAX_SIZE_PER_SECTION;
            if (last_section != 0) {
                sections += 1;
            }

            for (i = 0; i < sections; i ++) {
                if (i == sections - 1) {
                    if (last_section != 0) {
                        section_len = last_section;
                    }
                }

                len = RSA_KEY_SIZE;
#ifdef TFS_EMULATE
                ret = emu_RSA_public_encrypt(KEY_ID_0, in + i * RSA_MAX_SIZE_PER_SECTION,
                                             section_len, out + len_sum, &len, TFS_PKCS1_PADDING);
#else
                ret = hal_RSA_public_encrypt(KEY_ID_0, in + i * RSA_MAX_SIZE_PER_SECTION,
                                             section_len, out + len_sum, &len, TFS_PKCS1_PADDING);
#endif
                if (ret != 0) {
                    LOGE(TAG_ID2, "[%s]: RSA: encrypt error!\n", __func__);
                    return -1;
                }
                len_sum += len;
            }

            *out_len = len_sum;

            return 0;
#else
            return -1;
#endif
        case SIGN_ALGO_3DES:
#ifdef TFS_ID2_3DES
#ifdef TFS_EMULATE
            return emu_3DES_encrypt(KEY_ID_18, in, in_len, out, out_len, TFS_3DES_ECB);
#else
            return hal_3DES_encrypt(KEY_ID_18, in, in_len, out, out_len, TFS_3DES_ECB);
#endif
#else
            return -1;
#endif
        default:
            return -1;
    }
}

int id2_decrypt(uint8_t *in, uint32_t in_len,
                uint8_t *out, uint32_t *out_len)
{
    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (in == NULL || in_len == 0 || out == NULL || out_len == NULL) {
        LOGE(TAG_ID2, "[%s]: para wrong.\n", __func__);
        return -1;
    }

    if (in_len > MAX_ENCRYPT_LEN) {
        LOGE(TAG_ID2, "[%s]: input data len must not larger than %d.\n", __func__,
             MAX_ENCRYPT_LEN);
        return -1;
    }

    if ((out >= in && out < in + in_len) || (in >= out && in < out + *out_len)) {
        LOGE(TAG_ID2, "[%s]: out buffer should not overlap with in buffer.\n",
             __func__);
        return -1;
    }

    _get_crypt_algo();

#ifdef TFS_ID2_RSA
    int ret = 0;
    int i = 0;
    int sections = 0;
    uint32_t len = 0;
    uint32_t len_sum = 0;
#endif

    switch (g_sign_algo) {
        case SIGN_ALGO_RSA:
#ifdef TFS_ID2_RSA
            sections = in_len / RSA_KEY_SIZE;

            for (i = 0; i < sections; i ++) {
                len = RSA_KEY_SIZE;
#ifdef TFS_EMULATE
                ret = emu_RSA_private_decrypt(KEY_ID_0, in + i * RSA_KEY_SIZE,
                                              RSA_KEY_SIZE, out + len_sum, &len, TFS_PKCS1_PADDING);
#else
                ret = hal_RSA_private_decrypt(KEY_ID_0, in + i * RSA_KEY_SIZE,
                                              RSA_KEY_SIZE, out + len_sum, &len, TFS_PKCS1_PADDING);
#endif
                if (ret != 0) {
                    LOGE(TAG_ID2, "[%s]: RSA: decrypt error!\n", __func__);
                    return -1;
                }
                len_sum += len;
            }

            *out_len = len_sum;

            return 0;
#else
            return -1;
#endif
        case SIGN_ALGO_3DES:
#ifdef TFS_ID2_3DES
#ifdef TFS_EMULATE
            return emu_3DES_decrypt(KEY_ID_18, in, in_len, out, out_len, TFS_3DES_ECB);
#else
            return hal_3DES_decrypt(KEY_ID_18, in, in_len, out, out_len, TFS_3DES_ECB);
#endif
#else
            return -1;
#endif
        default:
            return -1;
    }
}

int get_auth_code(uint8_t *auth_code, uint32_t *len)
{
    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (auth_code == NULL || len == NULL) {
        LOGE(TAG_ID2, "[%s]: para error.\n", __func__);
        return -1;
    }

    int ret = 0;
    uint32_t id2_len = ID2_LEN + 1;
    int size = 0;
    uint32_t s_len = SIGN_LEN_MAX;
    char *random = (char *)pal_memory_malloc(5);
    char *seed = (char *)pal_memory_malloc(7);
    char *id2 = (char *)pal_memory_malloc(ID2_LEN + 1);
    char *random_id2 = (char *)pal_memory_malloc(23); // random^ID2
    char *random_id2_signed = (char *)pal_memory_malloc(
                                  SIGN_LEN_MAX); // signed data of ramdom^id2
    char *code_signed = (char *)pal_memory_malloc(SIGN_LEN_MAX);
    char *code_signed_base64 = (char *)pal_memory_malloc(SIGN_LEN_MAX * 2);

    memset(id2, 0, ID2_LEN + 1);
    memset(random, 0, 5);
    memset(seed, 0, 7);
    memset(random_id2, 0, 23);
    memset(random_id2_signed, 0, SIGN_LEN_MAX);
    memset(code_signed, 0, SIGN_LEN_MAX);
    memset(code_signed_base64, 0, SIGN_LEN_MAX * 2);

    ret = _get_ID2((uint8_t *)id2, &id2_len);
    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: get ID2 error.\n", __func__);
        goto error_exit;
    }

    sprintf(random, "%04X", (pal_get_random() & 0X0000FFFF));
    strcat(random_id2, id2);
    strcat(random_id2 + ID2_LEN, "~");
    strcat(random_id2 + ID2_LEN + 1, random);

    ret = id2_sign((uint8_t *)random_id2, 22, (uint8_t *)random_id2_signed, &s_len);
    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: sign data error.\n", __func__);
        goto error_exit;
    }

    ret = _get_seed(id2, random, random_id2_signed, s_len, seed);
    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: get seed error!\n", __func__);
        goto error_exit;
    }

    sprintf((char *)auth_code, "%d", SIGN_MODEL);
    strcat((char *)auth_code + 1 , "~");
    strcat((char *)auth_code + 2 , random);
    strcat((char *)auth_code + 6 , "~");
    strcat((char *)auth_code + 7, seed);
    strcat((char *)auth_code + 13, "~");

    ret = id2_sign((uint8_t *)auth_code + 2, 11, (uint8_t *)code_signed, &s_len);
    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: sign data error.\n", __func__);
        goto error_exit;
    }

    pal_base64_encode((const uint8_t *)code_signed, s_len,
                      (uint8_t *)code_signed_base64, &size);

    LOGD(TAG_ID2, "[%s]: code_signed_out size: %d\n", __func__, size);

    memcpy(auth_code + 14, code_signed_base64, strlen(code_signed_base64));
    *len = strlen((const char *)auth_code);

    LOGD(TAG_ID2, "[%s]: authcode = %s,%d\n", __func__, auth_code, *len);
    ret = 0;

error_exit:
    pal_memory_free(id2);
    pal_memory_free(random);
    pal_memory_free(seed);
    pal_memory_free(random_id2);
    pal_memory_free(random_id2_signed);
    pal_memory_free(code_signed);
    pal_memory_free(code_signed_base64);
    return ret;
}

int id2_get_auth_code(uint64_t timestamp, uint8_t *auth_code,
                      uint32_t *auth_len)
{
    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (auth_code == NULL || auth_len == NULL) {
        LOGE(TAG_ID2, "[%s]: para error.\n", __func__);
        return -1;
    }

    int ret = 0;
    uint32_t id2_len = ID2_LEN + 1;
    int size = 0;
    uint32_t s_len = SIGN_LEN_MAX;
    char *random = (char *)pal_memory_malloc(5);
    char *id2 = (char *)pal_memory_malloc(ID2_LEN + 1);
    char *timestamp_str = (char *)pal_memory_malloc(15);
    char *random_id2_timestamp = (char *)pal_memory_malloc(
                                     38); // ID2^random^timestamp
    char *random_id2_timestamp_signed = (char *)pal_memory_malloc(
                                            SIGN_LEN_MAX); // signed data of ramdom^id2
    char *random_id2_timestamp_signed_base64 = (char *)pal_memory_malloc(
                                                   SIGN_LEN_MAX * 2);

    memset(id2, 0, ID2_LEN + 1);
    memset(random, 0, 5);
    memset(timestamp_str, 0, 15);
    memset(random_id2_timestamp, 0, 38);
    memset(random_id2_timestamp_signed, 0, SIGN_LEN_MAX);
    memset(random_id2_timestamp_signed_base64, 0, SIGN_LEN_MAX * 2);

    ret = _get_ID2((uint8_t *)id2, &id2_len);
    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: get ID2 error.\n", __func__);
        goto error_exit;
    }

    sprintf(random, "%04X", (pal_get_random() & 0X0000FFFF));
    sprintf(timestamp_str, "%llu", timestamp);
    strcat(random_id2_timestamp, id2);
    strcat(random_id2_timestamp + ID2_LEN, random);
    strcat(random_id2_timestamp + ID2_LEN + strlen(random), timestamp_str);

    ret = id2_sign((uint8_t *)random_id2_timestamp, strlen(random_id2_timestamp),
                   (uint8_t *)random_id2_timestamp_signed, &s_len);
    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: sign data error.\n", __func__);
        goto error_exit;
    }

    sprintf((char *)auth_code, "%d", SIGN_MODEL);
    strcat((char *)auth_code + 1 , "~");
    strcat((char *)auth_code + 2 , random);
    strcat((char *)auth_code + 6 , "~");
    strcat((char *)auth_code + 7, timestamp_str);
    strcat((char *)auth_code + 7 + strlen(timestamp_str), "~");

    pal_base64_encode((const uint8_t *)random_id2_timestamp_signed, s_len,
                      (uint8_t *)random_id2_timestamp_signed_base64, &size);

    LOGD(TAG_ID2, "[%s]: code_signed_out size: %d\n", __func__, size);

    memcpy(auth_code + 8 + strlen(timestamp_str),
           random_id2_timestamp_signed_base64, strlen(random_id2_timestamp_signed_base64));
    *auth_len = strlen((const char *)auth_code);

    LOGD(TAG_ID2, "[%s]: authcode = %s,%d\n", __func__, auth_code, *auth_len);
    ret = 0;

error_exit:
    pal_memory_free(id2);
    pal_memory_free(random);
    pal_memory_free(timestamp_str);
    pal_memory_free(random_id2_timestamp);
    pal_memory_free(random_id2_timestamp_signed);
    pal_memory_free(random_id2_timestamp_signed_base64);
    return ret;
}

int id2_get_digest_auth_code(uint64_t timestamp, uint8_t *digest,
                             uint32_t digest_len, uint8_t *auth_code, uint32_t *auth_len)
{
    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (digest == NULL || digest_len == 0 || auth_code == NULL ||
        auth_len == NULL) {
        LOGE(TAG_ID2, "[%s]: para error.\n", __func__);
        return -1;
    }

    if (digest_len > MAX_DIGEST_LEN) {
        LOGE(TAG_ID2, "[%s]: digest len must not larger than %d.\n", __func__,
             MAX_DIGEST_LEN);
        return -1;
    }

    int ret = -1;
    uint32_t id2_len = ID2_LEN + 1;
    int size = 0;
    uint32_t s_len = SIGN_LEN_MAX;
    char *id2 = (char *)pal_memory_malloc(ID2_LEN + 1);
    char *timestamp_str = (char *)pal_memory_malloc(15);
    char *id2_digest_timestamp = (char *)pal_memory_malloc(ID2_LEN + 15 +
                                                           MAX_DIGEST_LEN); // ID2^digest^timestamp
    char *id2_digest_timestamp_signed = (char *)pal_memory_malloc(
                                            SIGN_LEN_MAX); // signed data of id2^digest^timestamp
    char *id2_digest_timestamp_signed_base64 = (char *)pal_memory_malloc(
                                                   SIGN_LEN_MAX * 2);

    memset(id2, 0, ID2_LEN + 1);
    memset(timestamp_str, 0, 15);
    memset(id2_digest_timestamp, 0, ID2_LEN + 15 + MAX_DIGEST_LEN);
    memset(id2_digest_timestamp_signed, 0, SIGN_LEN_MAX);
    memset(id2_digest_timestamp_signed_base64, 0, SIGN_LEN_MAX * 2);

    ret = _get_ID2((uint8_t *)id2, &id2_len);
    if (ret != 0) {
        LOGE(TAG_ID2, "[%s]: get ID2 error.\n", __func__);
        goto error_exit;
    }

    sprintf(timestamp_str, "%llu", timestamp);
    strcat(id2_digest_timestamp, id2);
    strcat(id2_digest_timestamp + ID2_LEN, (char *)digest);
    strcat(id2_digest_timestamp + ID2_LEN + strlen((char *)digest), timestamp_str);

    ret = id2_sign((uint8_t *)id2_digest_timestamp, strlen(id2_digest_timestamp),
                   (uint8_t *)id2_digest_timestamp_signed, &s_len);
    if (ret != 0) {
        LOGE(TAG_ID2, "[%s]: sign data error.\n", __func__);
        goto error_exit;
    }

    pal_base64_encode((const uint8_t *)id2_digest_timestamp_signed, s_len,
                      (uint8_t *)id2_digest_timestamp_signed_base64, &size);

    LOGD(TAG_ID2, "[%s]: code_signed_out size: %d\n", __func__, size);

    sprintf((char *)auth_code, "%d", SIGN_MODEL);
    strcat((char *)auth_code + 1, "~");
    strcat((char *)auth_code + 2, timestamp_str);
    strcat((char *)auth_code + strlen(timestamp_str) + 2, "~");
    strcat((char *)auth_code + strlen(timestamp_str) + 3,
           id2_digest_timestamp_signed_base64);
    *auth_len = strlen((const char *)auth_code);

    LOGD(TAG_ID2, "[%s]: authcode = %s,%d\n", __func__, auth_code, *auth_len);
    ret = 0;

error_exit:
    pal_memory_free(id2);
    pal_memory_free(timestamp_str);
    pal_memory_free(id2_digest_timestamp);
    pal_memory_free(id2_digest_timestamp_signed);
    pal_memory_free(id2_digest_timestamp_signed_base64);
    return ret;
}

int _replace_spec_char(char *in, char *out)
{
    int i = 0;
    int j = 0;
    char *plus  = "%2B";
    char *slash = "%2F";
    char *equal = "%3D";
    int len = strlen(in);

    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    for (i = 0; i < len; i++) {
        switch (in[i]) {
            case '+':
                strncpy(out + j, plus, 3);
                j += 3;
                break;

            case '/':
                strncpy(out + j, slash, 3);
                j += 3;
                break;

            case '=':
                strncpy(out + j, equal, 3);
                j += 3;
                break;

            default:
                strncpy(out + j, in + i, 1);
                j += 1;
        }
    }

    return j;
}

static int _get_ID2(uint8_t *id2, uint32_t *len)
{
    int ret = 0;

    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (_g_cached_flag == 0) {
#ifdef TFS_EMULATE
        ret = emu_get_ID2(_g_cached_ID2, len);
#else
        ret = hal_get_ID2(_g_cached_ID2, len);
#endif

        if (ret != 0) {
            LOGE(TAG_ID2, "[%s]: get ID2 error\n", __func__);
            return -1;
        }

        _g_cached_flag = 1;
    }

    if (id2) {
        memcpy(id2, _g_cached_ID2, ID2_LEN);
        *len = ID2_LEN;
    }

    _get_crypt_algo();

    return 0;
}

static int _get_crypt_algo(void)
{
    int ret;

    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    if (g_sign_algo != -1) {
        return g_sign_algo;
    }

    if (_g_cached_flag != 1) {
        ret = _get_ID2(NULL, NULL);

        if (ret != 0) {
            LOGE(TAG_ID2, "[%s]: get ID2 error.\n", __func__);
            return -1;
        }
    }

    switch (_g_cached_ID2[5]) {
        case '0':
        case '2':
            g_sign_algo = SIGN_ALGO_RSA;
            break;

        case '4':
        case '5':
            g_sign_algo = SIGN_ALGO_3DES;
            break;

        default:
            g_sign_algo = -1;
            LOGE(TAG_ID2, "[%s]: sign algo error.\n", __func__);
    }

    LOGD(TAG_ID2, "[%s]: sign algo is: %d\n", __func__, g_sign_algo);
    return g_sign_algo;
}

static int _get_seed(const char *id2, const char *random,
                     const char *sign, uint32_t sign_len, char *seed)
{
    int ret = -1;
    int size = 0;

    char *argu_str = (char *)pal_memory_malloc(300);
    char *_sign_base64 = (char *)pal_memory_malloc(256);
    char *sign_base64 = (char *) pal_memory_malloc(256);

    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    memset(argu_str, 0, 300);
    memset(_sign_base64, 0, 256);
    memset(sign_base64, 0, 256);

    pal_base64_encode((const uint8_t *)sign, sign_len, (uint8_t *)_sign_base64,
                      &size);
    _replace_spec_char(_sign_base64, sign_base64);

    sprintf(argu_str, "&id2=%s&sid=%s&sign=%s&model=%d",
            id2, random, sign_base64, SIGN_MODEL);

    LOGD(TAG_ID2, "[%s]: argu_str: %s\n", __func__, argu_str);

    ret = http_get_seed(ID2_GET_SEED, argu_str, seed);

    pal_memory_free(argu_str);
    pal_memory_free(_sign_base64);
    pal_memory_free(sign_base64);

    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: get seed error.\n", __func__);
        return -1;
    }

    return 0;
}

int is_id2_activated()
{
    char activate_buf[ACTIVATE_LEN + 1] = {0};
    int ret = -1;

    LOGD(TAG_ID2, "[%s]: enter.\n", __func__);
    ret = pal_get_info(FILE_ID2_ACTIVATED, activate_buf);
    if (ret == -1) {
        LOGE(TAG_ID2, "[%s]: read activate info fail\n", __func__);
        return -1;
    }

    if ((strlen(activate_buf) < ACTIVATE_LEN - 1) ||
        (strncmp(activate_buf, ACTIVATE_SUCCESS_MSG, ACTIVATE_LEN) != 0)) {
        LOGE(TAG_ID2, "[%s]: compare activated msg error!\n", __func__);
        return -1;
    }

    return 0;
}

// flag == 0, for activate info
// flag == 1, for activate argu
static int get_device_info_len(struct device_info dev_info, int flag)
{
    int len = 0;
    int info_num = 0;
    if (dev_info.bt_mac) {
        len += strlen("btMac") + strlen(dev_info.bt_mac);
        info_num ++;
    }

    if (dev_info.build_time) {
        len += strlen("buildTime") + strlen(dev_info.build_time);
        info_num ++;
    }

    if (dev_info.camera_resolution) {
        len += strlen("cameraResolution") + strlen(dev_info.camera_resolution);
        info_num ++;
    }

    if (dev_info.cup_info) {
        len += strlen("cpuInfo") + strlen(dev_info.cup_info);
        info_num ++;
    }

    if (dev_info.dm_dpi) {
        len += strlen("dmDpi") + strlen(dev_info.dm_dpi);
        info_num ++;
    }

    if (dev_info.dm_pixels) {
        len += strlen("dmPixels") + strlen(dev_info.dm_pixels);
        info_num ++;
    }

    if (dev_info.hardware_id) {
        len += strlen("hardwareId") + strlen(dev_info.hardware_id);
        info_num ++;
    }

    if (dev_info.imei) {
        len += strlen("imei") + strlen(dev_info.imei);
        info_num ++;
    }

    if (dev_info.mac) {
        len += strlen("mac") + strlen(dev_info.mac);
        info_num ++;
    }

    if (dev_info.os_version) {
        len += strlen("osVersion") + strlen(dev_info.os_version);
        info_num ++;
    }

    if (dev_info.product_name) {
        len += strlen("productName") + strlen(dev_info.product_name);
        info_num ++;
    }

    if (dev_info.storage_total) {
        len += strlen("storageTotal") + strlen(dev_info.storage_total);
        info_num ++;
    }

    if (flag == 1) {
        len += 2 * info_num;
    }

    return len;
}

static int package_activate_info(char *info, char *argu)
{
    struct device_info dev_info;
    int device_info_len = 0;
    char id2[ID2_LEN + 1] = {0};
    uint32_t id2_len = ID2_LEN + 1;
    int id2_info_len = 0;
    int model_info_len = 0;
    int version_info_len = 0;
    char id2_info[32];
    char model_info[32];
    char version_info[32];
    int ret = -1;

    // get id2
    ret = _get_ID2((uint8_t *)id2, &id2_len);
    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: get ID2 error!\n", __func__);
        return -1;
    }
    LOGD(TAG_ID2, "[%s]: ID2 is %s\n", __func__, id2);
    sprintf(id2_info, "id2%s", id2);
    id2_info_len = strlen(id2_info);

    sprintf(model_info, "model%s", ACTIVATE_SIGN_MODEL);
    model_info_len = strlen(model_info);

    sprintf(version_info, "version%s", ACTIVATE_VERSION);
    version_info_len = strlen(version_info);

    // get device info
    ret = pal_collect_device_info(&dev_info);
    if (ret < 0) {
        LOGE(TAG_ID2, "[%s]: get device info error!\n", __func__);
        return -1;
    }

    device_info_len = get_device_info_len(dev_info, 0);
    // check if info len is enough to contain all info
    if (id2_info_len + model_info_len + version_info_len + device_info_len >
        ACTIVATE_DEVICE_INFO_LEN) {
        LOGE(TAG_ID2, "[%s]: info buffer too short.\n", __func__);
        return -1;
    }

    device_info_len = get_device_info_len(dev_info, 1);
    // check if argu len is enough to contain all info
    if (id2_info_len + model_info_len + version_info_len + device_info_len + 6 >
        ACTIVATE_DEVICE_ARGU_LEN) {
        LOGE(TAG_ID2, "[%s]: argu buffer too short.\n", __func__);
        return -1;
    }

    if (dev_info.bt_mac) {
        sprintf(info, "btMac%s", dev_info.bt_mac);
        sprintf(argu, "&btMac=%s", dev_info.bt_mac);
    }

    if (dev_info.build_time) {
        sprintf(info + strlen(info), "buildTime%s", dev_info.build_time);
        sprintf(argu + strlen(argu), "&buildTime=%s", dev_info.build_time);
    }

    if (dev_info.camera_resolution) {
        sprintf(info + strlen(info), "cameraResolution%s", dev_info.camera_resolution);
        sprintf(argu + strlen(argu), "&cameraResolution=%s",
                dev_info.camera_resolution);
    }

    if (dev_info.cup_info) {
        sprintf(info + strlen(info), "cpuInfo%s", dev_info.cup_info);
        sprintf(argu + strlen(argu), "&cpuInfo=%s", dev_info.cup_info);
    }

    if (dev_info.dm_dpi) {
        sprintf(info + strlen(info), "dmDpi%s", dev_info.dm_dpi);
        sprintf(argu + strlen(argu), "&dmDpi=%s", dev_info.dm_dpi);
    }

    if (dev_info.dm_pixels) {
        sprintf(info + strlen(info), "dmPixels%s", dev_info.dm_pixels);
        sprintf(argu + strlen(argu), "&dmPixels=%s", dev_info.dm_pixels);
    }

    if (dev_info.hardware_id) {
        sprintf(info + strlen(info), "hardwareId%s", dev_info.hardware_id);
        sprintf(argu + strlen(argu), "&hardwareId=%s", dev_info.hardware_id);
    }

    strcat(info + strlen(info), id2_info);
    sprintf(argu + strlen(argu), "&id2=%s", id2);

    if (dev_info.imei) {
        sprintf(info + strlen(info), "imei%s", dev_info.imei);
        sprintf(argu + strlen(argu), "&imei=%s", dev_info.imei);
    }

    if (dev_info.mac) {
        sprintf(info + strlen(info), "mac%s", dev_info.mac);
        sprintf(argu + strlen(argu), "&mac=%s", dev_info.mac);
    }

    strcat(info + strlen(info), model_info);
    sprintf(argu + strlen(argu), "&model=%s", ACTIVATE_SIGN_MODEL);

    if (dev_info.os_version) {
        sprintf(info + strlen(info), "osVersion%s", dev_info.os_version);
        sprintf(argu + strlen(argu), "&osVersion=%s", dev_info.os_version);
    }

    if (dev_info.product_name) {
        sprintf(info + strlen(info), "productName%s", dev_info.product_name);
        sprintf(argu + strlen(argu), "&productName=%s", dev_info.product_name);
    }

    if (dev_info.storage_total) {
        sprintf(info + strlen(info), "storageTotal%s", dev_info.storage_total);
        sprintf(argu + strlen(argu), "&storageTotal=%s", dev_info.storage_total);
    }

    strcat(info + strlen(info), version_info);
    sprintf(argu + strlen(argu), "&version=%s", ACTIVATE_VERSION);

    return 0;
}

int activate_device(void)
{
    LOGD(TAG_ID2, "[%s] enter.\n", __func__);

    int ret = -1;
    char info[ACTIVATE_DEVICE_INFO_LEN] = {0};
    char info_signed[SIGN_LEN_MAX] = {0};
    int info_signed_len = -1;
    char _info_signed_base64[SIGN_LEN_MAX * 2] = {0};
    int _info_signed_base64_len = 0;
    char info_signed_base64[SIGN_LEN_MAX * 2] = {0};
    char argu_str[ACTIVATE_DEVICE_INFO_LEN] = {0};

    if (is_id2_activated() == 0) {
        LOGD(TAG_ID2, "[%s]: the device has already been activated!\n", __func__);
        return 0;
    }

    do {
        ret = package_activate_info(info, argu_str);

        if (ret != 0) {
            LOGE(TAG_ID2, "[%s]: package info or argu error.\n", __func__);
            break;
        }

        LOGD(TAG_ID2, "[%s]: info : %s\n", __func__, info);
        LOGD(TAG_ID2, "[%s]: argu_str : %s\n", __func__, argu_str);
        LOGD(TAG_ID2, "[%s]: info max len 1024, actual len is %d!\n", __func__,
             (int)strlen(info));

        // sign device info
        ret = id2_sign((uint8_t *)info, strlen(info), (uint8_t *)info_signed,
                       (uint32_t *)&info_signed_len);
        if (ret < 0) {
            LOGE(TAG_ID2, "[%s]: sign error!\n", __func__);
            break;
        }
        LOGD(TAG_ID2, "[%s]: info_signed max len 128, actual len is %d!\n", __func__,
             info_signed_len);

        // base 64
        pal_base64_encode((const uint8_t *)info_signed, info_signed_len,
                          (uint8_t *)_info_signed_base64, &_info_signed_base64_len);

        LOGD(TAG_ID2, "[%s]: _info_signed_base64 max len 256, actual len is %d!\n",
             __func__, (int)strlen(_info_signed_base64));

        _replace_spec_char(_info_signed_base64, info_signed_base64);
        LOGD(TAG_ID2, "[%s]: _info_signed_base64 is %s!\n", __func__,
             _info_signed_base64);
        LOGD(TAG_ID2, "[%s]: info_signed_base64 is %s!\n", __func__,
             info_signed_base64);
        LOGD(TAG_ID2, "[%s]: info_signed_base64 max len 256, actual len is %d!\n",
             __func__, (int)strlen(info_signed_base64));

        if (strlen(argu_str) + strlen(info_signed_base64) > ACTIVATE_DEVICE_ARGU_LEN) {
            LOGE(TAG_ID2, "[%s]: argu buffer too short to contain activate info.\n",
                 __func__);
            break;
        }

        sprintf(argu_str + strlen(argu_str), "&sign=%s", info_signed_base64);
        LOGD(TAG_ID2, "[%s]: argu_str max len is %d, actual len is %d!\n", __func__,
             (int)sizeof(argu_str), (int)strlen(argu_str));

        ret = http_activate_dev(ID2_ACTIVATE_DEV, argu_str);

        if (ret != 0) {
            LOGE(TAG_ID2, "[%s]: http_activate_dev error!\n", __func__);
            break;
        }

        ret = pal_save_info(FILE_ID2_ACTIVATED, ACTIVATE_SUCCESS_MSG);
        if (ret < 0) {
            LOGE(TAG_ID2, "[%s]: save activate msg error!\n", __func__);
            break;
        }
    } while (0);

    if (ret != 0) {
        LOGE(TAG_ID2, "[%s]: activate fail!\n", __func__);
    } else {
        LOGD(TAG_ID2, "[%s]: id2 device activate success!\n", __func__);
    }

    return ret;
}
