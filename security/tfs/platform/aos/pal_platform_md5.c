/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdint.h>
#include "digest_algorithm.h"

void pal_md5_sum(const uint8_t *addr, const int len, uint8_t *mac)
{
    digest_md5(addr, len, mac);
    return;
}
