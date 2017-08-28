#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vfs_register.h>
#include <yos/kernel.h>
#include <yunit.h>
#include <yts.h>

static const char *g_string         = "Fatfs test string.";
static const char *g_filepath       = "/ramdisk/test.txt";
static const char *g_dirpath        = "/ramdisk/testDir";
static const char *g_dirtest_1      = "/ramdisk/testDir/test_1.txt";
static const char *g_dirtest_2      = "/ramdisk/testDir/test_2.txt";
static const char *g_dirtest_3      = "/ramdisk/testDir/test_3.txt";
static const char *g_new_filepath   = "/ramdisk/testDir/newname.txt";

static void test_fatfs_case(void)
{
    int ret, fd;
    char readBuffer[32];

    /* Fatfs write test */
    fd = yos_open(g_filepath, O_RDWR | O_CREAT | O_TRUNC);
    YUNIT_ASSERT(fd > 0);
 
    if (fd > 0) {
        ret = yos_write(fd, g_string, strlen(g_string));
        YUNIT_ASSERT(ret > 0);
        ret = yos_sync(fd);
        YUNIT_ASSERT(ret == 0);

        yos_close(fd);
    }

    /* Fatfs read test */
    fd = yos_open(g_filepath, O_RDONLY);
    YUNIT_ASSERT(fd > 0);
    if (fd > 0) {
        ret = yos_read(fd, readBuffer, sizeof(readBuffer));
        YUNIT_ASSERT(ret > 0);

        ret = memcmp(readBuffer, g_string, strlen(g_string));
        YUNIT_ASSERT(ret == 0);

        yos_close(fd);      
    }

    /* Fatfs mkdir test */
    yos_dir_t *dp = (yos_dir_t *)yos_opendir(g_dirpath);
    if (!dp) {
        ret = yos_mkdir(g_dirpath);
        YUNIT_ASSERT(ret == 0);
    } else {
        ret = yos_closedir(dp);
        YUNIT_ASSERT(ret == 0);
    }

    /* Fatfs readdir test */
    fd = yos_open(g_dirtest_1, O_RDWR | O_CREAT | O_TRUNC);
    YUNIT_ASSERT(fd > 0);
    if (fd > 0)
        yos_close(fd);

    fd = yos_open(g_dirtest_2, O_RDWR | O_CREAT | O_TRUNC);
    YUNIT_ASSERT(fd > 0);
    if (fd > 0)
        yos_close(fd);

    fd = yos_open(g_dirtest_3, O_RDWR | O_CREAT | O_TRUNC);
    YUNIT_ASSERT(fd > 0);
    if (fd > 0)
        yos_close(fd);

    dp = (yos_dir_t *)yos_opendir(g_dirpath);
    YUNIT_ASSERT(dp != NULL);

    if (dp) {
        yos_dirent_t *out_dirent;
        while(1) {
            out_dirent = (yos_dirent_t *)yos_readdir(dp);
            if (out_dirent == NULL)
                break;

            printf("file name is %s\n", out_dirent->d_name);            
        }
    }
    yos_closedir(dp);

    /* Fatfs rename test */
    ret = yos_rename(g_filepath, g_new_filepath);
    YUNIT_ASSERT(ret == 0);

    fd = yos_open(g_filepath, O_RDONLY);
    YUNIT_ASSERT(fd < 0);
    if (fd >= 0)
        yos_close(fd);

    fd = yos_open(g_new_filepath, O_RDONLY);
    YUNIT_ASSERT(fd > 0);
    if (fd > 0)
        yos_close(fd);

    /* Fatfs unlink test */
    ret = yos_unlink(g_new_filepath);
    YUNIT_ASSERT(ret == 0);

    fd = yos_open(g_new_filepath, O_RDONLY);
    YUNIT_ASSERT(fd < 0);
    if (fd > 0)
        yos_close(fd);
}

static int init(void)
{
    int ret = 0;

    /* register RAMDISK dev */
    ret = fatfs_register(0);
    YUNIT_ASSERT(ret == 0);
    return 0;
}

static int cleanup(void)
{
    int ret = fatfs_unregister(0);
    YUNIT_ASSERT(ret == 0);
    return 0;
}

static void setup(void)
{

}

static void teardown(void)
{

}

static yunit_test_case_t yunos_basic_testcases[] = {
    { "fatfs_test", test_fatfs_case},
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "fatfs", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_fatfs(void)
{    
    yunit_add_test_suites(suites);
}


