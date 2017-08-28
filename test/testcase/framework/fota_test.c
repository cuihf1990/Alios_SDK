#include <stdlib.h>
#include <stdio.h>

#include <yos/kernel.h>
#include <yos/framework.h>

#include <yunit.h>
#include <yts.h>
#include "ota_transport.h"
extern void ota_check_update(const char *buf, int len);
extern void ota_service_init(void);
extern void do_update(const char *buf);
extern void http_gethost_info(char* src, char* web, char* file, int* port);
extern int http_socket_init(int port, char *host_addr);
extern void ota_download_start(void * buf);
extern int8_t parse_ota_requset(const char* request, int *buf_len, ota_request_params * request_parmas);
extern int8_t parse_ota_response(const char* response, int buf_len, ota_response_params * response_parmas);

const char *ota_info = "{\"md5\":\"6B21342306D0F619AF97006B7025D18A\",\"resourceUrl\":\"http://otalink.alicdn.com/ALINKTEST_LIVING_LIGHT_ALINK_TEST/v2.0.0.1/uthash-master.zip\",\"size\":\"265694 \",\"uuid\":\"5B7CFD5C6B1D6A231F5FB6B7DB2B71FD\",\"version\":\"v2.0.0.1\",\"zip\":\"0\"}";

static void test_fota_case(void)
{
    int ret = 0;
    ota_check_update("",1);
    ota_download_start(NULL);
    do_update(NULL);
    do_update(ota_info);

    http_gethost_info(NULL, NULL, NULL, NULL);
    ret = http_socket_init(0, NULL);
    YUNIT_ASSERT(ret == -1);
    ret = parse_ota_requset(NULL, NULL, NULL);
    YUNIT_ASSERT(ret == 0);
    ret = parse_ota_response(NULL, 0, NULL);
    YUNIT_ASSERT(ret == -1)
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
    { "fota", test_fota_case },
    YUNIT_TEST_CASE_NULL
};

static yunit_test_suite_t suites[] = {
    { "fota", init, cleanup, setup, teardown, yunos_basic_testcases },
    YUNIT_TEST_SUITE_NULL
};

void test_fota(void)
{
    yunit_add_test_suites(suites);
}

