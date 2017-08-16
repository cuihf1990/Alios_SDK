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

#ifndef UR_LWIP_ADAPTER_H
#define UR_LWIP_ADAPTER_H

#include "lwip/pbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

ur_error_t ur_adapter_interface_init(void);

ur_error_t ur_adapter_interface_up(void);
ur_error_t ur_adapter_interface_down(void);
ur_error_t ur_adapter_interface_update(void);

#ifdef __cplusplus
}
#endif

#endif  /* UR_LWIP_ADAPTER_H */
