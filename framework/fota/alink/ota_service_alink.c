/*
 *Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */
#include <string.h>
#include <hal/ota.h>
#include <yos/framework.h>
#include <yos/version.h>
#include <yos/log.h>

#include "ota_transport.h"
#include "ota_update_manifest.h"
#include "ota_version.h"
#include "ota_util.h"
#include "ota_service.h"
#define TAG "ota"

static write_flash_cb_t ota_write_flash_callback;
static ota_finish_cb_t  ota_finish_callbak;

static void ota_set_callbacks(write_flash_cb_t flash_cb,
                              ota_finish_cb_t finish_cb)
{
    ota_write_flash_callback = flash_cb;
    ota_finish_callbak = finish_cb;
}

int ota_hal_init()
{
    return hal_ota_init();
}

static int ota_hal_write_cb(int32_t writed_size, uint8_t *buf, int32_t buf_len, int type)
{
    return hal_ota_write(hal_ota_get_default_module(), NULL, buf, buf_len);
}

static int ota_hal_finish_cb(int32_t finished_result, void *updated_type)
{
    return hal_ota_set_boot(hal_ota_get_default_module(), (void *)updated_type);
}

ota_request_params ota_request_parmas;
/*
const char *ota_info = "{\"md5\":\"6B21342306D0F619AF97006B7025D18A\","
        "\"resourceUrl\":\"http:\/\/otalink.alicdn.com\/ALINKTEST_LIVING_LIGHT_ALINK_TEST\/v2.0.0.1\/uthash-master.zip\","
        "\"size\": \"265694 \",\"uuid\": \"5B7CFD5C6B1D6A231F5FB6B7DB2B71FD\",\"version\": \"v2.0.0.1\",\"zip\": \"0\"}";
*/
//const char *ota_info = "{\"uuid\": \"5B7CFD5C6B1D6A231F5FB6B7DB2B71FD\"}";
void do_update(int len,  const char *buf)
{
    LOGD(TAG, "begin do update %s" , (char *)buf);
    if (!buf) {
        LOGE(TAG, "do update buf is null");
        return;
    }
    ota_response_params response_parmas;
    ota_set_callbacks(ota_hal_write_cb, ota_hal_finish_cb);
    parse_ota_response(buf, strlen((char *)buf), &response_parmas);
    ota_do_update_packet(&response_parmas, &ota_request_parmas, ota_write_flash_callback,
                         ota_finish_callbak);
}

void cancel_update(int len, const char *buf)
{
    LOGD(TAG, "begin cancel update %s" , (char *)buf);
    if (!buf) {
        LOGE(TAG, "cancel update buf is null");
        return;
    }

    ota_response_params response_parmas;
    parse_ota_cancel_response(buf, strlen((char *)buf), &response_parmas);
    ota_cancel_update_packet(&response_parmas);
}

void ota_check_update(const char *buf, int len)
{
    ota_sub_request_reply(do_update);
    ota_pub_request(&ota_request_parmas);
}

static int ota_init = 0;

void ota_regist_upgrade(void)
{
    ota_post_version_msg();
    ota_sub_upgrade(&do_update);
    ota_cancel_upgrade(&cancel_update);
}

static void init_device_parmas()
{
#ifdef SYSINFO_OS_BINS
    ota_request_parmas.primary_version = get_yos_os_version();
#else
    ota_request_parmas.primary_version = get_yos_kernel_version();
#endif
    ota_request_parmas.secondary_version = get_yos_app_version();
    ota_request_parmas.product_type = get_yos_product_model();
    ota_request_parmas.device_uuid = ota_get_id();
}

void ota_service_event(input_event_t *event, void *priv_data)
{
    if (event->type == EV_SYS && event->code == CODE_SYS_ON_START_FOTA) {
        if (ota_init) {
            return;
        }
        ota_init = 1;
        init_device_parmas();
        ota_regist_upgrade();
    }
}

void ota_service_ch_init(void)
{
    yos_register_event_filter(EV_SYS, ota_service_event, NULL);
}
