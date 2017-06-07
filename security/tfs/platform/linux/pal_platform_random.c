/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <stdlib.h>
#include <time.h>

int pal_get_random()
{
    srand((unsigned int)time(NULL));
    return rand();
}

