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

#ifndef SERVICE_MANAGER_H
#define SERVICE_MANAGER_H

#include "service.h"
#ifdef __cplusplus
extern "C" {
#endif

int sm_init(void);
int sm_exit(void);
int sm_add_service(service_t *);
int sm_del_service(service_t *);
int sm_get_service_state(service_t *);
int sm_query_service(void);
service_t *sm_get_service(char *);
int sm_attach_service(char *, service_cb);
int sm_detach_service(char *, service_cb);
const char *sm_code2string(int);
#ifdef __cplusplus
}
#endif

#endif
