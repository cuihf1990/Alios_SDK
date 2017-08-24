/* copyright (C) 2017 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "diskio.h"
#include "ff.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAMDISK     0
#define DEV_SDCARD      1

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
        case CTRL_SYNC:
        default:
            return RES_OK;
    }

    return RES_PARERR;
}

DSTATUS ff_disk_status(BYTE pdrv)
{
    switch (pdrv) {
        case DEV_RAMDISK:
             return ramdisk_status();

        case DEV_SDCARD:
            // return sdcard_status();

        default:
            break;
    }
    return STA_NOINIT;
}

DSTATUS ff_disk_initialize(BYTE pdrv)
{
    switch (pdrv) {
        case DEV_RAMDISK:
             return ramdisk_initialize();

        case DEV_SDCARD:
            // return sdcard_initialize();

        default:
            break;
    }
    return STA_NOINIT;
}

DRESULT ff_disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    switch (pdrv) {
        case DEV_RAMDISK:
             return ramdisk_read(pdrv, buff, sector, count);

        case DEV_SDCARD:
            // return sdcard_read();

        default:
            break;
    }
    return RES_PARERR;
}

DRESULT ff_disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    switch (pdrv) {
        case DEV_RAMDISK:
             return ramdisk_write(pdrv, buff, sector, count);

        case DEV_SDCARD:
            // return sdcard_write();

        default:
            break;
    }
    return RES_PARERR;
}

DRESULT ff_disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    switch (pdrv) {
        case DEV_RAMDISK:
             return ramdisk_ioctl(pdrv, cmd, buff);

        case DEV_SDCARD:
            // return sdcard_ioctl();

        default:
            break;
    }
    return RES_PARERR;
}
