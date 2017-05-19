/**
 * Copyright (C) 2017 The YunOS Project. All rights reserved.
 */

#include "ali_crypto_test.h"

int ali_crypto_rand_test(void)
{
    uint32_t i = 0;
    uint8_t seed[16];
    size_t seed_len = 16;
    uint8_t tmp_buf[32];
    uint8_t rand_buf[32];
    size_t rand_len = 32;
    ali_crypto_result result;

    result = ali_seed(seed, seed_len);
    if (result != ALI_CRYPTO_SUCCESS) {
        CRYPT_INF("ali_seed fail(%08x)\n", result);
        return -1;
    }

    result = ali_rand_gen(rand_buf, rand_len);
    if (result != ALI_CRYPTO_SUCCESS) {
        CRYPT_INF("gen rand fail(%08x)\n", result);
        return -1;
    }

    while(i++ < 10) {
        CRYPT_MEMCPY(tmp_buf, rand_buf, rand_len);

        result = ali_rand_gen(rand_buf, rand_len);
        if (result != ALI_CRYPTO_SUCCESS) {
            CRYPT_INF("gen rand fail(%08x)\n", result);
            return -1;
        }

        if (!CRYPT_MEMCMP(tmp_buf, rand_buf, rand_len)) {
            CRYPT_INF("RAND test fail!\n");
            ali_crypto_print_data("tmp_buf", tmp_buf, rand_len);
            ali_crypto_print_data("rand_buf", rand_buf, rand_len);
            return -1;
        }
    }

    CRYPT_INF("RAND test success!\n");

    return 0;
}
