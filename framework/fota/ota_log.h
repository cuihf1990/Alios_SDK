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

#ifndef _OTA_LOG_H_
#define _OTA_LOG_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#undef LOG_TAG
#define LOG_TAG "OTA"

#include "yos/log.h"

#define OTA_LOG_D(format, ...) LOGD(LOG_TAG, format"\r\n",##__VA_ARGS__)
#define OTA_LOG_I(format, ...) LOGI(LOG_TAG, format"\r\n",##__VA_ARGS__)
#define OTA_LOG_W(format, ...) LOGW(LOG_TAG, format"\r\n",##__VA_ARGS__)
#define OTA_LOG_E(format, ...) LOGE(LOG_TAG, format"\r\n",##__VA_ARGS__)

#endif  // _OTA_LOG_H_
