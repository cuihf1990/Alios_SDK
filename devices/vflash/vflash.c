#include <stdio.h>
#include <vfs_driver.h>

static int flash_open(inode_t *node, file_t *file)
{
    return 0;
}

static ssize_t flash_write(file_t *f, const void *buf, size_t len)
{
    int pno = (int)(long)f->node->i_arg;
    uint32_t offset = f->offset;
    int ret;

    ret = hal_flash_write(pno, &f->offset, buf, len);

    if (ret < 0)
        return 0;

    return f->offset - offset;
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
