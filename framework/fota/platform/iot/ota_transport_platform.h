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
/*
 * ota_transport_platform.h
 *
 *  Created on: 2017年5月17日
 *      Author: ting.guit
 */

#ifndef OTA_TRANSPORT_PLATFORM_H_
#define OTA_TRANSPORT_PLATFORM_H_
#include <stdint.h>

char *ota_get_iotId();

int8_t ota_sub_platform(const char *topic, int len, void *msgCallback);

int8_t ota_pub_platform(const char *topic, const char *pub_info, int len);

int8_t ota_dissub_info(const char *topic);

#endif /* OTA_TRANSPORT_PLATFORM_H_ */
