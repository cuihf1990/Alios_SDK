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
#include <string.h>
#include <hal/ota.h>
#include <yos/framework.h>
#include <yos/log.h>

#include "ota_transport.h"
#include "ota_update_manifest.h"
#include "ota_util.h"
#define TAG "ota"

static write_flash_cb_t ota_write_flash_callback;
static ota_finish_cb_t  ota_finish_callbak;

static void ota_set_callbacks(write_flash_cb_t flash_cb,
                              ota_finish_cb_t finish_cb)
{
    ota_write_flash_callback = flash_cb;
    ota_finish_callbak = finish_cb;
}


static int ota_write_ota_cb(int32_t writed_size, uint8_t *buf, int32_t buf_len, int type)
{
    return hal_ota_write(hal_ota_get_default_module(), NULL, buf, buf_len);
}

static int ota_finish_cb(int32_t finished_result, const char* updated_version)
{
    return hal_ota_set_boot(hal_ota_get_default_module(), (void *)updated_version);
}

ota_request_params ota_request_parmas;
/*
const char *ota_info = "{\"md5\":\"6B21342306D0F619AF97006B7025D18A\","
        "\"resourceUrl\":\"http:\/\/otalink.alicdn.com\/ALINKTEST_LIVING_LIGHT_ALINK_TEST\/v2.0.0.1\/uthash-master.zip\","
        "\"size\": \"265694 \",\"uuid\": \"5B7CFD5C6B1D6A231F5FB6B7DB2B71FD\",\"version\": \"v2.0.0.1\",\"zip\": \"0\"}";
*/
//const char *ota_info = "{\"uuid\": \"5B7CFD5C6B1D6A231F5FB6B7DB2B71FD\"}";
void do_update(const char *buf)
{
    LOGD(TAG, "begin do update %s" , (char *)buf);
    if(!buf) {
        LOGE(TAG, "do update buf is null");
        return;
    }

    ota_request_parmas.primary_version = ota_get_system_version();
    ota_request_parmas.product_type = ota_get_product_type();
    ota_request_parmas.product_internal_type = ota_get_product_internal_type();
    ota_request_parmas.device_uuid = ota_get_id();

    ota_response_params response_parmas;

    ota_set_callbacks(ota_write_ota_cb, ota_finish_cb);
    parse_ota_response(buf, strlen((char *)buf), &response_parmas);
    ota_do_update_packet(&response_parmas, &ota_request_parmas, ota_write_flash_callback,
            ota_finish_callbak);
}

void cancel_update(const char *buf)
{
    LOGD(TAG, "begin cancel update %s" , (char *)buf);
    if(!buf) {
        LOGE(TAG, "cancel update buf is null");
        return;
    }

    ota_response_params response_parmas;
    parse_ota_cancel_response(buf, strlen((char *)buf), &response_parmas);
    ota_cancel_update_packet(&response_parmas);
}

void ota_check_update(const char *buf, int len)
{
    ota_request_parmas.primary_version = ota_get_system_version();
    ota_request_parmas.product_type = ota_get_product_type();
    ota_request_parmas.product_internal_type = ota_get_product_internal_type();
    ota_request_parmas.device_uuid = ota_get_id();

    ota_sub_request_reply(do_update);
    ota_pub_request(&ota_request_parmas);

}


void ota_service_event(input_event_t *event, void *priv_data) {
    if (event->type == EV_SYS && event->code == CODE_SYS_ON_START_FOTA) {
        LOGD(TAG, "ota_service_event-------------fota");
        ota_request_parmas.primary_version = ota_get_system_version();
        ota_request_parmas.product_type = ota_get_product_type();
        ota_request_parmas.product_internal_type = ota_get_product_internal_type();
        ota_request_parmas.device_uuid = ota_get_id();
        ota_post_version_msg();
        ota_sub_upgrade(do_update);
        ota_cancel_upgrade(cancel_update);
    }
}

void ota_service_init(void) {
    yos_register_event_filter(EV_SYS, ota_service_event, NULL);
}
