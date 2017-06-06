#include "include.h"
#include "arm_arch.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mmgmt.h"
#include <k_api.h>

INT32 os_memcmp(const void *s1, const void *s2, UINT32 n)
{
    return memcmp(s1, s2, (unsigned int)n);
}

void *os_memmove(void *out, const void *in, UINT32 n)
{
    return memmove(out, in, n);
}

void *os_memcpy(void *out, const void *in, UINT32 n)
{
    return memcpy(out, in, n);
}

void *os_memset(void *b, int c, UINT32 len)
{
    return (void *)memset(b, c, (unsigned int)len);
}

#if 0
void os_mem_init(void)
{
    sys_mem_init();
}

void *os_malloc(size_t size)
{
    return mmgmt_malloc(size);
}

void * os_zalloc(size_t size)
{
	void *n = os_malloc(size);
	if (n)
		os_memset(n, 0, size);
	return n;
}

void *os_realloc(void *ptr, size_t size)
{
    return mmgmt_realloc(ptr, size);
}

void os_free(void *ptr)
{
    if(ptr)
    {
        mmgmt_free(ptr);
    }
}

#else

void os_mem_init(void)
{
 
}

void *os_malloc(size_t size)
{
    return yunos_mm_alloc(size);
}

void * os_zalloc(size_t size)
{
	void *n = yunos_mm_alloc(size);

	if (n) {
		os_memset(n, 0, size);
    }

	return n;
}

void *os_realloc(void *ptr, size_t size)
{
    yunos_mm_free(ptr);

    return yunos_mm_alloc(size);
}

void os_free(void *ptr)
{
    if(ptr)
    {
        yunos_mm_free(ptr);
    }
}

#endif


int os_memcmp_const(const void *a, const void *b, size_t len)
{
	const u8 *aa = a;
	const u8 *bb = b;
	size_t i;
	u8 res;

	for (res = 0, i = 0; i < len; i++)
		res |= aa[i] ^ bb[i];

	return res;
}

// EOF
