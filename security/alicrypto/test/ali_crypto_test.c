/**
 * Copyright (C) 2016 The YunOS Project. All rights reserved.
 */

#include "ali_crypto.h"
#include "ali_crypto_test.h"

void ali_crypto_test_entry(void *arg)
{
    int ret;

    CRYPT_INF("Test hash:\n");
    ret = ali_crypto_hash_test();
    if (ret < 0) {
        return;
    }

#if 0
    CRYPT_INF("Test hmac:\n");
    ret = tee_crypto_hmac_test();
    if (ret < 0) {
        return;
    }
#endif

    CRYPT_INF("Test rand:\n");
    ret = ali_crypto_rand_test();
    if (ret < 0) {
        return;
    }

    CRYPT_INF("Test aes:\n");
    ret = ali_crypto_aes_test();
    if (ret < 0) {
        return;
    }

    CRYPT_INF("Test rsa:\n");
    ret = ali_crypto_rsa_test();
    if (ret < 0) {
        return;
    }

    return;
}

#if 0
int main(void)
{
    ali_crypto_test_entry(NULL);

    return 0;
}
#endif
