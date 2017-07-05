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
#include <yos/network.h>


#include "yunit.h"

#define VFLASH_TEST_PNO 99
static const char *dev_name = "/dev/flash99";
static const char *t_string_1 = "t_str_1";
static const char *t_string_2 = "t_str_2";
static const char *t_string_3 = "t_str_3";

static void test_vflash_write_case()
{
    int fd, ret;

    fd = yos_open(dev_name, O_RDWR);
    YUNIT_ASSERT(fd != 0);

    ret = yos_write(fd, t_string_1, strlen(t_string_1));
    YUNIT_ASSERT(ret == strlen(t_string_1));

    ret = yos_write(fd, t_string_2, strlen(t_string_2));
    YUNIT_ASSERT(ret == strlen(t_string_2));

    ret = yos_write(fd, t_string_3, strlen(t_string_3));
    YUNIT_ASSERT(ret == strlen(t_string_3));

    ret = yos_close(fd);
    YUNIT_ASSERT(ret == 0);   
}

static void test_vflash_read_case()
{
    int fd, ret;
    char buf[10] = {0};

    fd = yos_open(dev_name, O_RDONLY);
    YUNIT_ASSERT(fd != 0);

    ret = yos_read(fd, buf, strlen(t_string_1));
    YUNIT_ASSERT(ret == strlen(t_string_1));
    ret = memcmp(buf, t_string_1, strlen(t_string_1));
    YUNIT_ASSERT(ret == 0);

    memset(buf, 0, sizeof(buf));
    ret = yos_read(fd, buf, strlen(t_string_2));
    YUNIT_ASSERT(ret == strlen(t_string_2));
    ret = memcmp(buf, t_string_2, strlen(t_string_2));
    YUNIT_ASSERT(ret == 0);

    memset(buf, 0, sizeof(buf));
    ret = yos_read(fd, buf, strlen(t_string_3));
    YUNIT_ASSERT(ret == strlen(t_string_3));
    ret = memcmp(buf, t_string_3, strlen(t_string_3));
    YUNIT_ASSERT(ret == 0);

    ret = yos_close(fd);
    YUNIT_ASSERT(ret == 0);   
}


#define SECTOR_SIZE 4096

static void test_vflash_overwrite_case()
{
    int fd, ret;
    char *buffer = NULL;
    char *cmp_buf = NULL;

    fd = yos_open(dev_name, O_RDWR);
    YUNIT_ASSERT(fd != 0);

    buffer = (char *)yos_malloc(2 *SECTOR_SIZE);
    if (!buffer)
        return;

    memset(buffer, 'a', 2000);
    ret = yos_write(fd, buffer, 2000);
    YUNIT_ASSERT(ret == 2000);

    memset(buffer, 'b', 2256);
    ret = yos_write(fd, buffer, 2256);
    YUNIT_ASSERT(ret == 2256);

    memset(buffer, 'c', 2 * SECTOR_SIZE);
    ret = yos_write(fd, buffer, 2 * SECTOR_SIZE);
    YUNIT_ASSERT(ret == (2 * SECTOR_SIZE));

    ret = yos_close(fd);
    YUNIT_ASSERT(ret == 0);

    cmp_buf = (char *)yos_malloc(2 * SECTOR_SIZE);
    if (!cmp_buf) {
        if (buffer)
            yos_free(buffer);
        return;
    }
    fd = yos_open(dev_name, O_RDONLY);
    YUNIT_ASSERT(fd != 0);

    memset(buffer, 0, 2 * SECTOR_SIZE);
    ret = yos_read(fd, buffer, 2000);
    YUNIT_ASSERT(ret == 2000);
    memset(cmp_buf, 'a', 2000);
    ret = memcmp(buffer, cmp_buf, 2000);
    YUNIT_ASSERT(ret == 0);

    ret = yos_read(fd, buffer, 2256);
    YUNIT_ASSERT(ret == 2256);
    memset(cmp_buf, 'b', 2256);
    ret = memcmp(buffer, cmp_buf, 2256);
    YUNIT_ASSERT(ret == 0);

    ret = yos_read(fd, buffer, 2 * SECTOR_SIZE);
    YUNIT_ASSERT(ret == (2 * SECTOR_SIZE));
    memset(cmp_buf, 'c', 2 * SECTOR_SIZE);
    ret = memcmp(buffer, cmp_buf, 2 * SECTOR_SIZE);
    YUNIT_ASSERT(ret == 0);

    yos_close(fd);
    if (buffer)
        yos_free(buffer);

    if (cmp_buf)
        yos_free(cmp_buf);

}


static int init(void)
{
    int ret = 0;
    
    ret = vflash_register_partition(VFLASH_TEST_PNO);
    YUNIT_ASSERT(ret == 0);
    return 0;
}

static int cleanup(void)
{
    int ret;

    ret = yunos_unregister_driver(dev_name);

    return ret;
}

static void setup(void)
{
}

static void teardown(void)
{
}

static yunit_test_case_t yos_vflash_testcases[] = {
    { "vflash_write", test_vflash_write_case },
    { "vflash_read", test_vflash_read_case },
    { "vflash_overwrite", test_vflash_overwrite_case },
    YUNIT_TEST_CASE_NULL
};


static yunit_test_suite_t suites[] = {
    { "vflash", init, cleanup, setup, teardown, yos_vflash_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_vflash(void)
{
    yunit_add_test_suites(suites);
}


