/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <syscall_uapi.h>
#include <k_api.h>
#include <aos/aos.h>
#include <hal/hal.h>
#ifdef CONFIG_AOS_MESH
#include <umesh.h>
#endif

#ifdef MBEDTLS_IN_KERNEL
#include <ali_crypto.h>
#endif

#define SYSCALL(nr, func)

#include <syscall_tbl.h>

#ifdef WITH_LWIP
#include <aos/network.h>
#endif

/* ---------------------function--------------------- */
typedef void (*aos_event_cb)(input_event_t *event, void *private_data);
typedef void (*aos_call_t)(void *arg);
typedef void (*aos_poll_call_t)(int fd, void *arg);
typedef void *aos_loop_t;


#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
extern void *sys_aos_malloc(unsigned int size, size_t allocator);
extern void *sys_aos_realloc(void *mem, unsigned int size, size_t allocator);
extern void *sys_aos_zalloc(unsigned int size, size_t allocator);
void *aos_malloc(unsigned int size)
{
    if ((size & AOS_UNSIGNED_INT_MSB) == 0) {
        return sys_aos_malloc(size, (size_t)__builtin_return_address(0));
    } else {
        return sys_aos_malloc(size, 0);
    }
}

void *aos_realloc(void *mem, unsigned int size)
{
    if ((size & AOS_UNSIGNED_INT_MSB) == 0) {
        return sys_aos_realloc(mem, size, (size_t)__builtin_return_address(0));
    } else {
        return sys_aos_realloc(mem, size, 0);
    }
}

void *aos_zalloc(unsigned int size)
{
    if ((size & AOS_UNSIGNED_INT_MSB) == 0) {
        return sys_aos_zalloc(size, (size_t)__builtin_return_address(0));
    } else {
        return sys_aos_zalloc(size, 0);
    }
}
#endif

/* ---------------------syscall function--------------------- */
