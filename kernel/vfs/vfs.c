#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/network.h>
#include <vfs_conf.h>
#include <vfs_dirent.h>
#include <vfs_err.h>
#include <vfs_inode.h>
#include <vfs.h>
#include <stdio.h>
#include <hal/soc/uart.h>

extern uart_dev_t uart_0;

static uint8_t    g_vfs_init;

yos_mutex_t g_vfs_mutex;

#ifdef IO_NEED_TRAP
static int trap_open(const char *path, int flags)
{
#ifdef WITH_LWIP
    return E_VFS_K_ERR;
#else
    int fd = open(path, flags);
    if (fd < 0) {
        fd = open(path, O_RDWR | O_CREAT, 0644);
    }
    return fd;
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
    return close(fd);
}

static int trap_fcntl(int fd, int cmd, int val)
{
    return fcntl(fd, cmd, val);
}

#else
static int trap_open(const char *path, int flags)
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

#define MAX_FILE_NUM (YOS_CONFIG_VFS_DEV_NODES * 2)
static file_t files[MAX_FILE_NUM];

static inline int get_fd(file_t *file)
{
    return (file - files) + YOS_CONFIG_VFS_FD_OFFSET;
}

static inline file_t *get_file(int fd)
{
    file_t *f;

    fd -= YOS_CONFIG_VFS_FD_OFFSET;

    if (fd < 0) {
        return NULL;
    }

    if (fd >= MAX_FILE_NUM) {
        return NULL;
    }

    f = &files[fd];
    return f->node ? f : NULL;
}

static file_t *new_file(inode_t *node)
{
    file_t *f;
    int idx;

    for (idx = 0; idx < MAX_FILE_NUM; idx++) {
        f = &files[idx];

        if (f->node == NULL) {
            goto got_file;
        }
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
        return trap_open(path, flags);
    }

    node->i_flags = flags;
    file = new_file(node);

    yos_mutex_unlock(&g_vfs_mutex);

    if (file == NULL) {
        return E_VFS_K_ERR;
    }

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->open) != NULL) {
            err = (node->ops.i_fops->open)(file, path, flags);
        }

    } else {
        if ((node->ops.i_ops->open) != NULL) {
            err = (node->ops.i_ops->open)(node, file);
        }
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

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->close) != NULL) {
            err = (node->ops.i_fops->close)(f);
        }

    } else {

        if ((node->ops.i_ops->close) != NULL) {
            err = (node->ops.i_ops->close)(f);
        }
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

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->read) != NULL) {
            nread = (node->ops.i_fops->read)(f, buf, nbytes);
        }
    } else {
        if ((node->ops.i_ops->read) != NULL) {
            nread = (node->ops.i_ops->read)(f, buf, nbytes);
        }
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

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->write) != NULL) {
            nwrite = (node->ops.i_fops->write)(f, buf, nbytes);
        }
    } else {
        if ((node->ops.i_ops->write) != NULL) {
            nwrite = (node->ops.i_ops->write)(f, buf, nbytes);
        }
    }

    return nwrite;
}

int yos_ioctl(int fd, int cmd, unsigned long arg)
{
    int      err = E_VFS_K_ERR;
    file_t  *f;
    inode_t *node;

    if (fd < 0) {
        return E_VFS_FD_ILLEGAL;
    }

    f = get_file(fd);

    if (f == NULL) {
        return E_VFS_K_ERR;
    }

    node = f->node;

    if ((node->ops.i_ops->ioctl) != NULL) {
        err = (node->ops.i_ops->ioctl)(f, cmd, arg);
    }

    return err;
}

off_t yos_lseek(int fd, off_t offset, int whence)
{
    file_t *f;
    inode_t *node;
    int err = E_VFS_NOSYS;

    f = get_file(fd);

    if (f == NULL) {
        return E_VFS_FD_ILLEGAL;
    }

    node = f->node;

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->lseek) != NULL) {
            err = (node->ops.i_fops->lseek)(f, offset, whence);
        }
    }

    return err;
}

int yos_sync(int fd)
{
    file_t  *f;
    inode_t *node;
    int err = E_VFS_NOSYS;

    f = get_file(fd);

    if (f == NULL) {
        return E_VFS_FD_ILLEGAL;
    }

    node = f->node;

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->sync) != NULL) {
            err = (node->ops.i_fops->sync)(f);
        }
    }

    return err;
}

int yos_stat(const char *path, struct stat *st)
{
    file_t  *file;
    inode_t *node;
    int      err = E_VFS_NOSYS;

    if (path == NULL) {
        return E_VFS_NULL_PTR;
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    node = inode_open(path);

    if (node == NULL) {
        yos_mutex_unlock(&g_vfs_mutex);
        return E_VFS_INODE_NOT_FOUND;
    }

    file = new_file(node);

    yos_mutex_unlock(&g_vfs_mutex);

    if (file == NULL) {
        return E_VFS_K_ERR;
    }

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->stat) != NULL) {
            err = (node->ops.i_fops->stat)(file, path, st);
        }
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    del_file(file);

    yos_mutex_unlock(&g_vfs_mutex);
    return err;
}

int yos_unlink(const char *path)
{
    file_t  *f;
    inode_t *node;
    int      err = E_VFS_NOSYS;

    if (path == NULL) {
        return E_VFS_NULL_PTR;
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    node = inode_open(path);

    if (node == NULL) {
        yos_mutex_unlock(&g_vfs_mutex);
        return E_VFS_INODE_NOT_FOUND;
    }

    f = new_file(node);

    yos_mutex_unlock(&g_vfs_mutex);

    if (f == NULL) {
        return E_VFS_K_ERR;
    }

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->unlink) != NULL) {
            err = (node->ops.i_fops->unlink)(f, path);
        }
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    del_file(f);

    yos_mutex_unlock(&g_vfs_mutex);
    return err;
}

int yos_rename(const char *oldpath, const char *newpath)
{
    file_t  *f;
    inode_t *node;
    int      err = E_VFS_NOSYS;

    if (oldpath == NULL || newpath == NULL) {
        return E_VFS_NULL_PTR;
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    node = inode_open(oldpath);

    if (node == NULL) {
        yos_mutex_unlock(&g_vfs_mutex);
        return E_VFS_INODE_NOT_FOUND;
    }

    f = new_file(node);

    yos_mutex_unlock(&g_vfs_mutex);

    if (f == NULL) {
        return E_VFS_K_ERR;
    }

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->rename) != NULL) {
            err = (node->ops.i_fops->rename)(f, oldpath, newpath);
        }
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    del_file(f);

    yos_mutex_unlock(&g_vfs_mutex);
    return err;
}

yos_dir_t *yos_opendir(const char *path)
{
    file_t  *file;
    inode_t *node;
    yos_dir_t *dp = NULL;

    if (path == NULL) {
        return NULL;
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return NULL;
    }

    node = inode_open(path);

    if (node == NULL) {
        yos_mutex_unlock(&g_vfs_mutex);
        return NULL;
    }

    file = new_file(node);

    yos_mutex_unlock(&g_vfs_mutex);

    if (file == NULL) {
        return NULL;
    }

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->opendir) != NULL) {
            dp = (node->ops.i_fops->opendir)(file, path);
        }
    }

    if (dp == NULL) {
        if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
            return NULL;
        }

        del_file(file);

        yos_mutex_unlock(&g_vfs_mutex);
        return NULL;
    }

    dp->dd_vfs_fd = get_fd(file);
    return dp;
}

int yos_closedir(yos_dir_t *dir)
{
    file_t  *f;
    inode_t *node;
    int      err = E_VFS_NOSYS;

    if (dir == NULL) {
        return E_VFS_ERR_PARAM;
    }

    f = get_file(dir->dd_vfs_fd);

    if (f == NULL) {
        return E_VFS_ERR_PARAM;
    }

    node = f->node;

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->closedir) != NULL) {
            err = (node->ops.i_fops->closedir)(f, dir);
        }
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    del_file(f);

    yos_mutex_unlock(&g_vfs_mutex);

    return err;
}

yos_dirent_t *yos_readdir(yos_dir_t *dir)
{
    file_t *f;
    inode_t *node;
    yos_dirent_t *ret = NULL;

    if (dir == NULL) {
        return NULL;
    }

    f = get_file(dir->dd_vfs_fd);
    if (f == NULL) {
        return NULL;
    }

    node = f->node;

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->readdir) != NULL) {
            ret = (node->ops.i_fops->readdir)(f, dir);
        }
    }

    if (ret != NULL) {
        return ret;
    }

    return NULL;
}

int yos_mkdir(const char *path)
{
    file_t  *file;
    inode_t *node;
    int      err = E_VFS_NOSYS;

    if (path == NULL) {
        return E_VFS_NULL_PTR;
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    node = inode_open(path);

    if (node == NULL) {
        yos_mutex_unlock(&g_vfs_mutex);
        return E_VFS_INODE_NOT_FOUND;
    }

    file = new_file(node);

    yos_mutex_unlock(&g_vfs_mutex);

    if (file == NULL) {
        return E_VFS_K_ERR;
    }

    if (INODE_IS_FS(node)) {
        if ((node->ops.i_fops->mkdir) != NULL) {
            err = (node->ops.i_fops->mkdir)(file, path);
        }
    }

    if (yos_mutex_lock(&g_vfs_mutex, YOS_WAIT_FOREVER) != 0) {
        return E_VFS_K_ERR;
    }

    del_file(file);

    yos_mutex_unlock(&g_vfs_mutex);
    return err;
}


#if (YOS_CONFIG_VFS_POLL_SUPPORT>0)

#if !defined(WITH_LWIP) && defined(VCALL_RHINO)
#define NEED_WAIT_IO
#endif

#include <yos/network.h>

#ifdef NEED_WAIT_IO

#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

struct poll_arg {
    yos_sem_t sem;
};

static void setup_fd(int fd)
{
    int f = fcntl(fd, F_GETFL) | O_ASYNC;
    if (fcntl(fd, F_SETFL, f) < 0) {
        perror("fcntl setup");
    }
    if (fcntl(fd, F_SETOWN, gettid()) < 0) {
        perror("fcntl setown");
    }
}

static void teardown_fd(int fd)
{
    int f = fcntl(fd, F_GETFL) & ~O_ASYNC;
    if (fcntl(fd, F_SETFL, f) < 0) {
        perror("fcntl teardown");
    }
}

static int wait_io(int maxfd, fd_set *rfds, struct poll_arg *parg, int timeout)
{
    struct timeval tv = { 0 };
    int ret;
    fd_set saved_fds = *rfds;

    /* check if already data available */
    ret = select(maxfd + 1, rfds, NULL, NULL, &tv);
    if (ret > 0) {
        return ret;
    }

    timeout = timeout >= 0 ? timeout : YOS_WAIT_FOREVER;
    ret = yos_sem_wait(&parg->sem, timeout);
    if (ret != VFS_SUCCESS) {
        return 0;
    }

    *rfds = saved_fds;
    ret = select(maxfd + 1, rfds, NULL, NULL, &tv);
    return ret;
}

static void vfs_poll_notify(struct pollfd *fd, void *arg)
{
    struct poll_arg *parg = arg;
    yos_sem_signal(&parg->sem);
}

static void vfs_io_cb(int fd, void *arg)
{
    struct poll_arg *parg = arg;
    yos_sem_signal(&parg->sem);
}

void cpu_io_register(void (*f)(int, void *), void *arg);
void cpu_io_unregister(void (*f)(int, void *), void *arg);
static int init_parg(struct poll_arg *parg)
{
    cpu_io_register(vfs_io_cb, parg);
    yos_sem_new(&parg->sem,  0);
    return 0;
}

static void deinit_parg(struct poll_arg *parg)
{
    yos_sem_free(&parg->sem);
    cpu_io_unregister(vfs_io_cb, parg);
}

#else
struct poll_arg {
    int efd;
};

static void vfs_poll_notify(struct pollfd *fd, void *arg)
{
    struct poll_arg *parg = arg;
    uint64_t val = 1;
    write(parg->efd, &val, sizeof val);
}

static void setup_fd(int fd)
{
}

static void teardown_fd(int fd)
{
}

static int init_parg(struct poll_arg *parg)
{
    int efd;
    efd = eventfd(0, 0);

    if (efd < 0) {
        return -1;
    }

    parg->efd = efd;

    return 0;
}

static void deinit_parg(struct poll_arg *parg)
{
    close(parg->efd);
}

static int wait_io(int maxfd, fd_set *rfds, struct poll_arg *parg, int timeout)
{
    struct timeval tv = {
        .tv_sec  = timeout / 1024,
        .tv_usec = (timeout % 1024) * 1024,
    };

    FD_SET(parg->efd, rfds);
    maxfd = parg->efd > maxfd ? parg->efd : maxfd;
    return select(maxfd + 1, rfds, NULL, NULL, timeout >= 0 ? &tv : NULL);
}
#endif

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

        if (pfd->fd < YOS_CONFIG_VFS_FD_OFFSET) {
            setup_fd(pfd->fd);
            FD_SET(pfd->fd, rfds);
            maxfd = pfd->fd > maxfd ? pfd->fd : maxfd;
            continue;
        }

        f = get_file(pfd->fd);

        if (f == NULL) {
            return -1;
        }

        pfd = &fds[i];
        (f->node->ops.i_ops->poll)(f, true, vfs_poll_notify, pfd, parg);
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

        if (pfd->fd < YOS_CONFIG_VFS_FD_OFFSET) {
            teardown_fd(pfd->fd);
            continue;
        }


        f = get_file(pfd->fd);

        if (f == NULL) {
            continue;
        }

        (f->node->ops.i_ops->poll)(f, false, NULL, NULL, NULL);

        if (pfd->revents) {
            ret ++;
        }
    }

    return ret;
}

int yos_poll(struct pollfd *fds, int nfds, int timeout)
{
    fd_set rfds;

    int ret = VFS_SUCCESS;
    int nset = 0;
    struct poll_arg parg;

    if (init_parg(&parg) < 0) {
        return -1;
    }

    FD_ZERO(&rfds);
    ret = pre_poll(fds, nfds, &rfds, &parg);

    if (ret < 0) {
        goto check_poll;
    }

    ret = wait_io(ret, &rfds, &parg, timeout);

    if (ret >= 0) {
        int i;

        for (i = 0; i < nfds; i++) {
            struct pollfd *pfd = fds + i;

            if (FD_ISSET(pfd->fd, &rfds)) {
                pfd->revents |= POLLIN;
            }
        }

        nset += ret;
    }

check_poll:
    nset += post_poll(fds, nfds);

    deinit_parg(&parg);

    return ret < 0 ? 0 : nset;
}
#endif

int yos_fcntl(int fd, int cmd, int val)
{
    if (fd < 0) {
        return E_VFS_ERR_PARAM;
    }

    if (fd < YOS_CONFIG_VFS_FD_OFFSET) {
        return trap_fcntl(fd, cmd, val);
    }

    return 0;
}

int yos_ioctl_in_loop(int cmd, unsigned long arg)
{
    int      err;
    int      fd;

    for (fd = YOS_CONFIG_VFS_FD_OFFSET;
         fd < YOS_CONFIG_VFS_FD_OFFSET + YOS_CONFIG_VFS_DEV_NODES; fd++) {
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

        if ((node->ops.i_ops->ioctl) != NULL) {
            err = (node->ops.i_ops->ioctl)(f, cmd, arg);

            if (err != VFS_SUCCESS) {
                return err;
            }
        }
    }

    return VFS_SUCCESS;
}

int32_t yos_uart_send(void *data, uint32_t size, uint32_t timeout)
{
    return hal_uart_send(&uart_0, data, size, timeout);
}

