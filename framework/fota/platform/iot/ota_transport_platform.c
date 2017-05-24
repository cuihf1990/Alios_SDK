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
/**
* @author E-mail: ting.guit@alibaba-inc.com
* @version time：2017年5月17日 下午6:18:33
* ota_transport_platform.c
*/

#include "ota_transport_platform.h"
#include "stdio.h"

extern char *get_clientID();

extern void *get_iot_handler();

char * ota_get_iotId()
{
    return NULL;
}

static void * ota_get_iot_handler()
{
    return NULL;
}

int8_t ota_sub_platform(const char *topic, int len, void *msgCallback)
{
    //IOT_subscribe(ota_get_iot_handler(), topic);
    return 0;
}

int8_t ota_pub_platform(const char *topic, const char *pub_info, int len)
{
    //IOT_publish(ota_get_iot_handler(), topic ,(const uint8_t *)pub_info, len);
    return 0;
}

int8_t ota_dissub_info(const char *topic)
{
    return 0;
}
