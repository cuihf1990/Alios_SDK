/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */
#ifndef OTA_TRANSPORT_H_
#define OTA_TRANSPORT_H_
#include <stdint.h>
#include <yos/yos.h>

#define MAX_URL_LEN 256
#define MAX_MD5_LEN 34
#undef MAX_VERSION_LEN
#define MAX_VERSION_LEN 64
#define MAX_ID_LEN 64

typedef struct {
    const char *primary_version;
    const char *secondary_version;
    const char *product_type;
    const char *device_uuid;
} ota_request_params;

typedef struct {
    char primary_version[MAX_VERSION_LEN];
    char secondary_version[MAX_VERSION_LEN];
    const char *product_type;
    char download_url[MAX_URL_LEN];
    int frimware_size;
    char device_uuid[MAX_ID_LEN];
    char md5[MAX_MD5_LEN];
} ota_response_params;


int8_t parse_ota_requset(const char *request, int *buf_len, ota_request_params *request_parmas);

int8_t parse_ota_response(const char *buf, int buf_len, ota_response_params *response_parmas);

int8_t parse_ota_cancel_response(const char *response, int buf_len, ota_response_params *response_parmas);

int8_t ota_sub_upgrade(yos_cloud_cb_t msgCallback);

int8_t ota_cancel_upgrade(yos_cloud_cb_t msgCallback);

int8_t ota_pub_request(ota_request_params *request_parmas);

int8_t ota_sub_request_reply(yos_cloud_cb_t msgCallback);

void free_global_topic();

int8_t platform_ota_status_post(int status, int percent);

int8_t platform_ota_result_post(void);

char *ota_get_id();
#endif /* OTA_TRANSPORT_H_ */
