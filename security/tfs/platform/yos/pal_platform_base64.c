/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include "base64.h"

void pal_base64_encode(const unsigned char *src, int len,
                                 unsigned char *dst, int *out_len) {
    base64_encode(src, len, dst, (size_t *)out_len);
    return;
}
