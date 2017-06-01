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

#ifndef WSF_CLIENT_H
#define WSF_CLIENT_H
#include "wsf_list.h"
#include "wsf_defines.h"
#include "os.h"
/**
 *
 */
typedef wsf_response_t* (*wsf_push_callback)(void *arg, int length);

typedef void (*wsf_error_callback)(wsf_code error_code);

wsf_code wsf_config(config_opt opt, void *value);

wsf_code wsf_init();

wsf_code wsf_shutdown();

/**
 * return NULL if wsf client not initialized or already shutdown
 */
wsf_response_t *wsf_invoke(const char *service_name, wsf_list_t *parameters, int timeout);

wsf_code wsf_register_push_callback(wsf_push_callback callback);

void wsf_wait();

wsf_code wsf_response_destroy(wsf_response_t *response, int free_data);

wsf_code wsf_set_secret_key(const char *secret_key);

wsf_code wsf_register_error_callback(wsf_error_callback callback);

#endif
