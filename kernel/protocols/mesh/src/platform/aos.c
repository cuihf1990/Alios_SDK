/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdarg.h>

#include <aos/aos.h>
#include <ali_crypto.h>

#include <umesh_types.h>

/*
 * symbols to export
 */
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_init, "ur_error_t umesh_init(node_mode_t mode)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_start, "ur_error_t umesh_start(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_stop, "ur_error_t umesh_stop(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_device_state, "uint8_t umesh_get_device_state(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_mode, "uint8_t umesh_get_mode(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_set_mode, "ur_error_t umesh_set_mode(uint8_t mode)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_mac_address, "const mac_address_t *umesh_get_mac_address(media_type_t type)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, ur_adapter_get_default_ipaddr, "const void *ur_adapter_get_default_ipaddr(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, ur_adapter_get_mcast_ipaddr, "const void *ur_adapter_get_mcast_ipaddr(void)")

void *umesh_pal_malloc(int sz)
{
    return aos_malloc(sz);
}

void umesh_pal_free(void *ptr)
{
    aos_free(ptr);
}

uint32_t umesh_pal_now_ms(void)
{
    return aos_now_ms();
}

int umesh_pal_kv_get(const char *key, void *buf, int *len)
{
    return aos_kv_get(key, buf, len);
}

void umesh_pal_post_event(int code, unsigned long value)
{
    aos_post_event(EV_MESH, code, value);
}

void umesh_pal_log(const char *fmt, ...)
{
    va_list args;

    printf("[mesh][%06d] ", (unsigned)aos_now_ms());
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\r\n");
}

/*
 * security
 */
typedef void *umesh_aes_ctx_t;

uint8_t g_umesh_iv[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static ur_error_t umesh_aes_encrypt_decrypt(umesh_aes_ctx_t *aes,
                                            const void *src,
                                            uint16_t size,
                                            void *dst)
{
    ali_crypto_result result;
    uint32_t dlen = 1024;

    if (aes == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_finish(src, size, dst, &dlen, SYM_NOPAD, aes);

    if (result != ALI_CRYPTO_SUCCESS) {
        return UR_ERROR_FAIL;
    }

    return UR_ERROR_NONE;
}

ur_error_t umesh_pal_aes_encrypt(const uint8_t *key, uint8_t key_size,
                             const void *src,
                             uint16_t size, void *dst)
{
    ur_error_t error;
    umesh_aes_ctx_t *aes;
    uint32_t aes_ctx_size;
    ali_crypto_result result;

    if (key == NULL || src == NULL || dst == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_get_ctx_size(AES_CTR, &aes_ctx_size);
    if (result != ALI_CRYPTO_SUCCESS) {
        return UR_ERROR_FAIL;
    }

    aes = aos_malloc(aes_ctx_size);
    if (aes == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_init(AES_CTR, true,
                          key, NULL, key_size, g_umesh_iv, aes);
    if (result != ALI_CRYPTO_SUCCESS) {
        aos_free(aes);
        return UR_ERROR_FAIL;
    }

    error = umesh_aes_encrypt_decrypt(aes, src, size, dst);
    aos_free(aes);

    return error;
}

ur_error_t umesh_pal_aes_decrypt(const uint8_t *key, uint8_t key_size,
                             const void *src,
                             uint16_t size, void *dst)
{
    ur_error_t error;
    umesh_aes_ctx_t *aes;
    uint32_t aes_ctx_size;
    ali_crypto_result result;

    if (key == NULL || src == NULL || dst == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_get_ctx_size(AES_CTR, &aes_ctx_size);
    if (result != ALI_CRYPTO_SUCCESS) {
        return UR_ERROR_FAIL;
    }

    aes = aos_malloc(aes_ctx_size);
    if (aes == NULL) {
        return UR_ERROR_FAIL;
    }

    result = ali_aes_init(AES_CTR, false,
                          key, NULL, key_size, g_umesh_iv, aes);
    if (result != ALI_CRYPTO_SUCCESS) {
        aos_free(aes);
        return UR_ERROR_FAIL;
    }

    error = umesh_aes_encrypt_decrypt(aes, src, size, dst);
    aos_free(aes);

    return error;
}
