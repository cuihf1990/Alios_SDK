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

#include <stdio.h>
#include <unistd.h>

#include <yos/kernel.h>
#include <yos/framework.h>

#include "vfs.h"
#include "vfs_inode.h"
#include "vfs_driver.h"
#include "vfs_err.h"

#include "yunit.h"

static int test_ioctl(file_t *node, int cmd, unsigned long arg)
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
        ret = yunos_register_driver(names[i], &myops, NULL);
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

        ret = yunos_unregister_driver(names[i]);
        YUNIT_ASSERT(ret == 0);

        fd = yos_open(names[i], 0);
        YUNIT_ASSERT(fd < 0);
        YUNIT_ASSERT(E_VFS_FD_ILLEGAL == yos_ioctl(fd, 0, 0));
    }
}

static yunit_test_case_t yos_vfs_testcases[] = {
    { "Testing: yos_register add/del", test_yos_vfs_case },
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

