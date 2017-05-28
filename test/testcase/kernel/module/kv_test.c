#include <stdio.h>
#include <stdlib.h>

#include <yunit.h>
#include <yts.h>
#include "yos/framework.h"
#include "kvmgr.h"

static char *g_key_1 = "key_1";
static char *g_key_2 = "key_2";
static char *g_key_3 = "key_3";
static char *g_key_4 = "key_4";

static char *g_val_1 = "val_1";
static char *g_val_2 = "val_2";
static char *g_val_3 = "val_3";
static char *g_val_4 = "val_4";

static void test_kv_add(void)
{
    int ret = 0;
    
    ret = yos_kv_set(g_key_1,g_val_1,strlen(g_val_1),1);
    YUNIT_ASSERT(0 == ret);

    ret = yos_kv_set(g_key_2,g_val_2,strlen(g_val_2),1);
    YUNIT_ASSERT(0 == ret);
    
    ret = yos_kv_set(g_key_3,g_val_3,strlen(g_val_3),1);
    YUNIT_ASSERT(0 == ret);
    
    ret = yos_kv_set(g_key_4,g_val_4,strlen(g_val_4),1);
    YUNIT_ASSERT(0 == ret);
}

static void test_kv_find(void)
{
    int ret = 0;
    char buf[10] = {0};
    int len = sizeof(buf);

    ret = yos_kv_get(g_key_1,buf,&len);
    YUNIT_ASSERT(0 == ret);
    YUNIT_ASSERT(len == strlen(g_val_1));

    ret = yos_kv_get(g_key_2,buf,&len);
    YUNIT_ASSERT(0 == ret);
    YUNIT_ASSERT(len == strlen(g_val_2));

    ret = yos_kv_get(g_key_3,buf,&len);
    YUNIT_ASSERT(0 == ret);
    YUNIT_ASSERT(len == strlen(g_val_3));

    ret = yos_kv_get(g_key_4,buf,&len);
    YUNIT_ASSERT(0 == ret);
    YUNIT_ASSERT(len == strlen(g_val_4));
}

static void test_kv_del(void)
{
    int ret = 0;
    char buf[10] = {0};
    int len = sizeof(buf);

    ret = yos_kv_del(g_key_1); 
    YUNIT_ASSERT(0 == ret);

    ret = yos_kv_del(g_key_2); 
    YUNIT_ASSERT(0 == ret);
   
    ret = yos_kv_del(g_key_3); 
    YUNIT_ASSERT(0 == ret);

    ret = yos_kv_get(g_key_3,buf,&len);
    YUNIT_ASSERT(0 != ret);
    YUNIT_ASSERT(len != strlen(g_val_3)+1);
}
static int init(void)
{
    int ret = 0;

    ret = yos_kv_init();
    YUNIT_ASSERT(ret == 0);
    return 0;
}

static int cleanup(void)
{
	/*wangbin change it because it's a global table,
	call deinit it will cause whole system crash*/
    //yos_kv_deinit();
	
    return 0;
}

static void setup(void)
{

}

static void teardown(void)
{

}

static yunit_test_case_t yunos_basic_testcases[] = {
    { "ht_add", test_kv_add },
    { "ht_find", test_kv_find },
    { "ht_del", test_kv_del },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "kv", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_kv(void)
{    
    yunit_add_test_suites(suites);
}

