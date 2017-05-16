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

#ifndef UR_DIAGS_H
#define UR_DIAGS_H

#include "core/mesh_forwarder.h"
#include "utilities/message.h"

ur_error_t handle_diags_command(message_t *message, bool dest_reached);

ur_error_t send_trace_route_request(network_context_t *network,
                                    ur_addr_t *dest);

#endif  /* UR_DIAGS_H */
