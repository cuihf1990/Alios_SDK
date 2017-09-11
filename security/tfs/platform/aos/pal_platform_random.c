/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdlib.h>
#include "aos/aos.h"

int pal_get_random()
{
    srand((unsigned int)aos_now() / 1000);
    return rand();
}

