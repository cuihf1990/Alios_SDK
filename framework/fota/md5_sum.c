/*
 *  Copyright (C) 2016 YunOS Project. All rights reserved.
 */

#include <stdint.h>
#include <stddef.h>

#include "md5.h"

void __attribute__ ((weak)) md5_sum(const uint8_t * addr, const size_t len, uint8_t * mac)
{
  MD5_CTX ctx;

  MD5Init(&ctx);
  MD5Update(&ctx, addr, len);
  MD5Final(mac, &ctx);
}

