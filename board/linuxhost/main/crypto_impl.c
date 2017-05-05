#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crypto_impl.h"

#if defined(CONFIG_PROVIDES_POLARSSL)
hal_aes_module_t linux_aes_module_polarssl = {
    .priv_data_size     = sizeof(aes_context),
    .hexdump_result     = OSL_TRUE,
    .aes_init           = linux_aes_init_v1,
    .aes_destroy        = linux_aes_destroy_v1,
    .aes_cbc_encrypt    = linux_aes_cbc_encrypt_v1,
    .aes_cbc_decrypt    = linux_aes_cbc_decrypt_v1,
    .priv_data_size     = 128,
};
#endif /* defined(CONFIG_PROVIDES_POLARSSL) */

hal_aes_module_t linux_aes_module_openssl = {
    .aes_init           = linux_aes_init_v2,
    .aes_destroy        = linux_aes_destroy_v2,
    .aes_cbc_encrypt    = linux_aes_cbc_encrypt_v2,
    .aes_cbc_decrypt    = linux_aes_cbc_decrypt_v2,
    .priv_data_size     = 128,
};

hal_aes_module_t *linux_aes_get_module(int ver)
{
#if defined(CONFIG_PROVIDES_POLARSSL)
    if(ver == 1) {
        return &linux_aes_module_polarssl;
    } else {
        return &linux_aes_module_openssl;
    }
#else
    return &linux_aes_module_openssl;
#endif /* defined(CONFIG_PROVIDES_POLARSSL) */
}

#if defined(CONFIG_PROVIDES_POLARSSL)
int linux_aes_init_v1(void *priv_data)
{
    

    aes_init((aes_context *)priv_data);

    return 0;
}

int linux_aes_destroy_v1(void *priv_data)
{
    

    aes_free((aes_context *)priv_data);

    return 0;
}

int linux_aes_cbc_encrypt_v1(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_len,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len)
{
    unsigned char       iv[16] = {0};
    int                 aes_ret = -1;

    

    if(key_len != 256/8 && key_len != 192/8 && key_len != 128/8) {
        
        return -1;
    }
    if(src_len % 16 != 0) {
        
        return -2;
    }

    HEXDUMP_DEBUG(src, src_len);
    HEXDUMP_DEBUG(key, key_len);

    if(NULL == ivec) {
        memset(iv, 0, sizeof(iv));
    } else {
        memcpy(iv, ivec, sizeof(iv));
    }
    HEXDUMP_DEBUG(iv, sizeof(iv));

    aes_ret = aes_setkey_enc(priv_data, key, key_len*8);
    aes_ret = aes_crypt_cbc(priv_data, POLARSSL_AES_ENCRYPT, src_len, iv, src, dst);
    

    if(dst_len) {
        *dst_len = src_len;
    }

    return 0;
}

int linux_aes_cbc_decrypt_v1(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_len,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len)
{
    unsigned char           iv[16] = {0};
    int                     aes_ret = -1;

    

    if(key_len != 256/8 && key_len != 192/8 && key_len != 128/8) {
        
        return -1;
    }
    if(src_len % 16 != 0) {
        
        return -2;
    }

    HEXDUMP_DEBUG(src, src_len);
    HEXDUMP_DEBUG(key, key_len);

    if(ivec) {
        memcpy(iv, ivec, 16);
    } else {
        memset(iv, 0, 16);
    }
    HEXDUMP_DEBUG(iv, 16);

    aes_ret = aes_setkey_dec(priv_data, key, key_len*8);
    aes_ret = aes_crypt_cbc(priv_data, POLARSSL_AES_DECRYPT, src_len, iv, src, dst);
    

    if(dst_len) {
        *dst_len = src_len;
    }

    return aes_ret;
}
#endif /* defined(CONFIG_PROVIDES_POLARSSL) */

int linux_aes_init_v2(void *priv_data)
{
    return 0;
}

int linux_aes_destroy_v2(void *priv_data)
{
    return 0;
}

int linux_aes_cbc_encrypt_v2(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_len,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len)
{
    AES_KEY             aeskey;
    unsigned char       iv[16] = {0};
    int                 aes_ret = -1;

    if(NULL == ivec) {
        memset(iv, 0, sizeof(iv));
    } else {
        memcpy(iv, ivec, sizeof(iv));
    }

    aes_ret = AES_set_encrypt_key(key, key_len * 8, &aeskey);
    

    if(aes_ret) {
        return aes_ret;
    }
    AES_cbc_encrypt(src, dst, src_len, &aeskey, iv, AES_ENCRYPT);

    if(dst_len) {
        *dst_len = src_len;
    }

    return 0;
}

int linux_aes_cbc_decrypt_v2(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_len,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len)
{
    AES_KEY                 aeskey;
    unsigned char           iv[16] = {0};
    int                     aes_ret = -1;

    if(ivec) {
        memcpy(iv, ivec, 16);
    } else {
        memset(iv, 0, 16);
    }

    aes_ret = AES_set_decrypt_key(key, key_len * 8, &aeskey);
    
    AES_cbc_encrypt(src, dst, src_len, &aeskey, iv, AES_DECRYPT);

    if (dst_len) {
        *dst_len = src_len;
    }
    return aes_ret;
}
