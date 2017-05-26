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

#include <yos/kernel.h>
#include <yos/framework.h>
#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_inode.h>
#include <vfs.h>
#include <stdio.h>

static uint8_t    g_vfs_init;

yos_mutex_t g_vfs_mutex;

#ifdef IO_NEED_TRAP
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
static int trap_open(const char *path)
{
#ifdef WITH_LWIP
    return E_VFS_K_ERR;
#else
    return open(path, O_RDWR | O_CREAT, 0644);
#endif
}

static int trap_read(int fd, void *buf, int len)
{
    return read(fd, buf, len);
}

static int trap_write(int fd, const void *buf, int len)
{
    return write(fd, buf, len);
}

static int trap_close(int fd)
{
    close(fd);
    return VFS_SUCCESS;
}

static int trap_fcntl(int fd, int cmd, int val)
{
    return fcntl(fd, cmd, val);
}

#else
static int trap_open(const char *path)
{
    return E_VFS_K_ERR;
}

static int trap_read(int fd, void *buf, int len)
{
    return E_VFS_K_ERR;
}

static int trap_write(int fd, const void *buf, int len)
{
    return E_VFS_K_ERR;
}

static int trap_close(int fd)
{
    return E_VFS_K_ERR;
}

static int trap_fcntl(int fd, int cmd, int val)
{
    return E_VFS_K_ERR;
}
#endif


int vfs_init(void)
{
    if (g_vfs_init == 1) {
        return VFS_SUCCESS;
    }

    if (yos_mutex_new(&g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    inode_init();

    g_vfs_init = 1;

    return VFS_SUCCESS;
}

#define MAX_FILE_NUM (YUNOS_CONFIG_VFS_DEV_NODES * 2)
static file_t files[MAX_FILE_NUM];

static inline int get_fd(file_t *file)
{
    return (file - files) + YUNOS_CONFIG_VFS_FD_OFFSET;
}

static inline file_t *get_file(int fd)
{
    file_t *f;

    fd -= YUNOS_CONFIG_VFS_FD_OFFSET;
    if (fd < 0)
        return NULL;

    if (fd >= MAX_FILE_NUM)
        return NULL;

    f = &files[fd];
    return f->node ? f : NULL;
}

static file_t *new_file(inode_t *node)
{
    file_t *f;
    int idx;
    for (idx=0;idx<MAX_FILE_NUM;idx++) {
        f = &files[idx];
        if (f->node == NULL)
            goto got_file;
    }

    return NULL;

got_file:
    f->node = node;
    f->f_arg = NULL;
    f->offset = 0;
    inode_ref(node);
    return f;
}

static void del_file(file_t *file)
{
    inode_unref(file->node);
    file->node = NULL;
}

int yos_open(const char *path, int flags)
{
    file_t  *file;
    inode_t *node;
    int      err = VFS_SUCCESS;

    if (path == NULL) {
        return E_VFS_NULL_PTR;
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    node = inode_open(path);
    if (node == NULL) {
        yos_mutex_unlock(&g_vfs_mutex);
        return trap_open(path);
    }

    node->i_flags = flags;
    file = new_file(node);

    yos_mutex_unlock(&g_vfs_mutex);

    if (file == NULL) {
        return E_VFS_K_ERR;
    }

    if ((node->ops->open) != NULL) {
        err = node->ops->open(node, file);
    }

    if (err != VFS_SUCCESS) {
        del_file(file);
        return err;
    }

    return get_fd(file);
}

int yos_close(int fd)
{
    int      err = VFS_SUCCESS;
    file_t  *f;
    inode_t *node;

    f = get_file(fd);
    if (f == NULL) {
        return trap_close(fd);
    }

    node = f->node;
    if ((node->ops->close) != NULL) {
        (node->ops->close)(f);
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    del_file(f);

    yos_mutex_unlock(&g_vfs_mutex);

    return err;
}

ssize_t yos_read(int fd, void *buf, size_t nbytes)
{
    ssize_t  nread = -1;
    file_t  *f;
    inode_t *node;

    f = get_file(fd);
    if (f == NULL) {
        return trap_read(fd, buf, nbytes);
    }

    node = f->node;
    if ((node->ops->read) != NULL) {
        nread = (node->ops->read)(f, buf, nbytes);
    }

    return nread;
}

ssize_t yos_write(int fd, const void *buf, size_t nbytes)
{
    ssize_t  nwrite = -1;
    file_t  *f;
    inode_t *node;

    f = get_file(fd);
    if (f == NULL) {
        return trap_write(fd, buf, nbytes);
    }

    node = f->node;
    if ((node->ops->write) != NULL) {
        nwrite = (node->ops->write)(f, buf, nbytes);
    }

    return nwrite;
}

int yos_ioctl(int fd, int cmd, unsigned long arg)
{
    int      err = E_VFS_K_ERR;
    file_t  *f;
    inode_t *node;

    if (fd < 0)
        return E_VFS_FD_ILLEGAL;

    f = get_file(fd);
    if (f == NULL) {
        return E_VFS_K_ERR;
    }

    node = f->node;
    if ((node->ops->ioctl) != NULL) {
        err = (node->ops->ioctl)(f, cmd, arg);
    }

    return err;
}

#if (YUNOS_CONFIG_VFS_POLL_SUPPORT>0)
#include <yos/network.h>

struct poll_arg {
    int efd;
};

static void vfs_poll_notify(struct pollfd *fd, void *arg)
{
    struct poll_arg *parg = arg;
    uint64_t val = 1;
    write(parg->efd, &val, sizeof val);
}

static int pre_poll(struct pollfd *fds, int nfds, fd_set *rfds, void *parg)
{
    int i;
    int maxfd = 0;

    for (i = 0; i < nfds; i++) {
        struct pollfd *pfd = &fds[i];

        pfd->revents = 0;
    }

    for (i = 0; i < nfds; i++) {
        file_t  *f;
        struct pollfd *pfd = &fds[i];

        if (pfd->fd < YUNOS_CONFIG_VFS_FD_OFFSET) {
            FD_SET(pfd->fd, rfds);
            maxfd = pfd->fd > maxfd ? pfd->fd : maxfd;
            continue;
        }

        f = get_file(pfd->fd);

        if (f == NULL) {
            return -1;
        }

        pfd = &fds[i];
        (f->node->ops->poll)(f, true, vfs_poll_notify, pfd, parg);
    }

    return maxfd;
}

static int post_poll(struct pollfd *fds, int nfds)
{
    int j;
    int ret = 0;
    for (j = 0; j < nfds; j++) {
        file_t  *f;
        struct pollfd *pfd = &fds[j];

        if (pfd->fd < YUNOS_CONFIG_VFS_FD_OFFSET)
        { continue; }


        f = get_file(pfd->fd);
        if (f == NULL) {
            continue;
        }

        (f->node->ops->poll)(f, false, NULL, NULL, NULL);

        if (pfd->revents) {
            ret ++;
        }
    }

    return ret;
}

int yos_poll(struct pollfd *fds, int nfds, int timeout)
{
    struct timeval tv = {
        .tv_sec  = timeout / 1024,
        .tv_usec = (timeout % 1024) * 1024,
    };

    fd_set rfds;

    int ret = VFS_SUCCESS;
    int nset = 0;
    int maxfd = 0;
    int efd;
    struct poll_arg parg;

    efd = eventfd(0, 0);
    if (efd < 0) {
        return -1;
    }

    parg.efd = efd;
    FD_ZERO(&rfds);
    FD_SET(efd, &rfds);
    ret = pre_poll(fds, nfds, &rfds, &parg);
    if (ret < 0) {
        goto check_poll;
    }

    maxfd = ret > efd ? ret : efd;
    ret = select(maxfd + 1, &rfds, NULL, NULL, timeout >= 0 ? &tv : NULL);
    if (ret >= 0) {
        int i;
        for (i=0;i<nfds;i++) {
            struct pollfd *pfd = fds + i;
            if (FD_ISSET(pfd->fd, &rfds))
                pfd->revents |= POLLIN;
        }

        nset += ret;
    }

check_poll:
    nset += post_poll(fds, nfds);

    close(efd);

    return ret < 0 ? 0 : nset;
}
#endif

int yos_fcntl(int fd, int cmd, int val)
{
    if (fd < 0) {
        return E_VFS_ERR_PARAM;
    }

    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET)
    { return trap_fcntl(fd, cmd, val); }

    return 0;
}

int yos_ioctl_in_loop(int cmd, unsigned long arg)
{
    int      err;
    int      fd;

    for (fd = YUNOS_CONFIG_VFS_FD_OFFSET; fd < YUNOS_CONFIG_VFS_FD_OFFSET + YUNOS_CONFIG_VFS_DEV_NODES; fd++) {
        file_t  *f;
        inode_t *node;
        if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
            return E_VFS_K_ERR;
        }

        f = get_file(fd);

        if (f == NULL) {
            yos_mutex_unlock(&g_vfs_mutex);
            return err;
        }

        if (yos_mutex_unlock(&g_vfs_mutex) != 0) {
            return E_VFS_K_ERR;
        }

        node = f->node;
        if ((node->ops->ioctl) != NULL) {
            err = (node->ops->ioctl)(f, cmd, arg);

            if (err != VFS_SUCCESS) {
                return err;
            }
        }
    }

    return VFS_SUCCESS;
}

