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

#include <csp.h>
#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_inode.h>
#include <vfs.h>

static uint8_t    g_vfs_init;

csp_mutex_t g_vfs_mutex;

#ifdef IO_NEED_TRAP
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
static int trap_open(const char *path)
{
#ifdef WITH_LWIP
    return E_VFS_K_ERR;
#else
    return open(path, O_RDWR);
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

    if (csp_mutex_new(&g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    inode_init();

    g_vfs_init = 1;

    return VFS_SUCCESS;
}

int yunos_open(const char *path, int flags)
{
    int      fd;
    inode_t *node;
    int      err = VFS_SUCCESS;

    if (path == NULL) {
        return E_VFS_NULL_PTR;
    }

    if (csp_mutex_lock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    fd = inode_open(path);

    if (fd  ==  E_VFS_INODE_NOT_FOUND) {
        csp_mutex_unlock(g_vfs_mutex);
        return trap_open(path);
    }

    inode_ref(fd);
    inode_ptr_get(fd, &node);
    node->i_flags = flags;

    if (csp_mutex_unlock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    if ((node->ops->open) != NULL) {
        err = node->ops->open(node);
    }
    return (err == VFS_SUCCESS)?fd:err;
}

static int pseudo_ops_check(int fd, inode_t **node)
{
    if ((fd < YUNOS_CONFIG_VFS_FD_OFFSET) || (fd >= YUNOS_CONFIG_VFS_FD_OFFSET + YUNOS_CONFIG_VFS_DEV_NODES)) {
        return E_VFS_FD_ILLEGAL;
    }

    inode_ptr_get(fd, node);

    if ((*node)->type == VFS_TYPE_NOT_INIT) {
        return E_VFS_INODE_NOT_INIT;
    }

    /* not support other dev */
    if ((*node)->type != VFS_TYPE_CHAR_DEV) {
        return E_VFS_INODE_TYPE_ILLEAGL;
    }

    return VFS_SUCCESS;
}

int yunos_close(int fd)
{
    int      err = VFS_SUCCESS;
    inode_t *node;

    if (fd < 0) {
        return E_VFS_ERR_PARAM;
    }

    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET) {
        return trap_close(fd);
    }

    if (csp_mutex_lock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    err = pseudo_ops_check(fd, &node);

    if (err != VFS_SUCCESS) {
        csp_mutex_unlock(g_vfs_mutex);
        return err;
    }

    if (csp_mutex_unlock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    if ((node->ops->close) != NULL) {
        err = (node->ops->close)(node);
    }

    inode_unref(fd);

    return err;
}

ssize_t yunos_read(int fd, void *buf, size_t nbytes)
{
    inode_t *node  = NULL;
    ssize_t  nread = 0;
    int      err   = VFS_SUCCESS;

    if (fd < 0) {
        return E_VFS_ERR_PARAM;
    }

    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET) {
        return trap_read(fd, buf, nbytes);
    }

    if (csp_mutex_lock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    err = pseudo_ops_check(fd, &node);

    if (err != VFS_SUCCESS) {
        csp_mutex_unlock(g_vfs_mutex);
        return err;
    }

    if (csp_mutex_unlock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    if ((node->ops->read) != NULL) {
        nread = (node->ops->read)(node, (char *)buf, nbytes);
    }

    return nread;
}

ssize_t yunos_write(int fd, const void *buf, size_t nbytes)
{
    inode_t *node   = NULL;
    ssize_t  nwrite = 0;
    int      err    = VFS_SUCCESS;

    if (fd < 0) {
        return E_VFS_ERR_PARAM;
    }

    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET) {
        return trap_write(fd, buf, nbytes);
    }

    if (csp_mutex_lock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    err = pseudo_ops_check(fd, &node);

    if (err != VFS_SUCCESS) {
        csp_mutex_unlock(g_vfs_mutex);
        return err;
    }

    if (csp_mutex_unlock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    if ((node->ops->write) != NULL) {
        nwrite = (node->ops->write)(node, (const char *)buf, nbytes);
    }

    return nwrite;
}

int yunos_ioctl(int fd, int cmd, unsigned long arg)
{
    inode_t *node;
    int      err;

    if (csp_mutex_lock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    err = pseudo_ops_check(fd, &node);

    if (err != VFS_SUCCESS) {
        csp_mutex_unlock(g_vfs_mutex);
        return err;
    }

    if (csp_mutex_unlock(g_vfs_mutex) != 0) {
        return E_VFS_K_ERR;
    }

    if ((node->ops->ioctl) != NULL) {
        err = (node->ops->ioctl)(node, cmd, arg);
    }

    return err;
}

#if (YUNOS_CONFIG_VFS_POLL_SUPPORT>0)
int yunos_poll(struct pollfd *fds, int nfds, int timeout)
{
    csp_sem_t sem;
    inode_t  *node = NULL;
    int       ret = VFS_SUCCESS;
    int       i;
    int       j;
    int       succeed = 1;
    int       n_i_fds = 0;

    csp_sem_new(&sem, 0);

    for (i = 0; i < nfds; i++) {
        struct pollfd *pfd = &fds[i];

        pfd->revents = 0;
    }

    for (i = 0; i < nfds; i++) {
        struct pollfd *pfd = &fds[i];

        if (pfd->fd < YUNOS_CONFIG_VFS_FD_OFFSET) {
            succeed = 2;
            continue;
        }

        ret = inode_ptr_get(pfd->fd, &node);

        if (ret != VFS_SUCCESS) {
            succeed = 0;
            break;
        }

        if (i != n_i_fds) {
            struct pollfd tmp = fds[n_i_fds];
            fds[n_i_fds] = fds[i];
            fds[i] = tmp;
        }

        pfd = &fds[n_i_fds];
        (node->ops->poll)(node, true, pfd, sem.hdl);
        n_i_fds ++;
    }

    if (!timeout)
    { goto check_poll; }

    if (succeed == 1) {
        ret = csp_sem_wait(sem, timeout);
    } else if (succeed == 2) {
        ret = csp_poll(fds + n_i_fds, nfds - n_i_fds, sem, timeout);
    }

check_poll:

    for (j = 0; j < i; j++) {
        struct pollfd *pfd = &fds[j];

        if (pfd->fd < YUNOS_CONFIG_VFS_FD_OFFSET)
        { continue; }


        inode_ptr_get(pfd->fd, &node);

        (node->ops->poll)(node, false, pfd, sem.hdl);

        if (pfd->revents) {
            ret ++;
        }
    }

    csp_sem_free(&sem);

    return ret < 0 ? 0 : ret;
}
#endif

int yunos_fcntl(int fd, int cmd, int val)
{
    if (fd < 0) {
        return E_VFS_ERR_PARAM;
    }

    if (fd < YUNOS_CONFIG_VFS_FD_OFFSET)
    { return trap_fcntl(fd, cmd, val); }

    return 0;
}

int yunos_ioctl_in_loop(int cmd, unsigned long arg)
{
    inode_t *node;
    int      err;
    int      fd;

    for (fd = YUNOS_CONFIG_VFS_FD_OFFSET; fd < YUNOS_CONFIG_VFS_FD_OFFSET + YUNOS_CONFIG_VFS_DEV_NODES; fd++) {
        if (csp_mutex_lock(g_vfs_mutex) != 0) {
            return E_VFS_K_ERR;
        }

        err = pseudo_ops_check(fd, &node);

        if (err != VFS_SUCCESS) {
            csp_mutex_unlock(g_vfs_mutex);
            return err;
        }

        if (csp_mutex_unlock(g_vfs_mutex) != 0) {
            return E_VFS_K_ERR;
        }

        if ((node->ops->ioctl) != NULL) {
            err = (node->ops->ioctl)(node, cmd, arg);

            if (err != VFS_SUCCESS) {
                return err;
            }
        }
    }

    return VFS_SUCCESS;
}

