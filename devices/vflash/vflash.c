#include <stdio.h>
#include <vfs_driver.h>
#include <hal/soc/flash.h>

static int flash_open(inode_t *node, file_t *file)
{
    return 0;
}

#define SECTOR_SIZE 0x1000 /* 4 K/SECTOR */

static ssize_t flash_write(file_t *f, const void *buf, size_t len)
{
    int pno = (int)(long)f->node->i_arg;
    uint32_t offset = f->offset;
    int ret;
    size_t write_size = (len / SECTOR_SIZE) * SECTOR_SIZE + (len % SECTOR_SIZE? 1: 0) * SECTOR_SIZE;
    void *buffer = (void *)yos_malloc(write_size);
    if (!buffer) {
        return 0;
    }

    ret = hal_flash_read(pno, &offset, buffer, write_size);
    if (ret < 0)
        goto exit;

    memcpy(buffer, buf, len);
    offset = f->offset;

    ret = hal_flash_erase(pno, f->offset, write_size);
    if (ret < 0)
        goto exit;

    ret = hal_flash_write(pno, &f->offset, buffer, write_size);
    if (ret < 0)
        goto exit;
    ret = f->offset - offset;

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

    if (ret < 0)
        return 0;

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
    ret = yunos_register_driver(pname, &flash_fops, (void *)(long)pno);

    return ret;
}
