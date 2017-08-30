/****************************************************************************
 * Included Files
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "pal.h"
#include "tfs_id2.h"
#include "tfs_aes.h"
#include "log.h"

#ifndef TFS_ONLINE
#include "tfs_test_decrypt.h"
#endif

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
static void prepare_test_data();
static int test_tfs_get_ID2(void);
static int test_tfs_id2_sign(void);
static int test_tfs_id2_verify(void);
static int test_tfs_id2_encrypt(void);
static int test_tfs_id2_decrypt(void);
#ifndef TFS_ONLINE
static int test_tfs_id2_decrypt_daily(void);
#endif
static int test_tfs_activate_device(void);
static int test_tfs_get_auth_code(void);
static int test_tfs_id2_get_auth_code(void);
static int test_tfs_id2_get_digest_auth_code(void);

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

//#define CREATE_ID2_SIGN
#ifdef CREATE_ID2_SIGN
static int create_id2_sign(void);
#endif

#define TAG_TFS_TEST "TFS_TEST"

void tfs_test()
{

    prepare_test_data();
    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_get_ID2 <<<<<<\n");
    test_tfs_get_ID2();

    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_id2_sign <<<<<<\n");
    test_tfs_id2_sign();

    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_id2_verify <<<<<<\n");
    test_tfs_id2_verify();

    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_id2_encrypt <<<<<<\n");
    test_tfs_id2_encrypt();

    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_id2_decrypt <<<<<<\n");
    test_tfs_id2_decrypt();

#ifdef CREATE_ID2_SIGN
    LOGI(TAG_TFS_TEST, ">>>>>>func: create_id2_sign <<<<<<\n");
    create_id2_sign();
#endif

#ifndef TFS_ONLINE
    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_id2_decrypt_daily <<<<<<\n");
    test_tfs_id2_decrypt_daily();
#endif

    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_activate_device <<<<<<\n");
    test_tfs_activate_device();

    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_get_auth_code <<<<<<\n");
    test_tfs_get_auth_code();

    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_id2_get_auth_code <<<<<<\n");
    test_tfs_id2_get_auth_code();

    LOGI(TAG_TFS_TEST, ">>>>>>func: test_tfs_id2_get_digest_auth_code <<<<<<\n");
    test_tfs_id2_get_digest_auth_code();
}

static void prepare_test_data()
{
    int i = 0;
    for (i = 0; i < IN_DATA_SIZE; i ++) {
        in_data[i] = i % 10 + '0';
    }
    in_data[IN_DATA_SIZE] = '\0';
}

static inline void hexdump(const uint8_t *str, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++) {
        LOG("%02X,", *str++);
        if ((i + 1) % 32 == 0) {
            LOG("\n");
        }
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

    LOGI(TAG_TFS_TEST, "tfs_get_ID2: ret = %d, the ID2(%d): %s\n\ntime: %lld\n\n",
         ret, len, id2, cost_time);
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
        //LOGI(TAG_TFS_TEST,"sign: %d time.\n", i + 1);
#endif
        start = pal_get_current_time();
        ret = tfs_id2_sign((const uint8_t *)in_data, strlen(in_data), out_data, &len);
        end = pal_get_current_time();
#ifdef DEBUG_PRESS
        total_time += end - start;
    }
    average_time = total_time / PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end - start;
#endif
    LOGI(TAG_TFS_TEST, "tfs_id2_sign: ret = %d, sign out(%d) \n\ntime:%lld\n\n",
         ret, len, cost_time);
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
    ret = tfs_id2_sign((const uint8_t *)in_data, in_len, out_data,
                       &len); //sign first
#ifdef DEBUG_PRESS
    total_time = 0;
    for (i = 0; i < PRESS_TIME; i ++) {
        //LOGI(TAG_TFS_TEST,"verify: %d time.\n", i + 1);
#endif
        start = pal_get_current_time();
        ret = tfs_id2_verify((const uint8_t *)in_data, in_len, out_data,
                             len); //then verify
        end = pal_get_current_time();
#ifdef DEBUG_PRESS
        total_time += end - start;
    }
    average_time = total_time / PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end - start;
#endif
    LOGI(TAG_TFS_TEST, "tfs_id2_verify: ret = %d!\n\ntime:%lld\n\n", ret,
         cost_time);
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
        // LOGI(TAG_TFS_TEST,"encrypt: %d time.\n", i + 1);
#endif
        start = pal_get_current_time();
        ret = tfs_id2_encrypt((uint8_t *)in_data, in_len, out_data, &len);
        end = pal_get_current_time();
#ifdef DEBUG_PRESS
        total_time += end - start;
    }
    average_time = total_time / PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end - start;
#endif
    LOGI(TAG_TFS_TEST,
         "tfs_id2_encrypt: ret = %d, encrypt out(%d)\n\ntime:%lld\n\n", ret, len,
         cost_time);
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
    ret = tfs_id2_encrypt((uint8_t *)in_data, in_len, out_data,
                          &enc_len); //encrypt first
#ifdef DEBUG_PRESS
    total_time = 0;
    for (i = 0; i < PRESS_TIME; i ++) {
        //LOGI(TAG_TFS_TEST,"decrypt: %d time.\n", i + 1);
#endif
        start = pal_get_current_time();
        ret = tfs_id2_decrypt(out_data, enc_len, dec_out, &dec_len); //then decrypt
        end = pal_get_current_time();
#ifdef DEBUG_PRESS
        total_time += end - start;
    }
    average_time = total_time / PRESS_TIME;
#endif
#ifdef DEBUG_PRESS
    cost_time = average_time;
#else
    cost_time = end - start;
#endif
    LOGI(TAG_TFS_TEST,
         "tfs_id2_decrypt: ret = %d, decrypt out(%d): %s\n\ntime:%lld\n\n", ret, dec_len,
         dec_out, cost_time);

    return 0;
}

#ifdef CREATE_ID2_SIGN
static int create_id2_sign(void)
{
    int ret = 0;
    uint32_t id2_len = TFS_ID2_LEN + 1;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};
    uint8_t id2_sign[BUF_MAX];
    uint32_t id2_sign_len = 0;
    uint8_t id2_sign_base64[BUF_MAX];
    uint32_t id2_sign_base64_len;
    const char *encrypted_data_base64 = "JKSX1+AIE7EE1VWP5sQRlw==";
    uint8_t encrypted_data[BUF_MAX];
    uint32_t encrypted_data_len;
    uint32_t i = 0;

    ret = tfs_get_ID2(id2, &id2_len);
    if (ret != 0) {
        LOGE(TAG_TFS_TEST, "tfs_get_id2 error.\n");
        return -1;
    }

    memset(id2_sign, 0, BUF_MAX);
    memset(id2_sign_base64, 0, BUF_MAX);
    ret = tfs_id2_sign(id2, id2_len, id2_sign, &id2_sign_len);
    if (ret != 0) {
        LOGE(TAG_TFS_TEST, "sign id2 error.\n");
        return -1;
    }

    for (i = 0; i < id2_sign_len; i ++) {
        LOG("%X", id2_sign[i]);
    }

    LOGI(TAG_TFS_TEST, "id2 is %s.\n", id2);

    pal_base64_encode(id2_sign, id2_sign_len, id2_sign_base64,
                      (int *)&id2_sign_base64_len);
    LOGI(TAG_TFS_TEST, "id2_sign_base64 is %s.\n", id2_sign_base64);

    pal_base64_decode((const unsigned char *)encrypted_data_base64,
                      strlen(encrypted_data_base64), encrypted_data, (int *)&encrypted_data_len);
    LOGI(TAG_TFS_TEST, "encrypted_data_base64 is %s.\n", encrypted_data_base64);

    for (i = 0; i < encrypted_data_len; i ++) {
        if (i != 0 && i % 8 == 0) {
            LOG("\n");
        }
        LOG("0x%02X,", encrypted_data[i]);
    }

    return 0;
}
#endif

#ifndef TFS_ONLINE
int test_tfs_id2_decrypt_daily(void)
{
    int ret = 0;
    uint32_t id2_len = TFS_ID2_LEN + 1;
    uint8_t id2[TFS_ID2_LEN + 1] = {0};
    uint32_t dec_len = BUF_MAX;
    int test_id2_list_len = sizeof(tfs_test_id2) / (TFS_ID2_LEN + 1);
    int index = 0;

    ret = tfs_get_ID2(id2, &id2_len);
    if (ret != 0) {
        LOGE(TAG_TFS_TEST, "tfs_get_id2 error.\n");
        return -1;
    }

    LOGI(TAG_TFS_TEST, "id2 is %s.\n", id2);

    LOGI(TAG_TFS_TEST, "id2 list len: %d.\n", test_id2_list_len);
    for (index = 0; index < test_id2_list_len; index ++) {
        if (memcmp(id2, tfs_test_id2[index], TFS_ID2_LEN) == 0) {
            break;
        }
    }

    if (index > test_id2_list_len) {
        LOGE(TAG_TFS_TEST, "no test id2 data.\n");
        return -1;
    }

    LOGI(TAG_TFS_TEST, "index is %d.\n", index);
    memset(dec_out, 0, BUF_MAX);
    ret = tfs_id2_decrypt(tfs_test_cipher_text[index],
                          sizeof(tfs_test_cipher_text[index]), dec_out, &dec_len); //then decrypt

    LOGI(TAG_TFS_TEST, "tfs_id2_decrypt: ret = %d, decrypt out(%d): %s\n\n", ret,
         dec_len, dec_out);

    return 0;
}
#endif

static int test_tfs_get_auth_code(void)
{
    int ret = 0;
    uint32_t len = BUF_MAX;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_get_auth_code(out_data, &len);

    LOGI(TAG_TFS_TEST, "tfs_get_auth_code: ret = %d, the auth_code(%d): %s\n\n",
         ret, len, out_data);
    return 0;
}

static int test_tfs_activate_device(void)
{
    int ret = -1;

    ret = tfs_is_device_activated();

    LOGI(TAG_TFS_TEST, "tfs_is_device_activated: ret = %d.\n\n", ret);

    ret = tfs_activate_device();

    LOGI(TAG_TFS_TEST, "tfs_activate_device: ret = %d.\n\n", ret);

    ret = tfs_is_device_activated();

    LOGI(TAG_TFS_TEST, "tfs_is_device_activated: ret = %d.\n\n", ret);
    return 0;
}

static int test_tfs_id2_get_auth_code(void)
{
    int ret = -1;
    uint32_t len = BUF_MAX;
    uint64_t timestamp = 0; // in ms

    start = pal_get_current_time();
    timestamp = start / 1000;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_auth_code(timestamp, out_data, &len);
    LOGI(TAG_TFS_TEST, "tfs_id2_get_auth_code: ret = %d, the auth_code(%d): %s\n\n",
         ret, len, out_data);
    return 0;
}

static int test_tfs_id2_get_digest_auth_code(void)
{
    int ret = -1;
    uint32_t len = BUF_MAX;
    uint64_t timestamp = 0; // in ms
    char *digest = "abcd";

    start = pal_get_current_time();
    timestamp = start / 1000;

    memset(out_data, 0, BUF_MAX);
    ret = tfs_id2_get_digest_auth_code(timestamp, (uint8_t *)digest, strlen(digest),
                                       out_data, &len);
    LOGI(TAG_TFS_TEST,
         "tfs_id2_get_digest_auth_code: ret = %d, the auth_code(%d): %s\n\n", ret, len,
         out_data);
    return 0;
}

