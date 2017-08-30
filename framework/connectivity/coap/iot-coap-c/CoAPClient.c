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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yos/conf.h>
#include <yos/framework.h>
#include <yos/log.h>
#include <k_err.h>
#include <netmgr.h>
#include "iot_import.h"
#include "iot_export.h"

#define IOTX_PRE_DTLS_SERVER_URI "coaps://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5684"
#define IOTX_PRE_NOSEC_SERVER_URI "coap://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5683"

#define IOTX_ONLINE_DTLS_SERVER_URL "coaps://%s.iot-as-coap.cn-shanghai.aliyuncs.com:5684"

#define TAG "coap_client"

#if 0
#define IOTX_PRODUCT_KEY         "trTceekBd1P"
#define IOTX_DEVICE_NAME         "KAW7ihRrroLevlHN1y21"
#define IOTX_DEVICE_SECRET       "0yscIv4r7cIc3aDu6kKGvyVVEWvobQF6"
#define IOTX_DEVICE_ID           "trTceekBd1P.KAW7ihRrroLevlHN1y21"
#else
#define IOTX_PRODUCT_KEY         "GJ2uoVqx7ka"
#define IOTX_DEVICE_NAME         "dev1"
#define IOTX_DEVICE_SECRET       "Rdzq3RkePPcAjX82rz2yANa4BREwIWvW"
#define IOTX_DEVICE_ID           "dev1.GJ2uoVqx7ka"
#endif

extern int iotx_device_name_auth(iotx_coap_context_t *p_context);

iotx_coap_context_t *p_ctx = NULL;

iotx_coap_context_t *get_coap_context()
{
    return p_ctx;
}

int coap_service_connected = 0;

int iotx_set_devinfo(iotx_deviceinfo_t *p_devinfo)
{
    if(NULL == p_devinfo){
        return IOTX_ERR_INVALID_PARAM;
    }

    memset(p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));
    strncpy(p_devinfo->device_id,    IOTX_DEVICE_ID,   IOTX_DEVICE_ID_LEN);
    strncpy(p_devinfo->product_key,  IOTX_PRODUCT_KEY, IOTX_PRODUCT_KEY_LEN);
    strncpy(p_devinfo->device_secret,IOTX_DEVICE_SECRET, IOTX_DEVICE_SECRET_LEN);
    strncpy(p_devinfo->device_name,  IOTX_DEVICE_NAME, IOTX_DEVICE_NAME_LEN);

    return IOTX_SUCCESS;
}

int coap_service_init()
{
    if(!coap_service_connected) {
        printf("[COAP-Client]: Enter Coap Client\r\n");
        coap_service_connected = 1;
        
        iotx_coap_config_t config;
        iotx_deviceinfo_t deviceinfo;
        #ifdef COAP_ONLINE
            #ifdef COAP_DTLS_SUPPORT
                char url[256] = {0};
                snprintf(url, sizeof(url), IOTX_ONLINE_DTLS_SERVER_URL, IOTX_PRODUCT_KEY);
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
        iotx_set_devinfo(&deviceinfo);
        config.p_devinfo = &deviceinfo;

        p_ctx = IOT_CoAP_Init(&config);
        if(NULL != p_ctx){
            return IOT_CoAP_DeviceNameAuth(p_ctx);
        }
    }
    return 0;
}

int coap_service_deinit(){
    coap_service_connected = 0;
    IOT_CoAP_Deinit(&p_ctx);
}
