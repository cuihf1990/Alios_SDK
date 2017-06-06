/*
 *  Copyright (C) 2015 YunOS Project. All rights reserved.
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

uint64_t pal_get_current_time() {
    struct timeval now;
    gettimeofday(&now, NULL);
    return 1000000 * now.tv_sec + now.tv_usec;
}
