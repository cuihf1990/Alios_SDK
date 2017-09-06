/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <vfs_register.h>
#include <hal/soc/flash.h>
#include <yos/yos.h>

static int flash_open(inode_t *node, file_t *file)
{
    return 0;
}

#define SECTOR_SIZE     0x1000      /* sector size: 4k */
#define SECTOR_MASK     ~(SECTOR_SIZE - 1)

static ssize_t flash_write(file_t *f, const void *buf, size_t len)
{
    int pno = (int)(long)f->node->i_arg;
    uint32_t offset, sector_off;
    int ret;
    size_t write_size;

    sector_off = f->offset & SECTOR_MASK;
    write_size = (f->offset - sector_off + len + (~SECTOR_MASK)) & SECTOR_MASK;

    void *buffer = (void *)yos_malloc(write_size);
    if (!buffer) {
        return 0;
    }
    memset(buffer, 0, write_size);

    offset = sector_off;
    ret = hal_flash_read(pno, &offset, buffer, write_size);
    if (ret < 0) {
        goto exit;
    }

    memcpy(buffer + (f->offset) - sector_off, buf, len);

    offset = sector_off;
    ret = hal_flash_erase(pno, offset, write_size);
    if (ret < 0) {
        goto exit;
    }

    ret = hal_flash_write(pno, &offset, buffer, write_size);
    if (ret < 0) {
        goto exit;
    }

    if ((offset - sector_off) == write_size) {
        f->offset += len;
        ret = len;
    }

exit:
    yos_free(buffer);
    return ret;
}

static ssize_t flash_read(file_t *f, void *buf, size_t len)
{
    int pno = (int)(long)f->node->i_arg;
    uint32_t offset = f->offset;
    int ret;

    ret = hal_flash_read(pno, &f->offset, buf, len);

    if (ret < 0) {
        return 0;
    }

    return f->offset - offset;
}

static file_ops_t flash_fops = {
    .open = flash_open,
    .read = flash_read,
    .write = flash_write,
};

int vflash_register_partition(int pno)
{
    char pname[32];
    int ret;

    snprintf(pname, sizeof(pname) - 1, "/dev/flash%d", pno);
    ret = yos_register_driver(pname, &flash_fops, (void *)(long)pno);

    return ret;
}

