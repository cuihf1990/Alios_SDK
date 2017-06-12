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
#include "ota_platform_os.h"

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
char md5[33];

extern int  check_md5(const char *buffer, const int32_t len);

void ota_download_start(void * buf)
{
    OTA_LOG_I("task update start");
    ota_status_init();
    ota_set_status(OTA_INIT);
    ota_status_post(100);

    ota_set_status(OTA_DOWNLOAD);
    int ret = http_download(url, g_write_func); 
    if(ret <= 0) {
        OTA_LOG_E("ota download error");
        ota_set_status(OTA_DOWNLOAD_FAILED);
        goto OTA_END;
    }
    
    if(ret == OTA_DOWNLOAD_CANCEL) {
        OTA_LOG_E("ota download cancel");
        ota_set_status(OTA_CANCEL);
        goto OTA_END;
    }
   
    ota_status_post(100);
    ota_set_status(OTA_CHECK);
    ret = check_md5(md5,sizeof md5);    
    if(ret < 0 ) {
       OTA_LOG_E("ota check md5 error");
       ota_set_status(OTA_CHECK_FAILED);
       goto OTA_END;
    }
    ota_status_post(100);
    memset(url, 0, sizeof url);
    
    OTA_LOG_I("ota status %d",ota_get_status());
    ota_set_status(OTA_UPGRADE);
    if(NULL != g_finish_cb) {
        g_finish_cb(0,"");
    }
    ota_status_post(100);
    ota_set_status(OTA_REBOOT);

OTA_END:
    ota_status_post(100);    
    OTA_LOG_I("task update over");
    ota_reboot();
}

int8_t ota_post_version_msg()
{
    int ret = -1, ota_success = 0;
    OTA_LOG_I("ota_post_version_msg  [%s][%s] [%s]", ota_get_system_version(), ota_get_version(), ota_get_dev_version());
    if(strlen(ota_get_version()) > 0) {
	ota_success = !strncmp((char *)ota_get_system_version(),
             (char *)ota_get_version(), strlen(ota_get_system_version()));
        if(ota_success) {
            ota_set_status(OTA_REBOOT_SUCCESS);
            ret = ota_status_post(100);          
        }else {
            ota_set_status(OTA_INIT);
            ret = ota_status_post(0);
        }

	if(ret == 0) {
	    OTA_LOG_I("OTA finished, clear ota version in config");
            ota_set_version("");
	}
    }

    if(strncmp((char*)ota_get_system_version(), (char *)ota_get_dev_version(), strlen(ota_get_system_version()))) {
        ret = ota_result_post();
        if(ret == 0) {
            OTA_LOG_I("Save dev version to config");
	    ota_set_dev_version(ota_get_system_version());
	}
    }


    return 0;
}

int8_t ota_do_update_packet(ota_response_params *response_parmas,ota_request_params *request_parmas,
                               write_flash_cb_t func, ota_finish_cb_t fcb)
{
    int ret = 0;

    ret = ota_if_need(response_parmas,request_parmas);
    if(1 != ret) return ret;

    ota_set_version(response_parmas->primary_version);
    g_write_func = func;
    g_finish_cb = fcb;

    memset(md5, 0, sizeof md5);
    strncpy(md5, response_parmas->md5, sizeof md5);

    memset(url, 0, sizeof url);
    strncpy(url, response_parmas->download_url, sizeof url);
    ret = yos_task_new("ota", ota_download_start, 0, 8196);

    return ret;
}

static int8_t ota_is_cancelable()
{
    return ota_get_status() != OTA_INIT && ota_get_status() < OTA_UPGRADE;
}

static int8_t ota_if_cancel(ota_response_params *response_parmas)
{
    if(!response_parmas)
        return 0;

    if(!strncmp(response_parmas->device_uuid , ota_get_id(), sizeof response_parmas->device_uuid))
        return 0;

    if(!ota_is_cancelable())
        return 0;
    return 1;
}

int8_t ota_cancel_update_packet(ota_response_params *response_parmas)
{
    int ret = 0;
    
    ret = ota_if_cancel(response_parmas);
    if(ret) {
        ota_set_status(OTA_CANCEL);
    }
    return ret;
}





