/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdlib.h>
#include "yos/kernel.h"

int pal_get_random()
{
    srand((unsigned int)yos_now() / 1000);
    return rand();
}

