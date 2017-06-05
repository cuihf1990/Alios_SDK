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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "ota_transport.h"

/*
 *  "md5":"6B21342306D0F619AF97006B7025D18A",
    "resourceUrl":"http://otalink.alicdn.com/ALINKTEST_LIVING_LIGHT_ALINK_TEST/v2.0.0.1/uthash-master.zip",
    "size":"265694",
    "uuid":"35C858CFCD6A1FF9F734D749033A29A0",
    "version":"v2.0.0.1","zip":"0"
*/

int8_t parse_ota_requset(const char* request, int *buf_len, ota_request_params * request_parmas)
{
    return 0;
}

int8_t parse_ota_response(const char* response, int buf_len,ota_response_params * response_parmas)
{
    return 0;
}

int8_t ota_pub_request(ota_request_params *request_parmas)
{
   return 0;
}

int8_t ota_sub_request_reply(message_arrived *msgCallback)
{
    return 0;
}

int8_t ota_sub_upgrade(message_arrived *msgCallback)
{
    return 0;
}


int8_t ota_cancel_upgrade(message_arrived *msgCallback)
{
    return 0;
}



void free_global_topic()
{
}

char *ota_get_id()
{
   return NULL;
}





