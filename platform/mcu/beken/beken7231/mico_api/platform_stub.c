/**
 ******************************************************************************
 * @file    platform_init.c
 * @author  William Xu
 * @version V1.0.0
 * @date    05-May-2014
 * @brief   This file provide functions called by MICO to drive stm32f2xx
 *          platform: - e.g. power save, reboot, platform initialize
 ******************************************************************************
 *  UNPUBLISHED PROPRIETARY SOURCE CODE
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  The contents of this file may not be disclosed to third parties, copied or
 *  duplicated in any form, in whole or in part, without the prior written
 *  permission of MXCHIP Corporation.
 ******************************************************************************
 */

#include <stdarg.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <k_api.h>

#include "common.h"

#undef errno
extern int errno;

#ifndef EBADF
#include <errno.h>
#endif

/************** wrap C library functions **************/
void * __wrap_malloc (size_t size)
{
    void *tmp = NULL;

    if (size == 0) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    tmp = yunos_mm_alloc(size|YOS_UNSIGNED_INT_MSB);
    yunos_owner_attach(g_kmm_head, tmp, (size_t)__builtin_return_address(0));
#else
    tmp = yunos_mm_alloc(size);
#endif

    return tmp;
}


void * __wrap__malloc_r (void *p, size_t size)
{
    void *tmp = NULL;

    if (size == 0) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    tmp = yunos_mm_alloc(size|YOS_UNSIGNED_INT_MSB);
    yunos_owner_attach(g_kmm_head, tmp, (size_t)__builtin_return_address(0));
#else
    tmp = yunos_mm_alloc(size);
#endif

    return tmp;
}


void __wrap_free (void *pv)
{
    yunos_mm_free(pv);
}

void * __wrap_calloc (size_t a, size_t b)
{
    void *tmp = NULL;
    size_t size = a * b;
    if (size == 0) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
        tmp = yunos_mm_alloc(size|YOS_UNSIGNED_INT_MSB);
        yunos_owner_attach(g_kmm_head, tmp, (size_t)__builtin_return_address(0));
#else
        tmp = yunos_mm_alloc(size);
#endif
    if (tmp)
        bzero(tmp, size);
    return tmp;
}


void * __wrap_realloc (void* pv, size_t size)
{
    void *tmp = NULL;

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    tmp = yunos_mm_realloc(pv, size|YOS_UNSIGNED_INT_MSB);
    yunos_owner_attach(g_kmm_head, tmp, (size_t)__builtin_return_address(0));
#else
    tmp = yunos_mm_realloc(pv, size);
#endif

    return tmp;
}


void __wrap__free_r (void *p, void *x)
{
  __wrap_free(x);
}

void* __wrap__realloc_r (void *p, void* x, size_t sz)
{
    void *tmp = NULL;

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    tmp = yunos_mm_realloc(x, sz|YOS_UNSIGNED_INT_MSB);
    yunos_owner_attach(g_kmm_head, x, (size_t)__builtin_return_address(0));
#else
    tmp = yunos_mm_realloc(x, sz);
#endif

    return tmp;
}



