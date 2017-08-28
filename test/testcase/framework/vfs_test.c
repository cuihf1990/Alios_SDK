#include <stdio.h>
#include <unistd.h>

#include <yos/kernel.h>
#include <yos/framework.h>
#include <yos/network.h>

#include "vfs.h"
#include "vfs_inode.h"
#include "vfs_register.h"
#include "vfs_err.h"

#include "yunit.h"

static int test_ioctl(file_t *node, int cmd, unsigned long arg)
{
    return -123;
}

static off_t test_lseek(file_t *fp, off_t off, int whence)
{
    return -123;
}

static int test_sync(file_t *fp) 
{
    return -123;
}

static int test_stat(file_t *fp, const char *path, struct stat *st)
{
    return -123;
}

static int test_unlink(file_t *fp, const char *path)
{
    return -123;
}

static int test_rename(file_t *fp, const char *oldpath, const char *newpath)
{
    return -123;
}

static int test_mkdir(file_t *fp, const char *path)
{
    return -123;
}

static void test_yos_vfs_case(void)
{
    int i   = 0;
    int fd  = 0;
    int ret = 0;

    char *names[] = {
        "/tmp/abcd0",
        "/tmp/abcd1",
        "/tmp/abcd2",
        "/tmp/abcd3",
        "/tmp/abcd4",
        "/tmp/abcd5",
        "/tmp/abcd6",
        "/tmp/abcd7",
        "/tmp/abcd8",
        "/tmp/abcd9",
    };

    file_ops_t myops = {
        .open  = NULL,
        .ioctl = test_ioctl,
    };

    for (i = 0; i < 10; i++) {
        ret = yos_register_driver(names[i], &myops, NULL);
        YUNIT_ASSERT(ret == VFS_SUCCESS);
    }

    for (i = 0; i < 10; i++) {
        fd = yos_open(names[i], 0);
        YUNIT_ASSERT(fd >= 0);
        YUNIT_ASSERT(-123 == yos_ioctl(fd, 0, 0));

        yos_close(fd);
    }

    for (i = 0; i < 10; i++) {
        fd = yos_open(names[i], 0);
        YUNIT_ASSERT(fd >= 0);

        yos_close(fd);

        ret = yos_unregister_driver(names[i]);
        YUNIT_ASSERT(ret == 0);

        fd = yos_open(names[i], 0);
        ret = yos_ioctl(fd, 0, 0);
        YUNIT_ASSERT(E_VFS_K_ERR == ret);
    }
}

static void test_vfs_fs_case(void)
{
    int i   = 0;
    int fd  = 0;
    int ret = 0;
    struct stat st;
    yos_dir_t dir;
    char *names = "/tmp/abcd0";

    fs_ops_t myops = {
        .open       = NULL,
        .lseek      = test_lseek,
        .sync       = test_sync,
        .stat       = test_stat,
        .unlink     = test_unlink,
        .rename     = test_rename,
        .mkdir      = test_mkdir,
    };

    ret = yos_register_fs(names, &myops, NULL);
    YUNIT_ASSERT(ret == VFS_SUCCESS);

    fd = yos_open(names, 0);
    YUNIT_ASSERT(fd >= 0);
    
    YUNIT_ASSERT(-123 == yos_lseek(fd, 0, 0));
    YUNIT_ASSERT(-123 == yos_sync(fd));
    yos_close(fd);

    YUNIT_ASSERT(-123 == yos_stat(names, &st));
    YUNIT_ASSERT(-123 == yos_unlink(names));
    YUNIT_ASSERT(-123 == yos_rename(names, names));
    YUNIT_ASSERT(-123 == yos_mkdir(names));

    ret = yos_unregister_fs(names);
    YUNIT_ASSERT(ret == 0);

    YUNIT_ASSERT(E_VFS_INODE_NOT_FOUND == yos_stat(names, &st));
    YUNIT_ASSERT(E_VFS_INODE_NOT_FOUND == yos_unlink(names));
    YUNIT_ASSERT(E_VFS_INODE_NOT_FOUND == yos_rename(names, names));
}


static int create_socket(int port)
{
    struct sockaddr_in my_addr;
    int ret = -1;
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        goto out;

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &(int){1}, sizeof(int));

    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);
    ret = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0)
        goto out1;

    return sockfd;
out1:
    close(sockfd);
out:
    return -1;
}

static int do_poll(int fd_recv, int timeout)
{
    int ret;
    struct pollfd pfd;
    char buf2[256];

    pfd.fd = fd_recv;
    pfd.events = POLLIN;
    ret = yos_poll(&pfd, 1, timeout);

    if (ret > 0)
        ret = recv(fd_recv, buf2, sizeof buf2, 0);

    return ret;
}

#define MAXCNT 100
static void send_seq_data(void *arg)
{
    int fd = *(int *)arg;
    struct sockaddr_in addr;
    char buf[MAXCNT]={0};

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12346);

    int i;
    for (i=1;i<MAXCNT;i++) {
        int ret = sendto(fd, buf, i, 0, (struct sockaddr *)&addr, sizeof addr);
        yos_msleep((random() % 100) + 1);
    }
}

static void test_yos_poll_case(void)
{
    int fd_send = create_socket(12345);
    int fd_recv = create_socket(12346);
    struct sockaddr_in addr;
    char buf[128], buf2[256];
    int ret;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12346);

    memset(buf, 0x5a, sizeof buf);

    ret = sendto(fd_send, buf, sizeof buf, 0, (struct sockaddr *)&addr, sizeof addr);
    YUNIT_ASSERT(ret == sizeof(buf));

    ret = recv(fd_recv, buf2, sizeof buf2, 0);
    YUNIT_ASSERT(ret == sizeof(buf));

    ret = sendto(fd_send, buf, sizeof buf, 0, (struct sockaddr *)&addr, sizeof addr);
    YUNIT_ASSERT(ret == sizeof(buf));

    ret = do_poll(fd_recv, 0);
    YUNIT_ASSERT(ret == sizeof(buf));

    ret = do_poll(fd_recv, 0);
    YUNIT_ASSERT(ret == 0);

    yos_task_new("sender", send_seq_data, &fd_send, 4096);

    int i;
    for (i=1;i<MAXCNT;i++) {
        ret = do_poll(fd_recv, 5000);
        YUNIT_ASSERT(ret == i);
    }

    close(fd_send);
    close(fd_recv);
}

static yunit_test_case_t yos_vfs_testcases[] = {
    { "register", test_yos_vfs_case },
    { "poll", test_yos_poll_case },
    { "fs_register", test_vfs_fs_case},
    YUNIT_TEST_CASE_NULL
};

static int init(void)
{
    return 0;
}

static int cleanup(void)
{
    return 0;
}

static void setup(void)
{
}

static void teardown(void)
{
}

static yunit_test_suite_t suites[] = {
    { "vfs", init, cleanup, setup, teardown, yos_vfs_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_vfs(void)
{
    yunit_add_test_suites(suites);
}

