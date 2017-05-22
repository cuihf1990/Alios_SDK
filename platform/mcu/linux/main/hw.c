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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <yos/log.h>
#include <hal/soc/soc.h>

#define TAG "hw"

static int open_flash(int pno, bool w)
{
    char fn[64];
    int flash_fd;
    snprintf(fn, sizeof fn, "/tmp/yos_partition_%d.bin", pno);
    flash_fd = open(fn, O_RDWR);
    if (w && flash_fd < 0) {
        umask(0111);
        close(creat(fn, S_IRWXU | S_IRWXG));
        flash_fd = open(fn, O_RDWR);
    }
    return flash_fd;
}

int hal_flash_write(hal_partition_t pno, uint32_t* poff, const void* buf ,uint32_t buf_size)
{
    int flash_fd = open_flash(pno, true);
    if (flash_fd < 0)
        return -1;

    int ret = pwrite(flash_fd, buf, buf_size, 0);
    if (ret < 0)
        perror("error writing flash:");
    else if (poff)
        *poff += ret;
    close(flash_fd);

    return ret < 0 ? ret : 0;
}

int hal_flash_read(hal_partition_t pno, uint32_t* poff, void* buf, uint32_t buf_size)
{
    int flash_fd = open_flash(pno, false);
    if (flash_fd < 0)
        return -1;

    int ret = pread(flash_fd, buf, buf_size, 0);
    if (ret < 0)
        perror("error reading flash:");
    else if (poff)
        *poff += ret;
    close(flash_fd);

    return ret < 0 ? ret : 0;
}

void hw_start_hal(void)
{
}
