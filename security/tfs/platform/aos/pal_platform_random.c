/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <stdlib.h>
#include "yos/kernel.h"

int pal_get_random()
{
    srand((unsigned int)yos_now()/1000);
    return rand();
}

