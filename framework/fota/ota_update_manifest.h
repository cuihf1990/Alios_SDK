#ifndef OTA_UPDATE_MANIFEST_H_
#define OTA_UPDATE_MANIFEST_H_
#include <stdint.h>
#include "ota_transport.h"

typedef int (*write_flash_cb_t)(int32_t writed_size, uint8_t *buf,
                                int32_t buf_len, int type);

typedef int (*ota_finish_cb_t)(int32_t finish_result, const char *updated_version);


int8_t ota_do_update_packet(ota_response_params *response_parmas, ota_request_params *request_parmas,
                            write_flash_cb_t func, ota_finish_cb_t fcb);

int8_t ota_cancel_update_packet(ota_response_params *response_parmas);

int8_t ota_post_version_msg(void);

#endif /* OTA_UPDATE_MANIFEST_H_ */
