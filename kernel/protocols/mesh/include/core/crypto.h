#ifndef UMESH_CRYPTO_H
#define UMESH_CRYPTO_H

ur_error_t umesh_aes_encrypt(const uint8_t *key, uint8_t key_size,
                             const void *src,
                             uint16_t size, void *dst);
ur_error_t umesh_aes_decrypt(const uint8_t *key, uint8_t key_size,
                             const void *src,
                             uint16_t size, void *dst);

#endif  /* UMESH_CRYPTO_H */
