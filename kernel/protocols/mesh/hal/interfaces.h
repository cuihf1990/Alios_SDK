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

#ifndef UR_INTERFACES_H
#define UR_INTERFACES_H

void interface_init(void);
void interface_start(void);
void interface_stop(void);
void interface_deinit(void);
void reset_network_context(void);
slist_t *get_network_contexts(void);
network_context_t *get_default_network_context(void);
network_context_t *get_sub_network_context(hal_context_t *hal);
network_context_t *get_hal_default_network_context(hal_context_t *hal);
network_context_t *get_network_context_by_meshnetid(uint16_t meshnetid);
slist_t *get_hal_contexts(void);
hal_context_t *get_default_hal_context(void);
hal_context_t *get_hal_context(media_type_t type);

#endif  /* UR_INTERFACES_H */
