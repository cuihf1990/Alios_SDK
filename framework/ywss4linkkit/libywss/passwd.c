/*
 * Copyright (c) 2017-2018 Alibaba Group. All rights reserved.
 *
 * Alibaba Group retains all right, title and interest (including all
 * intellectual property rights) in and to this computer program, which is
 * protected by applicable intellectual property laws.  Unless you have
 * obtained a separate written license from Alibaba Group., you are not
 * authorized to utilize all or a part of this computer program for any
 * purpose (including reproduction, distribution, modification, and
 * compilation into object code), and you must immediately destroy or
 * return to Alibaba Group all copies of this computer program.  If you
 * are licensed by Alibaba Group, your rights to utilize this computer
 * program are limited by the terms of that license.  To obtain a license,
 * please contact Alibaba Group.
 *
 * This computer program contains trade secrets owned by Alibaba Group.
 * and, unless unauthorized by Alibaba Group in writing, you agree to
 * maintain the confidentiality of this computer program and related
 * information and to not disclose this computer program and related
 * information to any other person or entity.
 *
 * THIS COMPUTER PROGRAM IS PROVIDED AS IS WITHOUT ANY WARRANTIES, AND
 * Alibaba Group EXPRESSLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
 * INCLUDING THE WARRANTIES OF MERCHANTIBILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE, AND NONINFRINGEMENT.
 */

#include <stdlib.h>
#include <time.h>

#include "utils_hmac.h"
#include "passwd.h"
#include "sha256.h"
#include "awss_main.h"
#include "utils.h"
#include "awss_wifimgr.h"
#include "zconfig_utils.h"

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
extern "C"
{
#endif

unsigned char aes_random[RANDOM_MAX_LEN] = {0};

const char *cal_passwd(void *key, void *random, void *passwd)
{
    uint16_t key_len;
    uint8_t digest[SHA256_DIGEST_SIZE + 1] = {0};
    uint8_t passwd_src[KEY_MAX_LEN + RANDOM_MAX_LEN + 2] = {0};

    if (!passwd || !key || !random)
        return NULL;

    // combine key and random, with split of comma
    key_len = strlen(key);
    if (key_len > KEY_MAX_LEN)
        key_len = KEY_MAX_LEN;
    memcpy(passwd_src, key, key_len);
    passwd_src[key_len ++] = ',';
    memcpy(passwd_src + key_len, random, RANDOM_MAX_LEN);
    key_len += RANDOM_MAX_LEN;

    // produce digest using combination of key and random
    SHA256_hash(passwd_src, key_len, digest);

    // use the first 128bits as AES-Key
    memcpy(passwd, digest, AES128_KEY_LEN);

    return passwd;
}

void produce_random(unsigned char *random, unsigned int len)
{
    int i = 0;
    long long seed = aos_now();
    srand((unsigned int)seed);
    for (i = 0; i < len; i ++) {
        random[i] = rand() & 0xFF;
    }
}

extern void utils_hmac_sha1_hex(const char *msg, int msg_len, char *digest, const char *key, int key_len);

int produce_signature(unsigned char *sign, unsigned char *txt, unsigned int txt_len)
{
    if (sign == NULL || txt == NULL || txt_len == 0)
        return -1;

    char ps[OS_PRODUCT_SECRET_LEN + 1] = {0};
    os_get_device_secret(ps);
    utils_hmac_sha1_hex((const char *)txt, (int)txt_len,
                    (char *)sign, (const char *)ps, strlen(ps));

    return 0;
}

int aes_decrypt_string(char *cipher, char *plain, int len, int sec_lvl, char cbc)
{
    char res = 0;
    char decrypt = 1;
    uint8_t iv[AES128_KEY_LEN] = {0};
    uint8_t key[AES128_KEY_LEN] = {0};
    uint8_t random[RANDOM_MAX_LEN] = {0};

    uint8_t *decoded = os_zalloc(len + 1);
    if (decoded == NULL)
        return -1;

    if (cbc == 0) {  // for smartconfig
        memset(random, 0, sizeof(random));
        memcpy(decoded, cipher, len);
    } else {  // for mobile-ap, router, zconfig
        if (cbc == 2)  // zconfig
            memcpy(decoded, cipher, len);
        else  // mobile-ap, router
            utils_str_to_hex(cipher, len, decoded, len);
        memcpy(random, aes_random, sizeof(random));
    }

    awss_debug("security level: %d", sec_lvl);

    switch (sec_lvl) {
        case SEC_LVL_AES128_PRODUCT:
        {
            char product_sec[OS_PRODUCT_SECRET_LEN + 1] = {0};
            os_product_get_secret(product_sec);
            cal_passwd(product_sec, random, key);
            memcpy(iv, random, sizeof(random));
            break;
        }
        case SEC_LVL_AES128_DEVICE:
        {
            char dev_sec[OS_DEVICE_SECRET_LEN + 1] = {0};
            os_get_device_secret(dev_sec);
            cal_passwd(dev_sec, random, key);
            memcpy(iv, random, sizeof(random));
            break;
        }
        case SEC_LVL_AES128_MANU:
        {
            char manu_sec[OS_MANU_SECRET_LEN + 1] = {0};
            os_get_manufacture_secret(manu_sec);
            cal_passwd(manu_sec, random, key);
            memcpy(iv, random, sizeof(random));
            break;
        }
        default:
        {
            decrypt = 0;
            awss_debug("wrong security level: %d\n", sec_lvl);
            res = -2;
            break;
        }
    }

    plain[0] = '\0';

    if (decrypt) {
        p_aes128_t aes = os_aes128_init(key, iv, PLATFORM_AES_DECRYPTION);
        if (cbc == 1) { // AP
            os_aes128_cbc_decrypt(aes, decoded, len / AES128_KEY_LEN / 2, plain);
        } else {  // smartconfig
            os_aes128_cfb_decrypt(aes, decoded, len, plain);
        }
        os_aes128_destroy(aes);
    }

    awss_debug("descrypted '%s'\n", plain);

    os_free(decoded);

    return res;
}

#if defined(__cplusplus)  /* If this is a C++ compiler, use C linkage */
}
#endif
