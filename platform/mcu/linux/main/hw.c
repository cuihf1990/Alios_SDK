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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <k_api.h>
#include <yos/log.h>
#include <hal/soc/soc.h>
#include <hal/soc/timer.h>
#include <hal/base.h>
#include <hal/wifi.h>
#include <hal/ota.h>

#define TAG "hw"

static int open_flash(int pno, bool w)
{
    char fn[64];
    int flash_fd;

    snprintf(fn, sizeof(fn), "./yos_partition_%d_%d.bin", getpid(), pno);
    if(w)
        flash_fd = open(fn, O_RDWR);
    else
        flash_fd = open(fn, O_RDONLY);

    if (flash_fd < 0) {
        umask(0111);
        close(creat(fn, S_IRWXU | S_IRWXG));
        flash_fd = open(fn, O_RDWR);
    }
    return flash_fd;
}

int32_t hal_flash_write(hal_partition_t pno, uint32_t* poff, const void* buf ,uint32_t buf_size)
{
    int ret;
    char *tmp, *new_val;

    int flash_fd = open_flash(pno, true);
    if (flash_fd < 0)
        return -1;

    char *origin = (char *)malloc(buf_size);
    if (!origin) {
        ret = -1;
        goto exit;
    }
    memset(origin, -1, buf_size);

    ret = pread(flash_fd, origin, buf_size, *poff);
    if (ret < 0) {
        perror("error reading flash:");
        goto exit;
    }

    tmp = origin;
    new_val = (char *)buf;
    for (int i = 0; i < buf_size; i++) {
        (*tmp) &= (*new_val);
        new_val++;
        tmp++;
    }

    ret = pwrite(flash_fd, origin, buf_size, *poff);
    if (ret < 0)
        perror("error writing flash:");
    else if (poff)
        *poff += ret;

exit:
    close(flash_fd);
    free(origin);
    return ret < 0 ? ret : 0;
}

int32_t hal_flash_read(hal_partition_t pno, uint32_t* poff, void* buf, uint32_t buf_size)
{
    int flash_fd = open_flash(pno, false);
    if (flash_fd < 0)
        return -1;

    int ret = pread(flash_fd, buf, buf_size, *poff);
    if (ret < 0)
        perror("error reading flash:");
    else if (poff)
        *poff += ret;
    close(flash_fd);

    return ret < 0 ? ret : 0;
}

int32_t hal_flash_erase(hal_partition_t in_partition, uint32_t off_set,
                        uint32_t size)
{
    int ret;
    int flash_fd = open_flash(in_partition, true);
    if (flash_fd < 0)
        return -1;
    
    char *buf = (char *)malloc(size);
    if (!buf) {
        ret = -1;
        goto exit;
    }
    memset(buf, -1, size);

    ret = pwrite(flash_fd, buf, size, off_set);
    if (ret < 0)
        perror("error erase flash:");
    
exit:
    close(flash_fd);
    free(buf);
    return ret < 0 ? ret : 0;
}

void hal_reboot(void)
{

}

#define us2tick(us) \
    ((us * YUNOS_CONFIG_TICKS_PER_SECOND + 999999) / 1000000)

static void _timer_cb(void *timer, void *arg)
{
    hal_timer_t *tmr = arg;
    tmr->cb(tmr->arg);
}

void hal_timer_init(hal_timer_t *tmr, unsigned int period, unsigned char auto_reload, unsigned char ch, hal_timer_cb_t cb, void *arg)
{
    (void)ch;
    bzero(tmr, sizeof(*tmr));
    tmr->cb = cb;
    tmr->arg = arg;
    if (auto_reload > 0u) {
        yunos_timer_dyn_create((ktimer_t **)&tmr->priv, "hwtmr", _timer_cb,
                                us2tick(period), us2tick(period), tmr, 0);
    }
    else {
        yunos_timer_dyn_create((ktimer_t **)&tmr->priv, "hwtmr", _timer_cb,
                                us2tick(period), 0, tmr, 0);
    }
}

int hal_timer_start(hal_timer_t *tmr)
{
    return yunos_timer_start(tmr->priv);
}

void hal_timer_stop(hal_timer_t *tmr)
{
    yunos_timer_stop(tmr->priv);
    yunos_timer_dyn_del(tmr->priv);
    tmr->priv = NULL;
}

int csp_printf(const char *fmt, ...)
{
    va_list args;
    int ret;

    va_start(args, fmt);
    ret = vprintf(fmt, args);
    va_end(args);

    fflush(stdout);

    return ret;
}

extern hal_wifi_module_t sim_yos_wifi_linux;
extern struct hal_ota_module_s linuxhost_ota_module;
uart_dev_t uart_0;

void linux_wifi_register(void);
void hw_start_hal(void)
{
    uart_0.port                = 0;
    uart_0.config.baud_rate    = 921600;
    uart_0.config.data_width   = DATA_WIDTH_8BIT;
    uart_0.config.parity       = NO_PARITY;
    uart_0.config.stop_bits    = STOP_BITS_1;
    uart_0.config.flow_control = FLOW_CONTROL_DISABLED;

#ifdef CONFIG_YOS_CLI
    hal_uart_init(&uart_0);
#endif

    hal_wifi_register_module(&sim_yos_wifi_linux);
    hal_ota_register_module(&linuxhost_ota_module);
#ifdef LINUX_MESH_80211
    linux_wifi_register();
#endif
}
