/**
 * Copyright (C) 2017 The YunOS IoT Project. All rights reserved.
 */

#ifndef _THREADING_ALT_H_
#define _THREADING_ALT_H_

#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include <stdlib.h>
#include <yos/kernel.h>

typedef struct
{
    yos_mutex_t mutex;
    char is_valid;
} mbedtls_threading_mutex_t;

void threading_mutex_init(mbedtls_threading_mutex_t *mutex);
void threading_mutex_free(mbedtls_threading_mutex_t *mutex);
int threading_mutex_lock(mbedtls_threading_mutex_t *mutex);
int threading_mutex_unlock(mbedtls_threading_mutex_t *mutex);

#endif /* _THREADING_ALT_H_*/
