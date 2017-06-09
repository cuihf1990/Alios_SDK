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

#ifndef OTA_UPDATE_MANIFEST_H_
#define OTA_UPDATE_MANIFEST_H_
#include <stdint.h>
#include "ota_transport.h"

typedef int (*write_flash_cb_t)(int32_t writed_size, uint8_t *buf,
                                  int32_t buf_len, int type);

typedef int (*ota_finish_cb_t)(int32_t finish_result, const char *updated_version);


int8_t ota_do_update_packet(ota_response_params *response_parmas,ota_request_params *request_parmas,
                               write_flash_cb_t func, ota_finish_cb_t fcb);

int8_t ota_cancel_update_packet(ota_response_params *response_parmas);

#endif /* OTA_UPDATE_MANIFEST_H_ */
