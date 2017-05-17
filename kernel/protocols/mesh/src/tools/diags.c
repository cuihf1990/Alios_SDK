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

#include "core/mesh_mgmt_tlvs.h"
#include "core/mesh_mgmt.h"
#include "utilities/logging.h"
#include "hal/interfaces.h"
#include "tools/diags.h"

static ur_error_t send_trace_route_response(network_context_t *network,
                                            ur_addr_t *dest,
                                            uint32_t timestamp);

static ur_error_t handle_trace_route_request(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
    uint8_t *tlvs;
    uint16_t tlvs_length;
    mm_timestamp_tv_t *timestamp;
    message_info_t *info;
    network_context_t *network;

    if (mm_get_device_state() < DEVICE_STATE_DETACHED) {
        return UR_ERROR_NONE;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle trace route request\r\n");

    info = message->info;
    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    timestamp = (mm_timestamp_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TIMESTAMP);
    network = get_network_context_by_meshnetid(info->src.netid);
    if (network == NULL) {
        network = get_default_network_context();
    }
    error = send_trace_route_response(network, &info->src, timestamp->timestamp);

    return error;
}

static ur_error_t handle_trace_route_response(message_t *message)
{
    uint8_t *tlvs;
    uint16_t tlvs_length;
    mm_timestamp_tv_t *timestamp;
    uint32_t time;
    message_info_t *info;

    if (mm_get_device_state() < DEVICE_STATE_DETACHED) {
        return UR_ERROR_NONE;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle trace route response\r\n");

    tlvs = message_get_payload(message) + sizeof(mm_header_t);
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);

    timestamp = (mm_timestamp_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TIMESTAMP);
    time = ur_get_now() - timestamp->timestamp;
    info = message->info;
    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM, "%04x:%04x, time %d ms\r\n",
           info->src.netid, info->src.addr.short_addr, time);
    return UR_ERROR_NONE;
}

ur_error_t send_trace_route_request(network_context_t *network,
                                    ur_addr_t *dest)
{
    ur_error_t        error = UR_ERROR_NONE;
    message_t         *message;
    mm_header_t       *mm_header;
    mm_timestamp_tv_t *timestamp;
    message_info_t    *info;
    uint8_t           *data;
    uint16_t          length;

    length = sizeof(mm_header_t) + sizeof(mm_timestamp_tv_t);
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_TRACE_ROUTE_REQUEST;
    data += sizeof(mm_header_t);

    timestamp = (mm_timestamp_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)timestamp, TYPE_TIMESTAMP);
    timestamp->timestamp = ur_get_now();
    data += sizeof(mm_timestamp_tv_t);

    info = message->info;
    info->network = network;
    memcpy(&info->dest, dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send trace route request, len %d\r\n", length);
    return error;
}

static ur_error_t send_trace_route_response(network_context_t *network,
                                            ur_addr_t *dest,
                                            uint32_t src_timestamp)
{
    ur_error_t        error = UR_ERROR_NONE;
    message_t         *message;
    mm_header_t       *mm_header;
    mm_timestamp_tv_t *timestamp;
    message_info_t *info;
    uint8_t           *data;
    uint16_t          length;

    length = sizeof(mm_header_t) + sizeof(mm_timestamp_tv_t);
    message = message_alloc(length);
    if (message == NULL) {
        return UR_ERROR_MEM;
    }
    data = message_get_payload(message);
    mm_header = (mm_header_t *)data;
    mm_header->command = COMMAND_TRACE_ROUTE_RESPONSE;
    data += sizeof(mm_header_t);

    timestamp = (mm_timestamp_tv_t *)data;
    mm_init_tv_base((mm_tv_t *)timestamp, TYPE_TIMESTAMP);
    timestamp->timestamp = src_timestamp;
    data += sizeof(mm_timestamp_tv_t);

    info = message->info;
    info->network = network;
    memcpy(&info->dest, dest, sizeof(info->dest));

    set_command_type(info, mm_header->command);
    error = mf_send_message(message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send trace route response, len %d\r\n", length);
    return error;
}

ur_error_t handle_diags_command(message_t *message, bool dest_reached)
{
    ur_error_t  error = UR_ERROR_NONE;
    mm_header_t *mm_header;

    mm_header = (mm_header_t *)message_get_payload(message);
    switch (mm_header->command & COMMAND_COMMAND_MASK) {
        case COMMAND_TRACE_ROUTE_REQUEST:
            error = handle_trace_route_request(message);
            break;
        case COMMAND_TRACE_ROUTE_RESPONSE:
            if (dest_reached) {
                error = handle_trace_route_response(message);
            }
            break;
        default:
            error = UR_ERROR_FAIL;
            break;
    }

    return error;
}
