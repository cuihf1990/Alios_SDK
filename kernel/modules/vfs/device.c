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
#include <yoc/list.h>
#include <csp.h>

#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_driver.h>

extern void *yos_malloc(size_t size);
extern void yos_free(void *mem);
#if (YUNOS_CONFIG_VFS_POLL_SUPPORT > 0)
typedef struct {
    csp_mutex_t    mutex;
    struct pollfd *fd;
    void          *sem;
    int            counter;
} signal_dev_t;

static signal_dev_t sdev;
static int inited;

static int signal_open(inode_t *node)
{
    return 0;
}

static ssize_t signal_read(inode_t *node, char *buf, size_t nbytes)
{
    csp_mutex_lock(sdev.mutex);

    int cnt = sdev.counter;
    sdev.counter = 0;

    csp_mutex_unlock(sdev.mutex);
    return cnt;
}

static ssize_t signal_write(inode_t *node, const char *buf, size_t len)
{
    csp_mutex_lock(sdev.mutex);

    sdev.counter += len;

    if (sdev.fd) {
        sdev.fd->revents |= POLLIN;

        if (sdev.sem != NULL) {
            csp_sem_t csp_sem;
            csp_sem.hdl = sdev.sem;
            csp_sem_signal(csp_sem);
        }
    }

    csp_mutex_unlock(sdev.mutex);
    return len;
}

static int signal_poll(bool setup, struct pollfd *pfd, void *sem)
{
    if (!setup) {
        sdev.fd  = NULL;
        sdev.sem = NULL;
        return 0;
    }

    sdev.fd = pfd;
    sdev.sem = sem;

    if (sdev.counter) {
        pfd->revents |= POLLIN;
        csp_sem_t csp_sem;
        csp_sem.hdl = sem;
        csp_sem_signal(csp_sem);
    }

    return 0;
}

static file_ops_t signal_fops = {
    .open  = signal_open,
    .close = NULL,
    .read  = signal_read,
    .write = signal_write,
    .ioctl = NULL,
    .poll  = signal_poll
};

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
    int len;
    char buf[];
} dev_event_t;

static event_dev_t edev = {
    .bufs = YOC_DLIST_INIT(edev.bufs),
    .buf_cache = YOC_DLIST_INIT(edev.buf_cache),
};

static int event_open(inode_t *node)
{
    return 0;
}

static ssize_t event_write(inode_t *node, const char *buf, size_t len)
{
    csp_mutex_lock(edev.mutex);

    dev_event_t *evt;
    evt = (dev_event_t *)edev.buf_cache.next;

    if (edev.cache_count > 0 && evt->len == len) {
        dlist_del(&evt->node);
        edev.cache_count --;
    } else {
        evt = yos_malloc(sizeof(*evt) + len);
    }

    if (evt == NULL) {
        len = -1;
        goto out;
    }

    edev.counter ++;

    evt->len = len;
    memcpy(evt->buf, buf, len);
    dlist_add_tail(&evt->node, &edev.bufs);

    if (edev.fd) {
        edev.fd->revents |= POLLIN;

        if (edev.sem != NULL) {
            csp_sem_t csp_sem;
            csp_sem.hdl = edev.sem;
            csp_sem_signal(csp_sem);
        }
    }

out:
    csp_mutex_unlock(edev.mutex);
    return len;
}

static ssize_t event_read(inode_t *node, char *buf, size_t len)
{
    int cnt = edev.counter;

    if (!cnt) {
        return 0;
    }

    csp_mutex_lock(edev.mutex);

    dev_event_t *evt = (dev_event_t *)edev.bufs.next;
    dlist_del(&evt->node);
    cnt = (len > evt->len) ? evt->len : len;
    memcpy(buf, evt->buf, cnt);

    if (edev.cache_count < 4) {
        dlist_add(&evt->node, &edev.buf_cache);
        edev.cache_count ++;
    } else {
        yos_free(evt);
    }

    edev.counter --;

    csp_mutex_unlock(edev.mutex);

    return cnt;
}

static int event_poll(bool setup, struct pollfd *pfd, void *sem)
{
    if (!setup) {
        edev.fd = NULL;
        edev.sem = NULL;
        return 0;
    }

    edev.fd = pfd;
    edev.sem = sem;

    if (edev.counter) {
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
};

int vfs_device_init(void)
{
    int ret;

    if (inited == 1) {
        return  VFS_SUCCESS;
    }

    ret = csp_mutex_new(&sdev.mutex);
    ret |= csp_mutex_new(&edev.mutex);
    if (ret)
        return E_VFS_NO_MEM;

    ret = yunos_register_driver("/dev/signal", &signal_fops, NULL);

    if (ret != VFS_SUCCESS) {
        return ret;
    }

    ret = yunos_register_driver("/dev/event", &event_fops, NULL);

    if (ret != VFS_SUCCESS) {
        return ret;
    }

    inited = 1;

    return VFS_SUCCESS;
}
#endif

