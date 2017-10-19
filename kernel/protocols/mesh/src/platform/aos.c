/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdarg.h>
#include <aos/aos.h>

/*
 * symbols to export
 */
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_init, "ur_error_t umesh_init(node_mode_t mode)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_start, "ur_error_t umesh_start(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_stop, "ur_error_t umesh_stop(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_device_state, "uint8_t umesh_get_device_state(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_mode, "uint8_t umesh_get_mode(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_set_mode, "ur_error_t umesh_set_mode(uint8_t mode)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, umesh_get_mac_address, "const mac_address_t *umesh_get_mac_address(media_type_t type)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, ur_adapter_get_default_ipaddr, "const void *ur_adapter_get_default_ipaddr(void)")
EXPORT_SYMBOL_K(CONFIG_AOS_MESH > 0u, ur_adapter_get_mcast_ipaddr, "const void *ur_adapter_get_mcast_ipaddr(void)")

void *umesh_pal_malloc(int sz)
{
    return aos_malloc(sz);
}

void umesh_pal_free(void *ptr)
{
    aos_free(ptr);
}

uint32_t umesh_pal_now_ms(void)
{
    return aos_now_ms();
}

int umesh_pal_kv_get(const char *key, void *buf, int *len)
{
    return aos_kv_get(key, buf, len);
}

void umesh_pal_post_event(int code, unsigned long value)
{
    aos_post_event(EV_MESH, code, value);
}

void umesh_pal_log(const char *fmt, ...)
{
    va_list args;

    printf("[mesh][%06d] ", (unsigned)aos_now_ms());
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\r\n");
}
