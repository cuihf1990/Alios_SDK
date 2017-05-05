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

#include "mesh_mgmt_tlvs.h"
#include "mesh_mgmt.h"
#include "interfaces.h"
#include "diags.h"
#include "logging.h"

static ur_error_t send_trace_route_response(sid_t *sid, uint32_t timestamp);

static ur_error_t handle_trace_route_request(uint8_t *payload, uint16_t length,
                                             const mesh_src_t *src,
                                             const mac_address_t *dest)
{
    ur_error_t error = UR_ERROR_NONE;
    uint8_t *tlvs;
    uint16_t tlvs_length;
    sid_t sid;
    mm_timestamp_tv_t *timestamp;

    if (mm_get_device_state() < DEVICE_STATE_DETACHED) {
        return UR_ERROR_NONE;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle trace route request\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    timestamp = (mm_timestamp_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TIMESTAMP);
    sid.sid = src->sid;
    sid.meshnetid = src->meshnetid;
    error = send_trace_route_response(&sid, timestamp->timestamp);

    return error;
}

static ur_error_t handle_trace_route_response(uint8_t *payload, uint16_t length,
                                              const mesh_src_t *src,
                                              const mac_address_t *dest)
{
    uint8_t *tlvs;
    uint16_t tlvs_length;
    mm_timestamp_tv_t *timestamp;
    uint32_t time;

    if (mm_get_device_state() < DEVICE_STATE_DETACHED) {
        return UR_ERROR_NONE;
    }

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM, "handle trace route response\r\n");

    tlvs = payload + sizeof(mm_header_t);
    tlvs_length = length - sizeof(mm_header_t);

    timestamp = (mm_timestamp_tv_t *)mm_get_tv(tlvs, tlvs_length, TYPE_TIMESTAMP);
    time = ur_get_now() - timestamp->timestamp;
    ur_log(UR_LOG_LEVEL_INFO, UR_LOG_REGION_MM, "%04x:%04x, time %d ms\r\n",
                                                 src->meshnetid, src->sid, time);
    return UR_ERROR_NONE;
}

ur_error_t send_trace_route_request(sid_t *dest)
{
    ur_error_t        error = UR_ERROR_NONE;
    network_context_t *network;
    message_t         *message;
    mm_header_t       *mm_header;
    mm_timestamp_tv_t *timestamp;
    mesh_dest_t       *mesh_dest;
    uint8_t           *data;
    uint16_t          length;

    length = sizeof(mm_header_t) + sizeof(mm_timestamp_tv_t);
    network = get_network_context_by_meshnetid(dest->meshnetid);
    if (network == NULL) {
        network = get_default_network_context();
    }
    message = message_alloc(network, length);
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

    mesh_dest = message->dest;
    mesh_dest->flags = INSERT_DESTNETID_FLAG;
    mesh_dest->dest.len = 2;
    mesh_dest->dest.short_addr = dest->sid;
    mesh_dest->meshnetid = dest->meshnetid;
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send trace route request, len %d\r\n", length);
    return error;
}

static ur_error_t send_trace_route_response(sid_t *sid, uint32_t src_timestamp)
{
    ur_error_t        error = UR_ERROR_NONE;
    network_context_t *network;
    message_t         *message;
    mm_header_t       *mm_header;
    mm_timestamp_tv_t *timestamp;
    mesh_dest_t       *mesh_dest;
    uint8_t           *data;
    uint16_t          length;

    length = sizeof(mm_header_t) + sizeof(mm_timestamp_tv_t);
    network = get_network_context_by_meshnetid(sid->meshnetid);
    if (network == NULL) {
        network = get_default_network_context();
    }
    message = message_alloc(network, length);
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

    mesh_dest = message->dest;
    mesh_dest->flags = INSERT_DESTNETID_FLAG;
    mesh_dest->dest.len = 2;
    mesh_dest->dest.short_addr = sid->sid;
    mesh_dest->meshnetid = sid->meshnetid;
    error = mf_send_command(mm_header->command, message);

    ur_log(UR_LOG_LEVEL_DEBUG, UR_LOG_REGION_MM,
           "send trace route response, len %d\r\n", length);
    return error;
}

ur_error_t handle_diags_command(message_t *message, const mesh_src_t *src,
                                const mac_address_t *dest, bool dest_reached)
{
    ur_error_t  error = UR_ERROR_NONE;
    uint8_t     *payload;
    uint16_t    length;
    mm_header_t *mm_header;

    payload = message_get_payload(message);
    length = message_get_msglen(message);
    mm_header = (mm_header_t *)payload;

    switch (mm_header->command & COMMAND_COMMAND_MASK) {
        case COMMAND_TRACE_ROUTE_REQUEST:
            error = handle_trace_route_request(payload, length, src, dest);
            break;
        case COMMAND_TRACE_ROUTE_RESPONSE:
            if (dest_reached) {
                error = handle_trace_route_response(payload, length, src, dest);
            }
            break;
        default:
            error = UR_ERROR_FAIL;
            break;
    }

    return error;
}
