#include <stdio.h>
#include <stdlib.h>

#include <yunit.h>
#include <yts.h>
#include "alink_export.h"

static int cloud_is_connected;
void cloud_connected(void) { cloud_is_connected = 1; printf("alink cloud connected!\n"); }
void cloud_disconnected(void) { cloud_is_connected = 0; printf("alink cloud disconnected!\n"); }

static void test_alink_online(void)
{
    alink_register_callback(ALINK_CLOUD_CONNECTED, (void *)&cloud_connected);
    alink_register_callback(ALINK_CLOUD_DISCONNECTED, (void *)&cloud_disconnected);

    EXPECT_EQ(0, alink_enable_daily_mode(NULL, 0));

    EXPECT_EQ(0, alink_start());

    EXPECT_EQ(0, alink_end());

    ASSERT_EQ(0, cloud_is_connected);
}

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

static yunit_test_case_t yunos_basic_testcases[] = {
    { "alink_online", test_alink_online },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "alink", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_alink(void)
{    
    yunit_add_test_suites(suites);
}

