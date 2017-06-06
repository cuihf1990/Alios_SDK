/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <stdlib.h>

void* pal_memory_malloc(size_t size)
{
    return malloc(size);
}

void pal_memory_free(void* ptr)
{
    free(ptr);
    return;
}
