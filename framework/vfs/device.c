/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <csp.h>
#include <yos/list.h>
#include <csp.h>

#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_driver.h>
#include <event_device.h>

#if (YUNOS_CONFIG_VFS_POLL_SUPPORT > 0)
static int inited;

typedef struct {
    csp_mutex_t    mutex;
    struct pollfd *fd;
    void          *sem;
    int            counter;
    dlist_t        bufs;
    int            cache_count;
    dlist_t        buf_cache;
} event_dev_t;

typedef struct {
    dlist_t node;
    size_t len;
    char buf[];
} dev_event_t;

static int event_open(inode_t *node, file_t *file)
{
    event_dev_t *pdev = (event_dev_t *)yos_malloc(sizeof *pdev);
    bzero(pdev, sizeof *pdev);
    csp_mutex_new(&pdev->mutex);
    dlist_init(&pdev->bufs);
    dlist_init(&pdev->buf_cache);
    file->f_arg = pdev;
    return 0;
}

static ssize_t _event_write(file_t *f, const void *buf, size_t len, bool urgent)
{
    event_dev_t *pdev = f->f_arg;
    csp_mutex_lock(pdev->mutex);

    dev_event_t *evt;
    evt = (dev_event_t *)pdev->buf_cache.next;

    if (pdev->cache_count > 0 && evt->len == len) {
        dlist_del(&evt->node);
        pdev->cache_count --;
    } else {
        evt = (dev_event_t*)yos_malloc(sizeof(*evt) + len);
    }

    if (evt == NULL) {
        len = -1;
        goto out;
    }

    pdev->counter ++;

    evt->len = len;
    memcpy(evt->buf, buf, len);
    if (urgent)
        dlist_add(&evt->node, &pdev->bufs);
    else
        dlist_add_tail(&evt->node, &pdev->bufs);

    if (pdev->fd) {
        pdev->fd->revents |= POLLIN;

        if (pdev->sem != NULL) {
            csp_sem_t csp_sem;
            csp_sem.hdl = pdev->sem;
            csp_sem_signal(csp_sem);
        }
    }

out:
    csp_mutex_unlock(pdev->mutex);
    return len;
}

static ssize_t event_write(file_t *f, const void *buf, size_t len)
{
    return _event_write(f, buf, len, false);
}

static int event_ioctl(file_t *f, int cmd, unsigned long arg)
{
    int len = _GET_LEN(cmd);
    void *buf = (void *)arg;
    cmd = _GET_CMD(cmd);
    switch (cmd) {
    case IOCTL_WRITE_NORMAL:
        return _event_write(f, buf, len, false);
    case IOCTL_WRITE_URGENT:
        return _event_write(f, buf, len, true);
    }

    return -1;
}

static ssize_t event_read(file_t *f, void *buf, size_t len)
{
    event_dev_t *pdev = f->f_arg;
    int cnt = pdev->counter;

    if (!cnt) {
        return 0;
    }

    csp_mutex_lock(pdev->mutex);

    dev_event_t *evt = (dev_event_t *)pdev->bufs.next;
    dlist_del(&evt->node);
    cnt = (len > evt->len) ? evt->len : len;
    memcpy(buf, evt->buf, cnt);

    if (pdev->cache_count < 4) {
        dlist_add(&evt->node, &pdev->buf_cache);
        pdev->cache_count ++;
    } else {
        yos_free(evt);
    }

    pdev->counter --;

    csp_mutex_unlock(pdev->mutex);

    return cnt;
}

static int event_poll(file_t *f, bool setup, struct pollfd *pfd, void *sem)
{
    event_dev_t *pdev = f->f_arg;
    if (!setup) {
        pdev->fd = NULL;
        pdev->sem = NULL;
        return 0;
    }

    pdev->fd = pfd;
    pdev->sem = sem;

    if (pdev->counter) {
        pfd->revents |= POLLIN;
        csp_sem_t csp_sem;
        csp_sem.hdl = sem;
        csp_sem_signal(csp_sem);
    }

    return 0;
}

static file_ops_t event_fops = {
    .open = event_open,
    .read = event_read,
    .write = event_write,
    .poll = event_poll,
    .ioctl = event_ioctl,
};

int vfs_device_init(void)
{
    int ret;

    if (inited == 1) {
        return  VFS_SUCCESS;
    }

    ret = yunos_register_driver("/dev/event", &event_fops, NULL);

    if (ret != VFS_SUCCESS) {
        return ret;
    }

    inited = 1;

    return VFS_SUCCESS;
}
#endif

