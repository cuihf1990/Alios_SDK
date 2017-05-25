#include "base64.h"

void pal_base64_encode(const unsigned char *src, int len,
                                 unsigned char *dst, int *out_len) {
    base64_encode(src, len, dst, (size_t *)out_len);
    return;
}

void pal_base64_decode(const unsigned char *src, int len,
                                 unsigned char *dst, int *out_len) {
    base64_decode(src, len, dst, (size_t *)out_len);
    return;
}
