/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <aos/aos.h>
#include "ota_log.h"

#include "iot_import.h"
#include "iot_export.h"

typedef struct ota_device_info {
    const char *product_key;
    const char *device_name;
    void *h_coap;
} OTA_device_info;

OTA_device_info g_ota_device_info;

void *h_ota = NULL;

void coap_ota_init( void *signal)
{
    if (signal == NULL) {
        OTA_LOG_E("ota device info is null");
        return;
    }
    OTA_device_info *device_info = (OTA_device_info *)signal;
    OTA_LOG_D("device_info:%s,%s", device_info->product_key, device_info->device_name);
    memcpy(&g_ota_device_info, device_info , sizeof (OTA_device_info));

	h_ota = IOT_OTA_Init(g_ota_device_info.product_key, g_ota_device_info.device_name,
			g_ota_device_info.h_coap);

	if (NULL == h_ota) {
        OTA_LOG_E("initialize OTA failed");
    }
}

void coap_ota_report_version(char *version)
{
    int ota_code = 0;
    do {
        ota_code = IOT_OTA_ReportVersion(h_ota, version);
        IOT_CoAP_Yield(g_ota_device_info.h_coap);
        HAL_SleepMs(2000);
    } while (0 != ota_code);

}

void coap_ota_fetch()
{
    try_fetch_ota(h_ota);
}

void coap_ota_deinit()
{
    if (NULL != h_ota) {
        IOT_OTA_Deinit(h_ota);
    }
}
