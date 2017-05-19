#include <stdlib.h>
#include <sys/time.h>
#include "hal/hal.h"

int pal_get_random()
{
    struct timeval tv;

    hal_time_gettimeofday(&tv, NULL);
    srand((unsigned int)tv.tv_sec);
    return rand();
}

