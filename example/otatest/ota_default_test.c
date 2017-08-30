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

#include <stdlib.h>
#include <stdio.h>

#include <yos/kernel.h>
#include <yos/framework.h>

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

int application_start(void)
{
    int rc = 0;

    ota_check_update("",1);
    ota_download_start(NULL);
    do_update(NULL);
    do_update(ota_info);

    return rc;
}
