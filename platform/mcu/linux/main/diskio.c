/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "fatfs_diskio.h"
#include "ff.h"

#define RAMDISK_BLOCK_SIZE      4096
#define RAMDISK_SECTOR_COUNT    256
#define RAMDISK_SECTOR_SIZE     FF_MAX_SS
#define RAMDISK_SEC_PER_BLOCK   (RAMDISK_BLOCK_SIZE / RAMDISK_SECTOR_SIZE)

static DSTATUS ramdisk_status()
{
    return 0;
}

static DSTATUS ramdisk_initialize()
{
    return 0;
}

static INT ramdisk_open(BYTE w)
{
    INT fd;
    BYTE path[] = "./yos_partition_ramdisk.bin";

    if (w)
        fd = open(path, O_RDWR);
    else
        fd = open(path, O_RDONLY);

    if (fd < 0) {
        umask(0111);
        close(creat(path, S_IRWXU | S_IRWXG));
        fd = open(path, O_RDWR);
    }

    return fd;
}

static DRESULT ramdisk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    INT fd = ramdisk_open(0);
    if (fd < 0)
        return RES_ERROR;

    INT ret = pread(fd, buff, (count * FF_MAX_SS), (sector * FF_MAX_SS));
    close(fd);

    if (ret < 0)
        return RES_ERROR;
    else
        return RES_OK;
}

static DRESULT ramdisk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    INT fd = ramdisk_open(1);
    if (fd < 0)
        return RES_ERROR;

    INT ret = pwrite(fd, buff, (count * FF_MAX_SS), (sector * FF_MAX_SS));
    close(fd);

    if (ret < 0)
        return RES_ERROR;
    else
        return RES_OK;
}

static DRESULT ramdisk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    switch (cmd) {
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = RAMDISK_SECTOR_COUNT;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *(WORD *)buff = RAMDISK_SECTOR_SIZE;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *(DWORD *)buff = RAMDISK_SEC_PER_BLOCK;
            return RES_OK;
#if FF_USE_MKFS && !FF_FS_READONLY
        case GET_FORMAT_OPTION:
            *(BYTE *)buff = (FM_ANY | FM_SFD);
            return RES_OK;
#endif
        case CTRL_SYNC:
        default:
            return RES_OK;
    }

    return RES_PARERR;
}

DSTATUS ff_disk_status(BYTE pdrv)
{
    switch (pdrv) {
        case DEV_RAM:
             return ramdisk_status();

        case DEV_MMC:
            // return sdmmc_status();

        case DEV_USB:
            // retutn usb_status();

        default:
            break;
    }
    return STA_NOINIT;
}

DSTATUS ff_disk_initialize(BYTE pdrv)
{
    switch (pdrv) {
        case DEV_RAM:
             return ramdisk_initialize();

        case DEV_MMC:
            // return sdmmc_initialize();

        case DEV_USB:
            // return usb_initialize();

        default:
            break;
    }
    return STA_NOINIT;
}

DRESULT ff_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    switch (pdrv) {
        case DEV_RAM:
             return ramdisk_read(pdrv, buff, sector, count);

        case DEV_MMC:
            // return sdmmc_read();

        case DEV_USB:
            // return usb_read();

        default:
            break;
    }
    return RES_PARERR;
}

DRESULT ff_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    switch (pdrv) {
        case DEV_RAM:
             return ramdisk_write(pdrv, buff, sector, count);

        case DEV_MMC:
            // return sdmmc_write();

        case DEV_USB:
            // return usb_write();

        default:
            break;
    }
    return RES_PARERR;
}

DRESULT ff_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    switch (pdrv) {
        case DEV_RAM:
             return ramdisk_ioctl(pdrv, cmd, buff);

        case DEV_MMC:
            // return sdmmc_ioctl();

        case DEV_USB:
            // return usb_ioctl();

        default:
            break;
    }
    return RES_PARERR;
}
