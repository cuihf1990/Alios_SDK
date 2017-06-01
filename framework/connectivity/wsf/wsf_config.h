/*
 * Copyright (C) 2017 YunOS Project. All rights reserved.
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

#ifndef WSF_CONFIG_H
#define WSF_CONFIG_H

#include "wsf_defines.h"


#ifdef  __cplusplus
extern "C"
{
#endif

typedef struct wsf_config_t {
    int   heartbeat_interval;
    unsigned short server_port;
    unsigned short version;
    int   request_timeout;  //seconds
    int   max_msg_queue_length;
    int   max_msg_recv_length;
    int   enable_ssl;
    int   ssl_version;
    int   user_heartbeat_interval;
    char  *server_name;
    const char  *global_ca_str;
    unsigned int ca_str_len;
} wsf_config_t;

extern wsf_config_t *wsf_get_config();

extern wsf_code wsf_init_config();

extern wsf_code wsf_destroy_config();

#ifdef  __cplusplus
}
#endif

#endif
