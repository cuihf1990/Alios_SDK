/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>

#include "aos/aos.h"
#include "iot_import.h"
#include "iot_export.h"

#define IOTX_PRE_DTLS_SERVER_URI "coaps://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5684"
#define IOTX_PRE_NOSEC_SERVER_URI "coap://pre.iot-as-coap.cn-shanghai.aliyuncs.com:5683"

#define IOTX_ONLINE_DTLS_SERVER_URL "coaps://%s.iot-as-coap.cn-shanghai.aliyuncs.com:5684"

#define IOTX_PRODUCT_KEY         "vtkkbrpmxmF"
#define IOTX_DEVICE_NAME         "IoTxCoAPTestDev"
#define IOTX_DEVICE_SECRET       "Stk4IUErQUBc1tWRWEKWb5ACra4hFDYF"
#define IOTX_DEVICE_ID           "IoTxCoAPTestDev.1"

int iotx_set_devinfo(iotx_deviceinfo_t *p_devinfo)
{
    if (NULL == p_devinfo) {
        return IOTX_ERR_INVALID_PARAM;
    }

    memset(p_devinfo, 0x00, sizeof(iotx_deviceinfo_t));
    strncpy(p_devinfo->device_id,    IOTX_DEVICE_ID,   IOTX_DEVICE_ID_LEN);
    strncpy(p_devinfo->product_key,  IOTX_PRODUCT_KEY, IOTX_PRODUCT_KEY_LEN);
    strncpy(p_devinfo->device_secret, IOTX_DEVICE_SECRET, IOTX_DEVICE_SECRET_LEN);
    strncpy(p_devinfo->device_name,  IOTX_DEVICE_NAME, IOTX_DEVICE_NAME_LEN);

    return IOTX_SUCCESS;
}

typedef struct ota_device_info {
    const char *product_key;
    const char *device_name;
    void *h_coap;
} OTA_device_info_t;

OTA_device_info_t ota_device_info;

iotx_deviceinfo_t  devinfo;
iotx_coap_context_t *h_coap = NULL;

void coap_start_ota()
{
	IOT_OpenLog("coap-ota");
	IOT_SetLogLevel(IOT_LOG_DEBUG);

    ota_device_info.product_key = devinfo.product_key;
    ota_device_info.device_name = devinfo.device_name;
    ota_device_info.h_coap = h_coap;

	coap_ota_init(&ota_device_info);

	coap_ota_report_version("iotx_ver_1.0.0");

	HAL_SleepMs(2000);

	coap_ota_fetch();

	coap_ota_deinit();

	aos_post_delayed_action(7 * 24 * 60 * 60 * 1000, coap_start_ota, NULL);
}

void coap_client()
{
    iotx_coap_config_t config;

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

    if (NULL == h_coap) {
        printf("%s|%03d :: initialize CoAP failed \n", __func__, __LINE__);
        return;
    }

    IOT_CoAP_DeviceNameAuth(h_coap);

	coap_start_ota();
}


int application_start(void)
{
	coap_client();

	aos_loop_run();
    return 0;
}
