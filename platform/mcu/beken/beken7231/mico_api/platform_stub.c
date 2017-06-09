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


#include "common.h"

#undef errno
extern int errno;

#ifndef EBADF
#include <errno.h>
#endif

/************** wrap C library functions **************/
void * __wrap_malloc (size_t size)
{
	return os_malloc(size);
}

void * __wrap__malloc_r (void *p, size_t size)
{
	
	return os_malloc(size);
}

void __wrap_free (void *pv)
{
	os_free(pv);
}

void * __wrap_calloc (size_t a, size_t b)
{
	void *pvReturn;

    pvReturn = os_malloc( a*b );
    if (pvReturn)
    {
        os_memset(pvReturn, 0, a*b);
    }

    return pvReturn;
}

void * __wrap_realloc (void* pv, size_t size)
{
	return os_realloc(pv, size);
}

void __wrap__free_r (void *p, void *x)
{
  __wrap_free(x);
}

void* __wrap__realloc_r (void *p, void* x, size_t sz)
{
  return __wrap_realloc (x, sz);
}


