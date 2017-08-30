#ifndef _ALI_CRYPTO_TEST_H_
#define _ALI_CRYPTO_TEST_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ali_crypto.h"

#define CRYPT_ERR(_f, _a ...)  printf(_f, ##_a)
#define CRYPT_INF(_f, _a ...)  printf(_f, ##_a)

#define CRYPT_MALLOC           malloc
#define CRYPT_FREE             free
#define CRYPT_MEMSET           memset
#define CRYPT_MEMCPY           memcpy
#define CRYPT_MEMCMP           memcmp

#define PRINT_RET(_ret, _f, _a ...) do {            \
    CRYPT_ERR(_f, ##_a);                            \
    return _ret;                                    \
} while (0);

#define GO_RET(_ret, _f, _a ...) do {               \
    CRYPT_ERR(_f, ##_a);                            \
    result = _ret;                                  \
    goto _OUT;                                      \
} while (0);

void ali_crypto_print_data(const char *name, uint8_t *data, size_t size);

int ali_crypto_hash_test(void);
int ali_crypto_hmac_test(void);
int ali_crypto_rand_test(void);
int ali_crypto_aes_test(void);
int ali_crypto_rsa_test(void);
#endif
