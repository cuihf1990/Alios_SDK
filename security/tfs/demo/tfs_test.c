/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "pal.h"
#include "tfs_id2.h"
#include "tfs_aes.h"
#include "log.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
static void prepare_test_data();
static int test_tfs_get_ID2(void);
static int test_tfs_id2_sign(void);
static int test_tfs_id2_verify(void);
static int test_tfs_id2_encrypt(void);
static int test_tfs_id2_decrypt(void);
static int test_tfs_get_auth_code(void);
static int test_tfs_activate_device(void);
static int test_tfs_id2_get_auth_code(void);
static int test_tfs_id2_get_digest_auth_code(void);
static int test_tfs_aes128_enc_dec(int size, uint8_t padding);
static int test_tfs_aes128_enc_dec_performance(int size, uint8_t padding);

/****************************************************************************
 * Private Data
 ****************************************************************************/
//const char *in_data = "Y007A401104D230A4~6042"; //encode & sign in for RSA & 3DES
//const char *in_data = "1234567890"; //encode & sign in for RSA & 3DES
#define IN_DATA_SIZE 117
char in_data[IN_DATA_SIZE + 1];
#define BUF_MAX 256
static uint8_t out_data[BUF_MAX] = {0};
static uint8_t dec_out[BUF_MAX] = {0};

uint64_t start;
uint64_t end;

//#define DEBUG_PRESS
#ifdef DEBUG_PRESS
#define PRESS_TIME 1000
#endif

#define TAG_TFS_TEST "TFS_TEST"

void tfs_test() {
    prepare_test_data();
    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_get_ID2 <<<<<<\n");
    test_tfs_get_ID2();

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_id2_sign <<<<<<\n");
    test_tfs_id2_sign();

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_id2_verify <<<<<<\n");
    test_tfs_id2_verify();

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_id2_encrypt <<<<<<\n");
    test_tfs_id2_encrypt();

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_id2_decrypt <<<<<<\n");
    test_tfs_id2_decrypt();

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_get_auth_code <<<<<<\n");
    test_tfs_get_auth_code();

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_activate_device <<<<<<\n");
    test_tfs_activate_device();

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_id2_get_auth_code <<<<<<\n");
    test_tfs_id2_get_auth_code();

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_id2_get_digest_auth_code <<<<<<\n");
    test_tfs_id2_get_digest_auth_code();
	
    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_aes128_enc_dec <<<<<<\n");
    LOGI(TAG_TFS_TEST,">>>> TFS_AES_PKCS7_PADDING <<<<\n");
    test_tfs_aes128_enc_dec(50, TFS_AES_PKCS7_PADDING);
    test_tfs_aes128_enc_dec(64, TFS_AES_PKCS7_PADDING);
    LOGI(TAG_TFS_TEST,">>>> TFS_AES_ZERO_PADDING <<<<\n");
    test_tfs_aes128_enc_dec(50, TFS_AES_ZERO_PADDING);
    test_tfs_aes128_enc_dec(64, TFS_AES_ZERO_PADDING);
    LOGI(TAG_TFS_TEST,">>>> TFS_AES_NO_PADDING <<<<\n");
    test_tfs_aes128_enc_dec(50, TFS_AES_NO_PADDING);
    test_tfs_aes128_enc_dec(64, TFS_AES_NO_PADDING);

    LOGI(TAG_TFS_TEST,">>>>>>func: test_tfs_aes128_enc_dec_performance <<<<<<\n");
    LOGI(TAG_TFS_TEST,">>>> TFS_AES_PKCS7_PADDING <<<<\n");
    test_tfs_aes128_enc_dec_performance(4*1024, TFS_AES_PKCS7_PADDING);
    LOGI(TAG_TFS_TEST,">>>> TFS_AES_ZERO_PADDING <<<<\n");
    test_tfs_aes128_enc_dec_performance(4*1024, TFS_AES_ZERO_PADDING);
    LOGI(TAG_TFS_TEST,">>>> TFS_AES_NO_PADDING <<<<\n");
    test_tfs_aes128_enc_dec_performance(4*1024, TFS_AES_NO_PADDING);
}

static void prepare_test_data() {
    int i = 0;
    for (i = 0;i < IN_DATA_SIZE;i ++) {
        in_data[i] = i%10 + '0';
    }
    in_data[IN_DATA_SIZE] = '\0';
}

static inline void hexdump(const uint8_t *str, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++) {
        LOG("%02X,", *str++);
        if ((i + 1) % 32 == 0)
            LOG("\n");
    }
    LOG("\n\n");
}

static int test_tfs_get_ID2(void)
{
    int ret = 0;
    uint32_t len = TFS_ID2_LEN + 1;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};
    uint64_t cost_time;

    start = pal_get_current_time();
    ret = tfs_get_ID2(id2, &len);
    end = pal_get_current_time();
    cost_time = end - start;

    LOGI(TAG_TFS_TEST,"\ntfs_get_ID2: ret = %d, the ID2(%d): %s\n\ntime: %lld\n\n", ret, len, id2, cost_time);
    return 0;
}

static int test_tfs_id2_sign(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = strlen(in_data);
    uint32_t id2_len = TFS_ID2_LEN + 1;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};
    uint64_t cost_time;
#ifdef DEBUG_PRESS
    uint64_t total_time;
    uint64_t average_time;
    int i;
#endif

    tfs_get_ID2(id2, &id2_len);

    memset(out_data, 0, BUF_MAX);
#ifdef DEBUG_PRESS
    total_time = 0;
    for (i = 0; i < PRESS_TIME; i ++) {
    //LOGI(TAG_TFS_TEST,"sign: %d time.", i + 1);
#endif
    start = pal_get_current_time();
    ret = tfs_id2_sign((const uint8_t *)in_data, strlen(in_data), out_data, &len);
    end = pal_get_current_time();
#ifdef DEBUG_PRESS
    total_time += end - start;
    }
    average_time = total_time/PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end -start;
#endif
    LOGI(TAG_TFS_TEST,"\ntfs_id2_sign: ret = %d, sign out(%d) \n\ntime:%lld\n\n", ret, len, cost_time);
    hexdump(out_data, len);

    return 0;
}

static int test_tfs_id2_verify(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = strlen(in_data);
    uint32_t id2_len = TFS_ID2_LEN + 1;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};
    uint64_t cost_time;
#ifdef DEBUG_PRESS
    uint64_t total_time;
    uint64_t average_time;
    int i;
#endif

    tfs_get_ID2(id2, &id2_len);

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_sign((const uint8_t *)in_data, in_len, out_data, &len); //sign first
#ifdef DEBUG_PRESS
    total_time = 0;
    for (i = 0; i < PRESS_TIME; i ++) {
    //LOGI(TAG_TFS_TEST,"verify: %d time.", i + 1);
#endif
    start = pal_get_current_time();
    ret = tfs_id2_verify((const uint8_t *)in_data, in_len, out_data, len); //then verify
    end = pal_get_current_time();
#ifdef DEBUG_PRESS
    total_time += end - start;
    }
    average_time = total_time/PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end -start;
#endif 
    LOGI(TAG_TFS_TEST,"\ntfs_id2_verify: ret = %d!\n\ntime:%lld\n\n", ret, cost_time);
    return 0;
}

static int test_tfs_id2_encrypt(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;
    uint32_t in_len = strlen(in_data);
    uint32_t id2_len = TFS_ID2_LEN + 1;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};
    uint64_t cost_time;
#ifdef DEBUG_PRESS
    uint64_t total_time;
    uint64_t average_time;
    int i;
#endif

    tfs_get_ID2(id2, &id2_len);
    memset(out_data, 0, BUF_MAX);
#ifdef DEBUG_PRESS
    total_time = 0;
    for (i = 0; i < PRESS_TIME; i ++) {
    // LOGI(TAG_TFS_TEST,"encrypt: %d time.", i + 1);
#endif
    start = pal_get_current_time();
    ret = tfs_id2_encrypt((uint8_t *)in_data, in_len, out_data, &len);
    end = pal_get_current_time();
#ifdef DEBUG_PRESS
    total_time += end - start;
    }
    average_time = total_time/PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end -start;
#endif
    LOGI(TAG_TFS_TEST,"\ntfs_id2_encrypt: ret = %d, encrypt out(%d)\n\ntime:%lld\n\n", ret, len, cost_time);
    hexdump(out_data, len);

    return 0;
}

static int test_tfs_id2_decrypt(void)
{
    int ret = 0;
    uint32_t enc_len = BUF_MAX;
    uint32_t dec_len = BUF_MAX;
    uint32_t in_len = strlen(in_data);
    uint32_t id2_len = TFS_ID2_LEN + 1;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};
    uint64_t cost_time;
#ifdef DEBUG_PRESS
    uint64_t total_time;
    uint64_t average_time;
    int i;
#endif

    tfs_get_ID2(id2, &id2_len);
    memset(out_data, 0, BUF_MAX);
    memset(dec_out, 0, BUF_MAX);
    ret = tfs_id2_encrypt((uint8_t *)in_data, in_len, out_data, &enc_len); //encrypt first
#ifdef DEBUG_PRESS
    total_time = 0; 
    for (i = 0; i < PRESS_TIME; i ++) {
    //LOGI(TAG_TFS_TEST,"decrypt: %d time.", i + 1);
#endif
    start = pal_get_current_time();
    ret = tfs_id2_decrypt(out_data, enc_len, dec_out, &dec_len); //then decrypt
    end = pal_get_current_time();
#ifdef DEBUG_PRESS
    total_time += end - start;
    }
    average_time = total_time/PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end -start;
#endif
    LOGI(TAG_TFS_TEST,"\ntfs_id2_decrypt: ret = %d, decrypt out(%d): %s\n\ntime:%lld\n\n", ret, dec_len, dec_out, cost_time);

    return 0;
}

static int test_tfs_get_auth_code(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_get_auth_code(out_data, &len);

    LOGI(TAG_TFS_TEST,"\ntfs_get_auth_code: ret = %d, the auth_code(%d): %s\n\n", ret, len, out_data);
    return 0;
}

static int test_tfs_activate_device(void) {
    int ret = -1;

    ret = tfs_activate_device();

    LOGI(TAG_TFS_TEST,"\ntfs_activate_device: ret = %d.\n\n", ret);
    return 0;
}

static int test_tfs_id2_get_auth_code(void)
{
    int ret = -1;
    int len = BUF_MAX;
    uint64_t timestamp = 0; // in ms

    start = pal_get_current_time();
    timestamp = start / 1000;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_auth_code(timestamp, out_data, &len);
    LOGI(TAG_TFS_TEST,"\ntfs_id2_get_auth_code: ret = %d, the auth_code(%d): %s\n\n", ret, len, out_data);
    return 0;
}

static int test_tfs_id2_get_digest_auth_code(void)
{
    int ret = -1;
    int len = BUF_MAX;
    uint64_t timestamp = 0; // in ms
    uint8_t *digest = "abcd";

    start = pal_get_current_time();
    timestamp = start / 1000;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_digest_auth_code(timestamp, digest, strlen(digest), out_data, &len);
    LOGI(TAG_TFS_TEST,"\ntfs_id2_get_digest_auth_code: ret = %d, the auth_code(%d): %s\n\n", ret, len, out_data);
    return 0;
}

static int test_tfs_aes128_enc_dec(int size, uint8_t padding)
{
    int ret = -1;
    const uint8_t in[64] = "Hello World!1234567890123456789012345678901234567890";
    uint8_t out[128];
    uint8_t iv_enc[16] = {0};
    uint8_t iv_dec[16] = {0};
    const uint8_t key[16] = "Demo-Test";
    uint8_t dec[128] = {0};
    int32_t in_len = size;
    int32_t out_len = 0;
    int32_t dec_len = 0;
    int32_t i = 0;

    LOGI(TAG_TFS_TEST, "source data len %d:\n", in_len);
    hexdump(in, in_len);
    ret = tfs_aes128_cbc_enc(key, iv_enc, in_len, in, &out_len, out, padding);
    if (ret == -1) {
        LOGE(TAG_TFS_TEST,"tfs_aes128_enc error.\n");
        return -1;
    }

    LOGI(TAG_TFS_TEST, "encrypted data len %d:\n", out_len);
    hexdump(out, out_len);

    ret = tfs_aes128_cbc_dec(key, iv_dec, out_len, out, &dec_len, dec, padding);
    if (ret == -1) {
        LOGE(TAG_TFS_TEST,"tfs_aes128_dec error.\n");
        return -1;
    }
    LOGI(TAG_TFS_TEST, "decrypted data len %d:\n", dec_len);
    hexdump(dec, dec_len);

    if (in_len > dec_len || (in_len < dec_len && padding != TFS_AES_ZERO_PADDING)) {
        LOGE(TAG_TFS_TEST, "decrypted data len error.\n");
        return -1;
    }

    for (i = 0;i < in_len ; i ++) {
        if (in[i] != dec[i]) {
            break;
        }
    }
    if (i < in_len) {
        LOGE(TAG_TFS_TEST, "decrypted data is not equal to in data.\n");
        return -1;
    }

    if (in_len < dec_len) {
    // for zero padding
        for (i = in_len;i < dec_len; i ++) {
            if (dec[i] != 0) {
                break;
            }
        } 
        if (i < dec_len) {
            LOGE(TAG_TFS_TEST, "decrypted data padding error when zero padding.\n");
            return -1;
        }
    }

    LOGI(TAG_TFS_TEST, "aes encryption and decryption ok!\n");
    return 0;
}

static int test_tfs_aes128_enc_dec_performance(int size, uint8_t padding)
{
    int ret = -1;
    uint8_t *in;
    uint8_t *out;
    uint8_t iv_enc[16] = {0};
    uint8_t iv_dec[16] = {0};
    const uint8_t key[16] = "Demo-Test";
    uint8_t *dec;
    uint64_t cost_time;
    int i;
    int32_t in_len;
    int32_t out_len;
    int32_t dec_len;
#ifdef DEBUG_PRESS
    uint64_t total_time;
    uint64_t average_time;
#endif

    in = (uint8_t *)pal_memory_malloc(size);
    out = (uint8_t *)pal_memory_malloc(size + 16);
    dec = (uint8_t *)pal_memory_malloc(size + 16);

    for (i = 0 ; i < size;i ++) {
        *(in + i) = i%10 + '0';
    }

    in_len = size;
#ifdef DEBUG_PRESS
    total_time = 0; 
    for (i = 0; i < PRESS_TIME; i ++) {
#endif
    start = pal_get_current_time();
    ret = tfs_aes128_cbc_enc(key, iv_enc, in_len, in, &out_len, out, padding);
    if (ret == -1) {
        LOGE(TAG_TFS_TEST,"tfs_aes128_cbc_enc error.\n");
        return -1;
    }
    end = pal_get_current_time();
#ifdef DEBUG_PRESS
    total_time += end - start;
    }
    average_time = total_time/PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end -start;
#endif

    LOGI(TAG_TFS_TEST,"\ntfs_aes128_cbc_enc: ret = %d, time: %lld\n\n", ret, cost_time);

#ifdef DEBUG_PRESS
    total_time = 0; 
    for (i = 0; i < PRESS_TIME; i ++) {
#endif
    start = pal_get_current_time();
    ret = tfs_aes128_cbc_dec(key, iv_dec, out_len, out, &dec_len, dec, padding);
    if (ret == -1) {
        LOGE(TAG_TFS_TEST,"tfs_aes128_cbc_dec error.\n");
        return -1;
    }
    end = pal_get_current_time();
#ifdef DEBUG_PRESS
    total_time += end - start;
    }
    average_time = total_time/PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end -start;
#endif

    LOGI(TAG_TFS_TEST,"\ntfs_aes128_cbc_dec: ret = %d, time: %lld\n\n", ret, cost_time);

    pal_memory_free(in);
    pal_memory_free(out);
    pal_memory_free(dec);

    return ret;
}
