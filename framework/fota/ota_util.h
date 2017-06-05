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

#ifndef _OTA_UTIL_H_
#define _OTA_UTIL_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include "ota_constants.h"
typedef enum OTA_STATUS {
    E_OTA_IDLE,
    E_OTA_HOLD,
    E_OTA_DOWNLOAD_SUC,
    E_OTA_DOWNLOAD_FAIL,
    E_OTA_END
} OTA_STATUS_T;

void ota_set_status(OTA_STATUS_T status);
OTA_STATUS_T ota_get_status(void);

#ifdef __cplusplus
}
#endif

#endif
