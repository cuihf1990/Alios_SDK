#ifndef __HAL_IMPL_LINUX_H__
#define __HAL_IMPL_LINUX_H__

#include <openssl/aes.h>
#if defined(CONFIG_PROVIDES_POLARSSL)
#include "polarssl/aes.h"
#endif

#include <hal/hal.h>

hal_aes_module_t *linux_aes_get_module(int ver);
int linux_aes_init_v1(void *priv_data);
int linux_aes_destroy_v1(void *priv_data);
int linux_aes_cbc_encrypt_v1(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_size,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len);
int linux_aes_cbc_decrypt_v1(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_size,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len);

int linux_aes_init_v2(void *priv_data);
int linux_aes_destroy_v2(void *priv_data);
int linux_aes_cbc_encrypt_v2(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_size,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len);
int linux_aes_cbc_decrypt_v2(
        void *priv_data,
        const unsigned char *src, const unsigned int src_len,
        const unsigned char *key, const int key_size,
        const unsigned char *ivec,
        unsigned char *dst, unsigned int *dst_len);

#endif  /* __HAL_IMPL_LINUX_H__ */

