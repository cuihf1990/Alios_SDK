/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include "yos/yos.h"

void *pal_memory_malloc(size_t size)
{
    return yos_malloc(size);
}

void pal_memory_free(void *ptr)
{
    yos_free(ptr);
    return;
}
