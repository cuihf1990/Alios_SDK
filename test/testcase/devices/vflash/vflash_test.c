/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <unistd.h>

#include <aos/kernel.h>
#include <aos/aos.h>
#include <aos/network.h>


#include "yunit.h"

#define VFLASH_TEST_PNO 99
static const char *dev_name = "/dev/flash99";
static const char *t_string_1 = "t_str_1";
static const char *t_string_2 = "t_str_2";
static const char *t_string_3 = "t_str_3";

static void test_vflash_write_case()
{
    int fd, ret;

    fd = aos_open(dev_name, O_RDWR);
    YUNIT_ASSERT(fd != 0);

    ret = aos_write(fd, t_string_1, strlen(t_string_1));
    YUNIT_ASSERT(ret == strlen(t_string_1));

    ret = aos_write(fd, t_string_2, strlen(t_string_2));
    YUNIT_ASSERT(ret == strlen(t_string_2));

    ret = aos_write(fd, t_string_3, strlen(t_string_3));
    YUNIT_ASSERT(ret == strlen(t_string_3));

    ret = aos_close(fd);
    YUNIT_ASSERT(ret == 0);   
}

static void test_vflash_read_case()
{
    int fd, ret;
    char buf[10] = {0};

    fd = aos_open(dev_name, O_RDONLY);
    YUNIT_ASSERT(fd != 0);

    ret = aos_read(fd, buf, strlen(t_string_1));
    YUNIT_ASSERT(ret == strlen(t_string_1));
    ret = memcmp(buf, t_string_1, strlen(t_string_1));
    YUNIT_ASSERT(ret == 0);

    memset(buf, 0, sizeof(buf));
    ret = aos_read(fd, buf, strlen(t_string_2));
    YUNIT_ASSERT(ret == strlen(t_string_2));
    ret = memcmp(buf, t_string_2, strlen(t_string_2));
    YUNIT_ASSERT(ret == 0);

    memset(buf, 0, sizeof(buf));
    ret = aos_read(fd, buf, strlen(t_string_3));
    YUNIT_ASSERT(ret == strlen(t_string_3));
    ret = memcmp(buf, t_string_3, strlen(t_string_3));
    YUNIT_ASSERT(ret == 0);

    ret = aos_close(fd);
    YUNIT_ASSERT(ret == 0);   
}


#define SECTOR_SIZE 4096

static void test_vflash_overwrite_case()
{
    int fd, ret;
    char *buffer = NULL;
    char *cmp_buf = NULL;

    fd = aos_open(dev_name, O_RDWR);
    YUNIT_ASSERT(fd != 0);

    buffer = (char *)aos_malloc(2 *SECTOR_SIZE);
    if (!buffer)
        return;

    memset(buffer, 'a', 2000);
    ret = aos_write(fd, buffer, 2000);
    YUNIT_ASSERT(ret == 2000);

    memset(buffer, 'b', 2256);
    ret = aos_write(fd, buffer, 2256);
    YUNIT_ASSERT(ret == 2256);

    memset(buffer, 'c', 2 * SECTOR_SIZE);
    ret = aos_write(fd, buffer, 2 * SECTOR_SIZE);
    YUNIT_ASSERT(ret == (2 * SECTOR_SIZE));

    ret = aos_close(fd);
    YUNIT_ASSERT(ret == 0);

    cmp_buf = (char *)aos_malloc(2 * SECTOR_SIZE);
    if (!cmp_buf) {
        if (buffer)
            aos_free(buffer);
        return;
    }
    fd = aos_open(dev_name, O_RDONLY);
    YUNIT_ASSERT(fd != 0);

    memset(buffer, 0, 2 * SECTOR_SIZE);
    ret = aos_read(fd, buffer, 2000);
    YUNIT_ASSERT(ret == 2000);
    memset(cmp_buf, 'a', 2000);
    ret = memcmp(buffer, cmp_buf, 2000);
    YUNIT_ASSERT(ret == 0);

    ret = aos_read(fd, buffer, 2256);
    YUNIT_ASSERT(ret == 2256);
    memset(cmp_buf, 'b', 2256);
    ret = memcmp(buffer, cmp_buf, 2256);
    YUNIT_ASSERT(ret == 0);

    ret = aos_read(fd, buffer, 2 * SECTOR_SIZE);
    YUNIT_ASSERT(ret == (2 * SECTOR_SIZE));
    memset(cmp_buf, 'c', 2 * SECTOR_SIZE);
    ret = memcmp(buffer, cmp_buf, 2 * SECTOR_SIZE);
    YUNIT_ASSERT(ret == 0);

    aos_close(fd);
    if (buffer)
        aos_free(buffer);

    if (cmp_buf)
        aos_free(cmp_buf);

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

    ret = aos_unregister_driver(dev_name);

    return ret;
}

static void setup(void)
{
}

static void teardown(void)
{
}

static yunit_test_case_t aos_vflash_testcases[] = {
    { "vflash_write", test_vflash_write_case },
    { "vflash_read", test_vflash_read_case },
    { "vflash_overwrite", test_vflash_overwrite_case },
    YUNIT_TEST_CASE_NULL
};


static yunit_test_suite_t suites[] = {
    { "vflash", init, cleanup, setup, teardown, aos_vflash_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_vflash(void)
{
    yunit_add_test_suites(suites);
}


