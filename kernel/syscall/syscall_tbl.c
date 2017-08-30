/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <k_api.h>
#include <yos/cli.h>
#include <yos/kernel.h>
#include <yos/yos.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <hal/wifi.h>
#include <hal/ota.h>
#include <hal/soc/soc.h>
#include <umesh.h>

extern void hal_wlan_register_mgnt_monitor_cb(hal_wifi_module_t *m, monitor_data_cb_t fn);
extern int  hal_wlan_send_80211_raw_frame(hal_wifi_module_t *m, uint8_t *buf, int len);

void *sys_yos_malloc(unsigned int size, size_t allocator)
{
    void *tmp = yos_malloc(size);

    if (tmp == NULL) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        yunos_owner_attach(g_kmm_head, tmp, allocator);
    }
#endif

    return tmp;
}

void *sys_yos_realloc(void *mem, unsigned int size, size_t allocator)
{
    void *tmp = yos_realloc(mem, size);

    if (tmp == NULL) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        yunos_owner_attach(g_kmm_head, tmp, allocator);
    }
#endif

    return tmp;
}

void *sys_yos_zalloc(unsigned int size, size_t allocator)
{
    void *tmp = yos_zalloc(size);

    if (tmp == NULL) {
        return NULL;
    }

#if (YUNOS_CONFIG_MM_DEBUG > 0u && YUNOS_CONFIG_GCC_RETADDR > 0u)
    if ((size & YOS_UNSIGNED_INT_MSB) == 0) {
        yunos_owner_attach(g_kmm_head, tmp, allocator);
    }
#endif

    return tmp;
}

#define SYSCALL_MAX 188
#define SYSCALL_NUM 141

#define SYSCALL(nr, func) [nr] = func,

const void *g_syscall_tbl[] __attribute__ ((section(".syscall_tbl"))) = {
    [0 ... SYSCALL_MAX - 1] = (void *)0XABCDABCD,
#include <syscall_tbl.h>
};

