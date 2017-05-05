/**
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 */

/**
 *                      caution
 * linuxhost hw.c won't use any lwip functionalities,
 * disable WITH_LWIP to avoid close() -> lwip_close()
 */
#undef WITH_LWIP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <hal/hal.h>
#include <csp.h>
#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/log.h>

#define TAG "hw"

static int linuxhost_wifi_init(hal_wifi_module_t *m, void *something)
{
    return 0;
}

static int linuxhost_wifi_set_mode(hal_wifi_module_t *m, hal_wifi_mode_t mode)
{
    return 0;
}

static void post_delay_work(void (*f)(void *), void *arg)
{
    /* this work for linuxhost, to workaround thread not scheduled problem */
    yos_schedule_call(f, arg);
}

static void network_connected(void *arg)
{
    hal_wifi_module_t *m = (hal_wifi_module_t *)arg;

    m->status.connected = 1;
    if (m->ev_cb && m->ev_cb->connected_to_ap) {
        char ap[4] = { 't', 'e', 's', 't' };
        m->ev_cb->connected_to_ap(m, ap, 4, ap, 4);
    }

    if (m->ev_cb && m->ev_cb->got_ip_from_ap) {
        char ip[32] = {0x7f, 0x00, 0x00, 0x01};
        char mask[32] = {0xFF, 0xFF, 0xFF, 0x00};
        char gw[32] = {0x7f, 0x00, 0x00, 0x01};

        m->ev_cb->got_ip_from_ap(m, (hal_ip_addr *)ip, (hal_ip_addr *)mask, (hal_ip_addr *)gw);
    }
}

static void network_disconnected(void *arg)
{
    hal_wifi_module_t *m = (hal_wifi_module_t *)arg;

    m->status.connected = 0;
    if (m->ev_cb && m->ev_cb->disconnected_from_ap) {
        char ap[4] = { 't', 'e', 's', 't' };
        m->ev_cb->disconnected_from_ap(m, ap, 4, 4);
    }
}

struct ap_info {
    dlist_t node;
    char ssid[32];
};

static void network_scan_done(void *arg)
{
    hal_wifi_module_t *m = (hal_wifi_module_t *)arg;

    hal_wifi_scan_result_t *dummy = malloc(sizeof(*dummy));
    memset(dummy, 0, sizeof(*dummy));
    memcpy(dummy->bssid, "12345", 6);
    memcpy(dummy->ssid, "test", 4);
    dummy->ssid_len = 4;

    m->status.ap_num = 1;
    m->status.ap_records = dummy;

    if (m->ev_cb && m->ev_cb->scan_compeleted) {
        m->ev_cb->scan_compeleted(m, 0, 1, dummy);
    }
}

static void promiscuous_start(void *arg)
{
    hal_wifi_module_t *m = (hal_wifi_module_t *)arg;

    if (m->ev_cb && m->ev_cb->promiscuous_data) {
        m->ev_cb->promiscuous_data(m, 0, 0);
    }
}

static int linuxhost_wifi_connect_prepare(hal_wifi_module_t *m, char *ssid, char *password)
{
    if (strlen(ssid) > 32)
        return -1;
    if (strlen(password) > 64)
        return -1;

    post_delay_work(network_connected, m);

    return 0;
}

static int linuxhost_wifi_scan(hal_wifi_module_t *m, void *__not_use)
{
    post_delay_work(network_scan_done, m);

    return 0;
}

static int linuxhost_wifi_disconnect(hal_wifi_module_t *m)
{
    post_delay_work(network_disconnected, m);

    return 0;
}

static int linuxhost_wifi_getsetops(hal_wifi_module_t *m, hal_wifi_getset_cmd_t cmd, ...)
{
    static int channel = 1;
    va_list args;

    va_start(args, cmd);
    switch(cmd) {
        case HAL_WIFI_PROMISCUOUS_START:
            post_delay_work(promiscuous_start, m);
            break;
        case HAL_WIFI_SET_AUTO_CONNECT:
            break;
        case HAL_WIFI_CHECK_SUPPORT_SMART_CONFIG:
            return 1;
        case HAL_WIFI_SMART_CONFIG_START:
            post_delay_work(network_connected, m);
            break;
        case HAL_WIFI_SMART_CONFIG_STOP:
            break;
        case HAL_WIFI_GET_BROADCAST_PORT:
            return 10080;
        case HAL_WIFI_GET_CHANNEL:
            return channel;
        case HAL_WIFI_SET_CHANNEL:
            channel = va_arg(args, int);
            break;
        case HAL_WIFI_GET_MAC_ADDRESS:
        {
            uint8_t *mac;
            uint8_t fixmac[] = {0xd8,0x96,0xe0,0x03,0x04,0x01};
            va_arg(args, int);
            mac = va_arg(args, uint8_t *);
            memcpy(mac, fixmac, sizeof fixmac);
        }
            break;
        default:
            break;
    }
    va_end(args);

    return 0;
}

static hal_wifi_module_t linuxhost_wifi_module = {
    .base.name          = "yoc_wifi_module_linuxhost",
    .init               = linuxhost_wifi_init,
    .set_mode           = linuxhost_wifi_set_mode,
    .connect_prepare    = linuxhost_wifi_connect_prepare,
    .disconnect         = linuxhost_wifi_disconnect,
    .scan               = linuxhost_wifi_scan,
    .getset_ops         = linuxhost_wifi_getsetops,
};

static int open_flash(char *cat, bool w)
{
    char fn[64];
    int flash_fd;
    snprintf(fn, sizeof fn, "/tmp/yoc_%s.bin", cat);
    flash_fd = open(fn, O_RDWR);
    if (w && flash_fd < 0) {
        umask(0111);
        close(creat(fn, S_IRWXU | S_IRWXG));
        flash_fd = open(fn, O_RDWR);
    }
    return flash_fd;
}

static int linuxhost_flash_init(hal_flash_module_t *m, void *something)
{
    return 0;
}

static int linuxhost_flash_read_conf(hal_flash_module_t *m, char *key, unsigned char *buf, int buf_size)
{
    int flash_fd = open_flash(key, false);
    if (flash_fd < 0)
        return -1;
    pread(flash_fd, buf, buf_size, 0);
    close(flash_fd);
    return 0;
}

static int linuxhost_flash_write_conf(hal_flash_module_t *m, char *key, unsigned char *buf, int buf_size)
{
    int flash_fd = open_flash(key, true);
    if (flash_fd < 0)
        return -1;

    int ret = pwrite(flash_fd, buf, buf_size, 0);
    if (ret < 0)
        perror("error writing flash:");
    close(flash_fd);
    return 0;
}

static int linuxhost_flash_get_conf_size(hal_flash_module_t *m)
{
    return 0x10000;
}

static int linuxhost_flash_ota_update(hal_flash_module_t *m)
{
    return -1;
}

static hal_flash_module_t linuxhost_flash_module = {
    .init = linuxhost_flash_init,
    .read_conf = linuxhost_flash_read_conf,
    .write_conf = linuxhost_flash_write_conf,
    .get_conf_size = linuxhost_flash_get_conf_size,
    .reset = linuxhost_flash_ota_update,
};

extern hal_sensor_module_t linuxhost_sensor_module;

void hw_start_hal(void)
{
    /* Register hardware capability */
    hal_wifi_register_module(&linuxhost_wifi_module);

    hal_flash_register_module(&linuxhost_flash_module);

    extern hal_aes_module_t *linux_aes_get_module(int ver);
    hal_aes_register(linux_aes_get_module(2));

#ifdef CONFIG_YOC_URADAR_MESH
    extern void linuxhost_hal_urmesh_register(void);
    linuxhost_hal_urmesh_register();
#endif
    hal_sensor_register_module(&linuxhost_sensor_module);

    /* Do YOC startup */
    yoc_hal_init();
}
