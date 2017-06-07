/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include "md5.h"

void pal_md5_sum(const uint8_t *addr, const int len, uint8_t *mac) {
    md5_sum(addr, len, mac);
    return;
}
