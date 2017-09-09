/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include "aos/aos.h"

void *pal_memory_malloc(size_t size)
{
    return aos_malloc(size);
}

void pal_memory_free(void *ptr)
{
    aos_free(ptr);
    return;
}
