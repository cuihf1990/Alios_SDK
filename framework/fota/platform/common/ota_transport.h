#ifndef OTA_TRANSPORT_H_
#define OTA_TRANSPORT_H_
#include <stdint.h>

typedef struct {
    const char *primary_version;
    const char *product_type;
    const char *product_internal_type;
    const char *system;
    const char *device_uuid;
} ota_request_params;

#define MAX_URL_LEN 256
#define MAX_MD5_LEN 34
#define MAX_VERSION_LEN 64
#define MAX_ID_LEN 64

typedef struct {
    char primary_version[MAX_VERSION_LEN];
    const char *product_type;
    char download_url[MAX_URL_LEN];
    int frimware_size;
    char device_uuid[MAX_ID_LEN];
    char md5[MAX_MD5_LEN];
} ota_response_params;

typedef void message_arrived(const char *msg);

int8_t parse_ota_requset(const char *request, int *buf_len, ota_request_params *request_parmas);

int8_t parse_ota_response(const char *buf, int buf_len, ota_response_params *response_parmas);

int8_t parse_ota_cancel_response(const char *response, int buf_len, ota_response_params *response_parmas);

int8_t ota_cancel_upgrade(message_arrived *msgCallback);

int8_t ota_sub_upgrade(message_arrived *msgCallback);

int8_t ota_pub_request(ota_request_params *request_parmas);

int8_t ota_sub_request_reply(message_arrived *msgCallback);

void free_global_topic();

char *ota_get_id();

int8_t platform_ota_status_post(int status, int percent);

int8_t platform_ota_result_post(void);

void platform_ota_set_version(char *version);

const char *platform_ota_get_version();

const char *platform_get_main_version();

const char *platform_get_dev_version();

void platform_set_dev_version(const char *dev_version);

#endif /* OTA_TRANSPORT_H_ */
