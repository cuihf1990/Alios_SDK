 /*
  * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
  * License-Identifier: Apache-2.0
  *
  * Licensed under the Apache License, Version 2.0 (the "License"); you may
  * not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
  * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <yos/framework.h>

#include "ota_service.h"
#include "iot_import.h"
#include "iot_export.h"

#define IOTX_PRE_DTLS_SERVER_URI "coaps://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5684"
#define IOTX_PRE_NOSEC_SERVER_URI "coap://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5683"

#define IOTX_ONLINE_DTLS_SERVER_URL "coaps://%s.iot-as-coap.cn-shanghai.aliyuncs.com:5684"

#define EXAMPLE_TRACE(fmt, args...)  \
    do { \
        printf("%s|%03d :: ", __func__, __LINE__); \
        printf(fmt, ##args); \
        printf("%s", "\r\n"); \
    } while(0)

void coap_client()
{
    void *h_ota = NULL;
    iotx_coap_config_t config;
    iotx_deviceinfo_t  devinfo;
    iotx_coap_context_t *h_coap = NULL;

    iotx_set_devinfo(&devinfo);
    config.p_devinfo = &devinfo;
    #ifdef COAP_ONLINE
        #ifdef COAP_DTLS_SUPPORT
            char url[256] = {0};
            snprintf(url, sizeof(url), IOTX_ONLINE_DTLS_SERVER_URL, devinfo.product_key);
            config.p_url = url;
        #else
            printf("Online environment must access with DTLS\r\n");
            return -1;
        #endif
    #else
        #ifdef COAP_DTLS_SUPPORT
            config.p_url = IOTX_PRE_DTLS_SERVER_URI;
        #else
            config.p_url = IOTX_PRE_NOSEC_SERVER_URI;
        #endif
    #endif

    h_coap = IOT_CoAP_Init(&config);

    if(NULL == h_coap){
        EXAMPLE_TRACE("initialize CoAP failed");
        return;
    }

    IOT_CoAP_DeviceNameAuth(h_coap);

    h_ota = IOT_OTA_Init(devinfo.product_key, devinfo.device_name, h_coap);
    if (NULL == h_ota) {
        EXAMPLE_TRACE("initialize OTA failed");
        goto do_exit;
    }

    int ota_code = 0;
    do{

        HAL_SleepMs(2000);
        //TODO: get version by code
        ota_code = IOT_OTA_ReportVersion(h_ota, "iotx_ver_1.0.0");

        IOT_CoAP_Yield(h_coap);

    } while (0 != ota_code);

    HAL_SleepMs(2000);

    try_fetch_ota(h_ota);

    EXAMPLE_TRACE("OTA success");

do_exit:
    if (NULL != h_ota) {
        IOT_OTA_Deinit(h_ota);
    }
    if (NULL != h_coap) {
        IOT_CoAP_Deinit(&h_coap);
    }

    yos_post_delayed_action(7*24*60*60*1000,coap_client,NULL);
}

void ota_service_event(input_event_t *event, void *priv_data)
{
    if (event->type == EV_WIFI && event->code == CODE_WIFI_ON_GOT_IP)
    {
        IOT_OpenLog("coap-ota");
        IOT_SetLogLevel(IOT_LOG_DEBUG);

        coap_client();
    }
}

void ota_service_ch_init()
{
    yos_register_event_filter(EV_WIFI, ota_service_event, NULL);
}
