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
#include <yos_version.h>

#include "ota_transport.h"
#include "ota_update_manifest.h"
#include "ota_util.h"
#define TAG "osupdate"

static inline const char *ota_get_product_type(void);
static inline const char *ota_get_system_version(void);
static inline const char *ota_get_product_internal_type(void);

static write_flash_cb_t ota_write_flash_callback;
static ota_finish_cb_t  ota_finish_callbak;

static const char *ota_get_product_type(void)
{
    return (char *)get_yos_product_model();
}

static const char *ota_get_system_version(void)
{
    return (char *)get_yos_os_version();
}

static const char *ota_get_product_internal_type(void)
{
    return (char *)get_yos_product_internal_type();
}

static void ota_set_callbacks(write_flash_cb_t flash_cb,
                              ota_finish_cb_t finish_cb)
{
    ota_write_flash_callback = flash_cb;
    ota_finish_callbak = finish_cb;
}

static int ota_write_ota_cb(int32_t writed_size, uint8_t *buf, int32_t buf_len,
                            int type)
{
    return hal_ota_write_ota_cb(hal_ota_get_default_module(), writed_size,
                                buf, buf_len, type);
}

static int ota_finish_cb(int32_t finished_result, const char* updated_version)
{
    yos_local_event_post(EV_OTA, CODE_OTA_ON_RESULT, finished_result);
    return hal_ota_ota_finish_cb(hal_ota_get_default_module(), finished_result,
                                 updated_version);
}

ota_request_params ota_request_parmas;

void do_update(void *buf)
{
    LOGD(TAG, "begin do update");
    if(!buf) {
        LOGE(TAG, "do update buf is null");
        return;
    }

    ota_response_params response_parmas;

    ota_set_callbacks(ota_write_ota_cb, ota_finish_cb);
    parse_ota_response(buf, strlen((char *)buf), &response_parmas);
    ota_do_update(&response_parmas, &ota_request_parmas, ota_write_flash_callback,
            ota_finish_callbak);
}

void osupdate_do_update(const char *buf, int len)
{
    yos_schedule_call(do_update, (char *)buf);
}

void osupdate_check_update(const char *buf, int len)
{
    ota_request_parmas.primary_version = ota_get_system_version();
    ota_request_parmas.product_type = ota_get_product_type();
    ota_request_parmas.product_internal_type = ota_get_product_internal_type();
    ota_request_parmas.device_uuid = ota_get_id2();

    ota_sub_request_reply(osupdate_do_update);

    int8_t ret = ota_pub_request(&ota_request_parmas);
    if(ret) {
        return;
    }

}

int osupdate_service_init(void) {
    return 0;
}

void osupdate_service_deinit(void) {
}

int osupdate_service_start(void) {
    return 0;
}

void osupdate_service_stop(void) {
}


void osupdate_service_event(input_event_t *event) {
    if (event->type == EV_WIFI && event->code == CODE_WIFI_ON_GOT_IP) {
        //csp_task_new("iot", IOT_service_start, NULL, 1024 * 12);
        ota_sub_upgrade(osupdate_do_update);
    }
}

