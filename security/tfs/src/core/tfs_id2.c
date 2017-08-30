#include "id2.h"

int tfs_get_ID2(uint8_t *id2, uint32_t *len)
{
    return get_ID2(id2, len);
}

int tfs_get_auth_code(uint8_t *auth_code, uint32_t *len)
{
    return get_auth_code(auth_code, len);
}

int tfs_id2_sign(const uint8_t *in, uint32_t in_len,
                 uint8_t *sign, uint32_t *sign_len)
{
    return id2_sign(in, in_len, sign, sign_len);
}

int tfs_id2_verify(const uint8_t *in, uint32_t in_len,
                   uint8_t *sign, uint32_t sign_len)
{
    return id2_verify(in, in_len, sign, sign_len);
}

int tfs_id2_encrypt(uint8_t *in, uint32_t in_len,
                    uint8_t *out, uint32_t *out_len)
{
    return id2_encrypt(in, in_len, out, out_len);
}

int tfs_id2_decrypt(uint8_t *in, uint32_t in_len,
                    uint8_t *out, uint32_t *out_len)
{
    return id2_decrypt(in, in_len, out, out_len);
}

int tfs_is_device_activated(void)
{
    return is_id2_activated();
}

int tfs_activate_device(void)
{
    return activate_device();
}

int tfs_id2_get_auth_code(uint64_t timestamp, uint8_t *auth_code,
                          uint32_t *auth_len)
{
    return id2_get_auth_code(timestamp, auth_code, auth_len);
}

int tfs_id2_get_digest_auth_code(uint64_t timestamp, uint8_t *digest,
                                 uint32_t digest_len, uint8_t *auth_code, uint32_t *auth_len)
{
    return id2_get_digest_auth_code(timestamp, digest, digest_len, auth_code,
                                    auth_len);
}

