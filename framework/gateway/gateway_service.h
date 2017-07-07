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

#ifndef GATEWAY_SERVICE_H
#define GATEWAY_SERVICE_H

#include <yos/types.h>
#include <yos/framework.h>

#ifndef bool
#define bool unsigned char
#endif

int gateway_service_init(void);
void gateway_service_deinit(void);
int gateway_service_start(void);
void gateway_service_stop(void);
bool gateway_service_get_mesh_mqtt_state(void);

#endif
