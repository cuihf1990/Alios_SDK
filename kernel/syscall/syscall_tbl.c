/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <aos/aos.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <hal/hal.h>
#include <umesh.h>

extern void hal_wlan_register_mgnt_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn);
extern int  hal_wlan_send_80211_raw_frame(hal_wifi_module_t *m, uint8_t *buf, int len);

void *sys_aos_malloc(unsigned int size, size_t allocator)
{
    void *tmp = aos_malloc(size);

    if (tmp == NULL) {
        return NULL;
    }

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        krhino_owner_attach(g_kmm_head, tmp, allocator);
    }
#endif

    return tmp;
}

void *sys_aos_realloc(void *mem, unsigned int size, size_t allocator)
{
    void *tmp = aos_realloc(mem, size);

    if (tmp == NULL) {
        return NULL;
    }

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        krhino_owner_attach(g_kmm_head, tmp, allocator);
    }
#endif

    return tmp;
}

void *sys_aos_zalloc(unsigned int size, size_t allocator)
{
    void *tmp = aos_zalloc(size);

    if (tmp == NULL) {
        return NULL;
    }

#if (RHINO_CONFIG_MM_DEBUG > 0u && RHINO_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        krhino_owner_attach(g_kmm_head, tmp, allocator);
    }
#endif

    return tmp;
}

#define SYSCALL_MAX 188
#define SYSCALL_NUM 150

#define SYSCALL(nr, func) [nr] = func,

const void *g_syscall_tbl[] __attribute__ ((section(".syscall_tbl"))) = {
    [0 ... SYSCALL_MAX - 1] = (void *)0XABCDABCD,
#include <syscall_tbl.h>
};

