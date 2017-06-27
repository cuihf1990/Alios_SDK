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

#ifndef UMESH_80211_H
#define UMESH_80211_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <umesh_hal.h>

#define MESH_DATA_OFF 32

#define OFF_DST 4
#define OFF_SRC 10
#define OFF_BSS 16

int umesh_80211_make_frame(ur_mesh_hal_module_t *module, frame_t *frame, mac_address_t *dest, void *fpkt);
bool umesh_80211_filter_frame(ur_mesh_hal_module_t *module, uint8_t *pkt, int count);

#ifdef __cplusplus
}
#endif

#endif /* UMESH_80211_H */
