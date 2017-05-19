/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <stdint.h>
#include <string.h>
#include "keys_rsa.h"
#include "log.h"
#include "pal.h"

#if defined(TFS_OPENSSL)
#include "openssl/bn.h"
#include "openssl/pem.h"
#include "openssl/rsa.h"
#elif defined(TFS_MBEDTLS)
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/rsa.h>
#endif // TFS_OPENSSL

#define MD5_MAX 16

// sign type
#define EMU_TYPE_MD5    0
#define EMU_TYPE_SHA1   1
#define EMU_TYPE_SHA256 2

// PADDING TYPE
#define EMU_PKCS1_PADDING 0X01 // default
#define EMU_NO_PADDING    0X02

#define TAG_EMU_RSA "TFS_EMU_RSA"

int emu_RSA_sign(uint8_t ID, const uint8_t *in, uint32_t in_len,
                 uint8_t *sign, uint32_t *sign_len, uint8_t type)
{
    int ret = 0;
#if defined(TFS_OPENSSL)
    uint8_t md5[MD5_MAX];
    uint8_t sign_type;

    if (type != EMU_TYPE_MD5 && type != EMU_TYPE_SHA1 && type != EMU_TYPE_SHA256) {
        LOGE(TAG_EMU_RSA, "\n%s: not support sign type!\n", __FUNCTION__);
        return -1;
    }

    LOGD(TAG_EMU_RSA, "\n%s: for id : %d.\n", __FUNCTION__, ID);
    if (type == EMU_TYPE_MD5) {
        sign_type = NID_md5;
    } else {
        LOGE(TAG_EMU_RSA, "\n%s: to do support...\n", __FUNCTION__);
        return -1;
    }

    RSA *key = RSA_new();
    BN_hex2bn(&key->n, RSA_N);
    BN_hex2bn(&key->e, RSA_E);
    BN_hex2bn(&key->d, RSA_D);

    pal_md5_sum(in, in_len, md5);

    ret = RSA_sign(sign_type, md5, MD5_MAX, sign, sign_len, key);
    if (ret != 1) {
        LOGE(TAG_EMU_RSA, "\n%s: sign error , ret = %d\n", __FUNCTION__, ret);
        return -1;
    }

    ret = 0;
    LOGD(TAG_EMU_RSA, "sign_len:%d, ret:%d\n", *sign_len, ret);
    RSA_free(key);

    LOGD(TAG_EMU_RSA, "%s: OK!\n", __FUNCTION__);
    return ret;
#elif defined(TFS_MBEDTLS)
    uint8_t hash[16];
    mbedtls_rsa_context rsa;

    LOGD(TAG_EMU_RSA, "\n%s: for id : %d.\n", __FUNCTION__, ID);

    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    if ((ret = mbedtls_mpi_read_string(&rsa.N, 16, RSA_N)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.E, 16, RSA_E)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.D, 16, RSA_D)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.P, 16, RSA_P)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.Q, 16, RSA_Q)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.DP, 16, RSA_DP)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.DQ, 16, RSA_DQ)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.QP, 16, RSA_QP)) != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls rsa init error!\n", __FUNCTION__);
        return -1;
    }
    rsa.len = (mbedtls_mpi_bitlen(&rsa.N) + 7) >> 3;

    if (mbedtls_rsa_check_pubkey(&rsa) != 0 ||
        mbedtls_rsa_check_privkey(&rsa) != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls rsa check error!\n", __FUNCTION__);
        return -1;
    }

    pal_md5_sum(in, in_len, hash);
    ret = mbedtls_rsa_pkcs1_sign(&rsa, NULL, NULL, MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_MD5, 16, hash, sign);
    if (ret != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls sign error!\n", __FUNCTION__);
        return -1;
    }

    *sign_len = rsa.len;
    mbedtls_rsa_free(&rsa);

    LOGD(TAG_EMU_RSA, "%s: OK!\n", __FUNCTION__);
    return ret;
#else
    LOGD(TAG_EMU_RSA, "%s: no implement!\n", __FUNCTION__);
    return -1;
#endif
}

int emu_RSA_verify(uint8_t ID, const uint8_t *in, uint32_t in_len,
                   uint8_t *sign, uint32_t sign_len, uint8_t type)
{
    int ret = 0;
#if defined(TFS_OPENSSL)
    uint8_t md5[MD5_MAX];
    uint8_t sign_type;

    if (type != EMU_TYPE_MD5 && type != EMU_TYPE_SHA1 && type != EMU_TYPE_SHA256) {
        LOGE(TAG_EMU_RSA, "\n%s: not support sign type!\n", __FUNCTION__);
        return -1;
    }

    LOGD(TAG_EMU_RSA, "\n%s: for id : %d.\n", __FUNCTION__, ID);

    if (type == EMU_TYPE_MD5) {
        sign_type = NID_md5;
    } else {
        LOGE(TAG_EMU_RSA, "\n%s: to do support...\n", __FUNCTION__);
        return -1;
    }

    RSA *key = RSA_new();
    BN_hex2bn(&key->n, RSA_N);
    BN_hex2bn(&key->e, RSA_E);
    BN_hex2bn(&key->d, RSA_D);

    pal_md5_sum(in, in_len, md5);

    ret = RSA_verify(sign_type, md5, MD5_MAX, sign, sign_len, key);
    if (ret != 1) {
        LOGE(TAG_EMU_RSA, "\n%s: verify error , ret = %d\n", __FUNCTION__, ret);
        return -1;
    }
    ret = 0;
    LOGD(TAG_EMU_RSA, "verified ret:%d\n", ret);
    RSA_free(key);

    LOGD(TAG_EMU_RSA, "%s: OK!\n", __FUNCTION__);
    return ret;
#elif defined(TFS_MBEDTLS)
    uint8_t hash[16];
    mbedtls_rsa_context rsa;

    LOGD(TAG_EMU_RSA, "\n%s: for id : %d.\n", __FUNCTION__, ID);

    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    if ((ret = mbedtls_mpi_read_string(&rsa.N, 16, RSA_N)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.E, 16, RSA_E)) != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls rsa init error!\n", __FUNCTION__);
        return -1;
    }
    rsa.len = (mbedtls_mpi_bitlen(&rsa.N) + 7) >> 3;

    pal_md5_sum(in, in_len, hash);
    ret = mbedtls_rsa_pkcs1_verify(&rsa, NULL, NULL, MBEDTLS_RSA_PUBLIC, MBEDTLS_MD_MD5, 16, hash, sign);
    if (ret != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls verify error!\n", __FUNCTION__);
        return -1;
    }

    mbedtls_rsa_free(&rsa);

    LOGD(TAG_EMU_RSA, "%s: OK!\n", __FUNCTION__);
    return ret;
#else
    LOGD(TAG_EMU_RSA, "%s: no implement!\n", __FUNCTION__);
    return -1;
#endif
}
int emu_RSA_public_encrypt(uint8_t ID, const uint8_t *in, uint32_t in_len,
                           uint8_t *out, uint32_t *out_len, uint8_t padding)
{
    int ret = 0;
#if defined(TFS_OPENSSL)
    uint8_t padding_type;

    if (padding != EMU_PKCS1_PADDING && padding != EMU_NO_PADDING) {
        LOGE(TAG_EMU_RSA, "\n%s: not supported padding type.\n", __FUNCTION__);
        return -1;
    }
    LOGD(TAG_EMU_RSA, "\n%s: for id : %d.\n", __FUNCTION__, ID);

    if (padding == EMU_PKCS1_PADDING) {
        padding_type = RSA_PKCS1_PADDING;
    } else {
        LOGE(TAG_EMU_RSA, "\n%s: to do support.\n", __FUNCTION__);
        return -1;
    }

    RSA *key = RSA_new();

    BN_hex2bn(&key->n, RSA_N);
    BN_hex2bn(&key->e, RSA_E);
    BN_hex2bn(&key->d, RSA_D);

    memset(out, 0, 256);
    // public key encrypt
    ret = RSA_public_encrypt(in_len, in, out, key, padding_type);
    if (ret < 0) {
        LOGE(TAG_EMU_RSA, "\n%s: encrypt error , ret = %d\n", __FUNCTION__, ret);
        return -1;
    }
    *out_len = ret;
    ret = 0;
    //hexdump(out, ret);
    LOGD(TAG_EMU_RSA, "encrypted len:%d, ret:%d\n", *out_len, ret);

    RSA_free(key);

    LOGD(TAG_EMU_RSA, "%s: OK!\n", __FUNCTION__);
    return ret;
#elif defined(TFS_MBEDTLS)
    mbedtls_rsa_context rsa;
    /*
        const uint8_t *pers = "rsa_encrypt";
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;

        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_entropy_init(&entropy);
        ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, pers, strlen(pers));
        if (ret != 0) {
            LOGE(TAG_EMU_RSA,  "\n%s: mbedtls_ctr_drbg_seed error!\n", __FUNCTION__);
            mbedtls_ctr_drbg_DEBUG_FREE(&ctr_drbg);
            mbedtls_entropy_DEBUG_FREE(&entropy);
            return -1;
        }
    */
    LOGD(TAG_EMU_RSA, "\n%s: for id : %d.\n", __FUNCTION__, ID);

    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    if ((ret = mbedtls_mpi_read_string(&rsa.N, 16, RSA_N)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.E, 16, RSA_E)) != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls rsa init error!\n", __FUNCTION__);
        return -1;
    }
    rsa.len = (mbedtls_mpi_bitlen(&rsa.N) + 7) >> 3;

    ret = mbedtls_rsa_pkcs1_encrypt(&rsa, NULL, NULL,
                                    MBEDTLS_RSA_PUBLIC, in_len, in, out);
    if (ret != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls priv-encrypt error, ret = %d\n", __FUNCTION__, ret);
//        mbedtls_ctr_drbg_DEBUG_FREE(&ctr_drbg);
//        mbedtls_entropy_DEBUG_FREE(&entropy);
        return -1;
    }

//    mbedtls_ctr_drbg_DEBUG_FREE( &ctr_drbg );
//    mbedtls_entropy_DEBUG_FREE( &entropy );

    *out_len = rsa.len;

    LOGD(TAG_EMU_RSA, "%s: OK!\n", __FUNCTION__);
    return ret;
#else
    LOGD(TAG_EMU_RSA, "%s: no implement!\n", __FUNCTION__);
    return -1;
#endif
}

int emu_RSA_private_decrypt(uint8_t ID, uint8_t *in, uint32_t in_len,
                            uint8_t *out, uint32_t *out_len, uint8_t padding)
{
    int ret = 0;
#if defined(TFS_OPENSSL)
    uint8_t padding_type;

    if (padding != EMU_PKCS1_PADDING && padding != EMU_NO_PADDING) {
        LOGE(TAG_EMU_RSA, "\n%s: not supported padding type.\n", __FUNCTION__);
        return -1;
    }

    LOGD(TAG_EMU_RSA, "\n%s: for id : %d.\n", __FUNCTION__, ID);
    if (padding == EMU_PKCS1_PADDING) {
        padding_type = RSA_PKCS1_PADDING;
    } else {
        LOGE(TAG_EMU_RSA, "\n%s: to do support.\n", __FUNCTION__);
        return -1;
    }

    RSA *key = RSA_new();

    BN_hex2bn(&key->n, RSA_N);
    BN_hex2bn(&key->e, RSA_E);
    BN_hex2bn(&key->d, RSA_D);

    // private key decrypt
    ret = RSA_private_decrypt(in_len, in, out, key, padding_type);
    if (ret < 0) {
        *out_len = 0;
        LOGE(TAG_EMU_RSA, "\n%s: decrypt error , ret = %d\n", __FUNCTION__, ret);
        return -1;
    }
    *out_len = ret;
    ret = 0;
    LOGD(TAG_EMU_RSA, "decrypted len:%d, ret:%d ", *out_len, ret);

    RSA_free(key);

    LOGD(TAG_EMU_RSA, "%s: OK!\n", __FUNCTION__);
    return ret;
#elif defined(TFS_MBEDTLS)
    mbedtls_rsa_context rsa;
    /*
        const uint8_t *pers = "rsa_decrypt";
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;

        mbedtls_ctr_drbg_init(&ctr_drbg);
        mbedtls_entropy_init(&entropy);
        ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, pers, strlen(pers));
        if (ret != 0) {
            LOGE(TAG_EMU_RSA,  "\n%s: mbedtls_ctr_drbg_seed error!\n", __FUNCTION__);
            mbedtls_ctr_drbg_DEBUG_FREE(&ctr_drbg);
            mbedtls_entropy_DEBUG_FREE(&entropy);
            return -1;
        }
    */

    LOGD(TAG_EMU_RSA, "\n%s: for id : %d.\n", __FUNCTION__, ID);

    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    if ((ret = mbedtls_mpi_read_string(&rsa.N, 16, RSA_N)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.E, 16, RSA_E)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.D, 16, RSA_D)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.P, 16, RSA_P)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.Q, 16, RSA_Q)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.DP, 16, RSA_DP)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.DQ, 16, RSA_DQ)) != 0 ||
        (ret = mbedtls_mpi_read_string(&rsa.QP, 16, RSA_QP)) != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls rsa init error!\n", __FUNCTION__);
        return -1;
    }
    rsa.len = (mbedtls_mpi_bitlen(&rsa.N) + 7) >> 3;

    if (mbedtls_rsa_check_pubkey(&rsa) != 0 ||
        mbedtls_rsa_check_privkey(&rsa) != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls rsa check error!\n", __FUNCTION__);
        return -1;
    }

    ret = mbedtls_rsa_pkcs1_decrypt(&rsa, NULL, NULL,
                                    MBEDTLS_RSA_PRIVATE, (size_t *)out_len, in, out, 1024);
    if (ret != 0) {
        LOGE(TAG_EMU_RSA, "\n%s: mbedtls priv-decrypt error, ret = %d\n", __FUNCTION__, ret);
//        mbedtls_ctr_drbg_DEBUG_FREE(&ctr_drbg);
//        mbedtls_entropy_DEBUG_FREE(&entropy);
        return -1;
    }

//    mbedtls_ctr_drbg_DEBUG_FREE( &ctr_drbg );
//    mbedtls_entropy_DEBUG_FREE( &entropy );

    LOGD(TAG_EMU_RSA, "%s: OK!\n", __FUNCTION__);
    return ret;
#else
    LOGD(TAG_EMU_RSA, "%s: no implement!\n", __FUNCTION__);
    return -1;
#endif
}
