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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <yos/kernel.h>
#include "ota_update_manifest.h"
#include "ota_log.h"
#include "ota_transport.h"
#include "ota_util.h"

#define OTA_URL_MAX_LEN 512

/*
 param-name           meaning                      value-examples
 productType          device model                 XJ-1
 phone                internal device model        XJ-1R
 id2                  intelligent device ID        Y00F30001068494F4
 system               system versioin              1.0.0-R-20150101.1201

 url example:
 http://10.101.111.160/update/manifest?productType＝XJ-1&phone=XJ-1R&
 uuid=yos_id2&system=1.0.0-R-20150101.1201
 */

/*static const char _g_update_manifest_url_test[] =
 "http://10.101.111.160/update/manifest?productType＝XJ-1& \
 phone=XJ-1R&uuid=yos_id2&system=1.0.0-R-20150101.1201";*/

extern int http_download(char *url, write_flash_cb_t func);

int8_t ota_if_need(ota_response_params *response_parmas, ota_request_params *request_parmas)
{
    if(strncmp(response_parmas->primary_version , request_parmas->primary_version, sizeof response_parmas->primary_version) > 0 )
        return 1;
    return 0;
}

write_flash_cb_t g_write_func;
ota_finish_cb_t g_finish_cb;
char url[OTA_URL_MAX_LEN];

void ota_download_start(void * buf)
{
    OTA_LOG_E("/************************************task update start**************************************/");
    ota_set_status(E_OTA_IDLE);
    int ret = http_download(url, g_write_func); 
    if(ret < 0 ) {
       OTA_LOG_E(" ota download error");
       ota_set_status(E_OTA_DOWNLOAD_FAIL);
       return;
    }
    OTA_LOG_I(" ota status %d",ota_get_status());
    ota_set_status(E_OTA_DOWNLOAD_SUC);
    if(NULL != g_finish_cb) {
        g_finish_cb(0,"");
    }
 
    ota_set_status(E_OTA_END);
    OTA_LOG_E("/************************************task update over**************************************/");
    free_global_topic();
}



int8_t ota_do_update_packet(ota_response_params *response_parmas,ota_request_params *request_parmas,
                               write_flash_cb_t func, ota_finish_cb_t fcb)
{
    int ret = 0;

    ret = ota_if_need(response_parmas,request_parmas);
    if(1 != ret) return ret;

    g_write_func = func;
    g_finish_cb = fcb;
    strncpy(url, response_parmas->download_url,sizeof url);
    ret = yos_task_new("ota", ota_download_start, NULL, 8196);

    return ret;
}







