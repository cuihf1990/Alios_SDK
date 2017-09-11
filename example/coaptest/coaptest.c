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

#ifdef PLATFORM_MK3060
#include <lwip/inet.h>
#else
#include <arpa/inet.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <aos/log.h>
#include <aos/aos.h>
#include <netmgr.h>
#include <k_err.h>
#include "iot_import.h"
#include "iot_export.h"

#define IOTX_PRE_DTLS_SERVER_URI "coaps://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5684"
#define IOTX_PRE_NOSEC_SERVER_URI "coap://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5683"

#define IOTX_ONLINE_DTLS_SERVER_URL "coaps://%s.iot-as-coap.cn-shanghai.aliyuncs.com:5684"

#define TAG "coaptest"

char m_coap_client_running = 0;
int coap_client_init = 0;

static void iotx_response_handler(void * arg, void * p_response)
{
    int            len       = 0;
    unsigned char *p_payload = NULL;
    iotx_coap_resp_code_t resp_code;
    IOT_CoAP_GetMessageCode(p_response, &resp_code);
    IOT_CoAP_GetMessagePayload(p_response, &p_payload, &len);
    printf("[APPL]: Message response code: %d\r\n", resp_code);
    printf("[APPL]: Len: %d, Payload: %s, \r\n", len, p_payload);
}

static void iotx_post_data_to_server(void *param)
{
    char               path[IOTX_URI_MAX_LEN+1] = {0};
    iotx_message_t     message;
    iotx_deviceinfo_t  devinfo;
    message.p_payload = (unsigned char *)"{\"name\":\"hello world\"}";
    message.payload_len = strlen("{\"name\":\"hello world\"}");
    message.resp_callback = iotx_response_handler;
    message.msg_type = IOTX_MESSAGE_CON;
    message.content_type = IOTX_CONTENT_TYPE_JSON;
    iotx_coap_context_t *p_ctx = (iotx_coap_context_t *)param;

    iotx_set_devinfo(&devinfo);
    snprintf(path, IOTX_URI_MAX_LEN, "/topic/%s/%s/update/", (char *)devinfo.product_key,
                                            (char *)devinfo.device_name);

    IOT_CoAP_SendMessage(p_ctx, path, &message);
}

iotx_coap_context_t *p_ctx = NULL;

static void user_code_start()
{
    iotx_post_data_to_server((void *)p_ctx);
    IOT_CoAP_Yield(p_ctx);
    if (m_coap_client_running) {
        m_coap_client_running = 0;
        aos_post_delayed_action(3000,user_code_start,NULL);
    } else {
        IOT_CoAP_Deinit(&p_ctx);
    }
}

static void coap_client_example() {
    iotx_coap_config_t config;
    iotx_deviceinfo_t deviceinfo;

    m_coap_client_running = 1;

    iotx_set_devinfo(&deviceinfo);
    #ifdef COAP_ONLINE
        #ifdef COAP_DTLS_SUPPORT
            char url[256] = {0};
            snprintf(url, sizeof(url), IOTX_ONLINE_DTLS_SERVER_URL, deviceinfo.product_key);
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
    config.p_devinfo = &deviceinfo;

    p_ctx = IOT_CoAP_Init(&config);
    if(NULL != p_ctx){
        IOT_CoAP_DeviceNameAuth(p_ctx);
        user_code_start();
    }
    else{
        printf("IoTx CoAP init failed\r\n");
    }
}

void wifi_service_event(input_event_t *event, void *priv_data)
{
    if (event->type == EV_WIFI && event->code == CODE_WIFI_ON_GOT_IP)
    {
		if (coap_client_init) {
			return;
		}
		coap_client_init = 1;
        coap_client_example();
    }
}

int application_start(void)
{
    aos_register_event_filter(EV_WIFI, wifi_service_event, NULL);

	netmgr_init();
	netmgr_start(true);

    aos_loop_run();

    /* never return */
    return 0;
}
