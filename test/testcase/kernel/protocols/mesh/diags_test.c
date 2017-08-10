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

#include <yunit.h>
#include <yos/framework.h>
#include <yos/kernel.h>

#include "core/mesh_mgmt.h"
#include "tools/diags.h"
#include "hal/interfaces.h"

void test_diags_case(void)
{
    network_context_t *network;
    ur_addr_t dest;
    message_t *message;
    mm_header_t *mm_header;
    mm_timestamp_tv_t *timestamp;
    message_info_t *info;
    uint8_t *data;
    uint16_t length;

    interface_start();
    network = get_default_network_context();
    dest.netid = 0x100;
    dest.addr.len = SHORT_ADDR_SIZE;
    dest.addr.short_addr = 0x1000;
    YUNIT_ASSERT(UR_ERROR_NONE == send_trace_route_request(network, &dest));

    length = sizeof(mm_header_t) + sizeof(mm_timestamp_tv_t);
    message = message_alloc(length, UT_MSG);
    if (message == NULL) {
        return;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_TRACE_ROUTE_RESPONSE;
    data += sizeof(mm_header_t);

    timestamp = (mm_timestamp_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)timestamp, TYPE_TIMESTAMP);
    timestamp->timestamp = 10;
    data += sizeof(mm_timestamp_tv_t);

    info = message->info;
    info->network = network;
    memcpy(&info->dest, &dest, sizeof(info->dest));
    set_command_type(info, mm_header->command);

    YUNIT_ASSERT(UR_ERROR_NONE == handle_diags_command(message, true));
    message_free(message);

    length = sizeof(mm_header_t) + sizeof(mm_timestamp_tv_t);
    message = message_alloc(length, UT_MSG);
    if (message == NULL) {
        return;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_TRACE_ROUTE_REQUEST;
    data += sizeof(mm_header_t);

    timestamp = (mm_timestamp_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)timestamp, TYPE_TIMESTAMP);
    timestamp->timestamp = ur_get_now();
    data += sizeof(mm_timestamp_tv_t);

    info = message->info;
    info->network = network;
    memcpy(&info->dest, &dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);

    YUNIT_ASSERT(UR_ERROR_NONE == handle_diags_command(message, true));
    message_free(message);
    interface_stop();
}
