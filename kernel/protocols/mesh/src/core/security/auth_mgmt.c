/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <string.h>

#include "aos/aos.h"
#include "core/auth_mgmt.h"
#include "core/auth_relay.h"
#include "core/link_mgmt.h"
#include "core/mesh_mgmt.h"
#include "core/mesh_forwarder.h"
#include "core/topology.h"
#include "hal/interfaces.h"
#include "umesh_utils.h"
#include "umesh_types.h"

auth_context_t g_auth_context = {
    .is_auth_enable   = false,
    .is_auth_busy     = false,
    .is_auth_success  = false,
    .local_id2_len    = TFS_ID2_LEN,
    .auth_code_len    = TFS_AUTH_CODE_LEN,
    .peer_auth_result = false,
    .auth_candidate   = NULL,
    .auth_retry_times = 0,
};
socket_t udp_sock;

static void handle_auth_timer(void *args);
static void handle_udp_socket(const uint8_t *payload, uint16_t length)
{
    uint8_t cmd;
    uint8_t *cur = (uint8_t *)payload;
    uint8_t src[EXT_ADDR_SIZE];
    uint8_t challenge[TFS_CHALLENGE_LEN];
    const mac_address_t *mac = umesh_mm_get_mac_address();
    network_context_t *network = get_default_network_context();

    if (length == 0) {
        return;
    }

    MESH_LOG_DEBUG("recv socket from sp server");
    MESH_LOG_DEBUG("src mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                  mac->addr[0], mac->addr[1], mac->addr[2],
                  mac->addr[3], mac->addr[4], mac->addr[5]);

    // src
    if (umesh_mm_get_device_state() == DEVICE_STATE_LEADER ||
        g_auth_context.is_auth_success) {
        memcpy(g_auth_context.peer.addr.addr, cur, EXT_ADDR_SIZE);

        MESH_LOG_DEBUG("peer src mac: %02x:%02x:%02x:%02x:%02x:%02x\n",
                       g_auth_context.peer.addr.addr[0], g_auth_context.peer.addr.addr[1],
                       g_auth_context.peer.addr.addr[2], g_auth_context.peer.addr.addr[3],
                       g_auth_context.peer.addr.addr[4], g_auth_context.peer.addr.addr[5]);
    }
    memcpy(src, cur, EXT_ADDR_SIZE);
    cur += EXT_ADDR_SIZE;
    length -= EXT_ADDR_SIZE;

    // dst
    cur += EXT_ADDR_SIZE;
    length -= EXT_ADDR_SIZE;

    // cmd
    cmd = *cur++;
    length--;

    switch (cmd) {
        case ID2_AUTH_CHALLENGE:
            if (umesh_mm_get_device_state() == DEVICE_STATE_LEADER &&
                (memcmp(mac->addr, src, EXT_ADDR_SIZE) == 0)) {
                MESH_LOG_INFO("sp server -> leader: challenge");
                ur_stop_timer(&g_auth_context.auth_timer, NULL);

                // challenge
                memcpy(challenge, cur, TFS_CHALLENGE_LEN);
                MESH_LOG_DEBUG("challenge: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
                                           %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                               challenge[0], challenge[1], challenge[2], challenge[3],
                               challenge[4], challenge[5], challenge[6], challenge[7],
                               challenge[8], challenge[9], challenge[10], challenge[11],
                               challenge[12], challenge[13], challenge[14], challenge[15],
                               challenge[16], challenge[17], challenge[18], challenge[19],
                               challenge[20], challenge[21], challenge[22], challenge[23],
                               challenge[24], challenge[25], challenge[26], challenge[27],
                               challenge[28], challenge[29], challenge[30], challenge[31]);
 
                // calculate the auth code
                tfs_id2_get_challenge_auth_code(challenge, NULL, 0,
                                                g_auth_context.auth_code,
                                                &g_auth_context.auth_code_len);
 
                mac = umesh_mm_get_mac_address();
                socket_sendmsg(udp_sock.socket, mac->addr, mac->addr, ID2_AUTH_CODE,
                               g_auth_context.auth_code, g_auth_context.auth_code_len,
                               ID2_SERVER_ADDR, ID2_SERVER_PORT);

                MESH_LOG_INFO("leader -> sp server: auth code");

                g_auth_context.auth_state = AUTH_RECV_CHALLENGE;
                ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_relay_interval,
                               handle_auth_timer, NULL);
            } else if (umesh_mm_get_device_state() == DEVICE_STATE_LEADER ||
                       g_auth_context.is_auth_success) {
                MESH_LOG_INFO("sp server -> joiner router: challenge");

                // challenge
                memcpy(g_auth_context.id2_challenge, cur, length);
                MESH_LOG_DEBUG("challenge: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
                                           %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                               g_auth_context.id2_challenge[0], g_auth_context.id2_challenge[1], g_auth_context.id2_challenge[2],
                               g_auth_context.id2_challenge[3], g_auth_context.id2_challenge[4], g_auth_context.id2_challenge[5],
                               g_auth_context.id2_challenge[6], g_auth_context.id2_challenge[7], g_auth_context.id2_challenge[8],
                               g_auth_context.id2_challenge[9], g_auth_context.id2_challenge[10], g_auth_context.id2_challenge[11],
                               g_auth_context.id2_challenge[12], g_auth_context.id2_challenge[13], g_auth_context.id2_challenge[14],
                               g_auth_context.id2_challenge[15], g_auth_context.id2_challenge[16], g_auth_context.id2_challenge[17],
                               g_auth_context.id2_challenge[18], g_auth_context.id2_challenge[19], g_auth_context.id2_challenge[20],
                               g_auth_context.id2_challenge[21], g_auth_context.id2_challenge[22], g_auth_context.id2_challenge[23],
                               g_auth_context.id2_challenge[24], g_auth_context.id2_challenge[25], g_auth_context.id2_challenge[26],
                               g_auth_context.id2_challenge[27], g_auth_context.id2_challenge[28], g_auth_context.id2_challenge[29],
                               g_auth_context.id2_challenge[30], g_auth_context.id2_challenge[31]);
 
                // send challenge to joiner
                ur_stop_timer(&g_auth_context.auth_timer, NULL);
                g_auth_context.auth_state = AUTH_RELAY_CHALLENGE;
                send_auth_relay(network, &g_auth_context.peer);
                ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_relay_interval,
                               handle_auth_timer, NULL);
            }
            break;

        case ID2_AUTH_RESULT:
            if (umesh_mm_get_device_state() == DEVICE_STATE_LEADER &&
                (memcmp(mac->addr, src, EXT_ADDR_SIZE) == 0)) {
                MESH_LOG_INFO("sp server -> leader: auth result");
                ur_stop_timer(&g_auth_context.auth_timer, NULL);

                // auth result
                g_auth_context.is_auth_success = *cur;
                g_auth_context.auth_state = AUTH_DONE;
                g_auth_context.auth_candidate = NULL;
                g_auth_context.auth_handler(NULL, g_auth_context.is_auth_success);
            } else if (umesh_mm_get_device_state() == DEVICE_STATE_LEADER ||
                       g_auth_context.is_auth_success) {
 
                MESH_LOG_INFO("sp server -> joiner router: joiner's auth result");

                // auth result
                g_auth_context.peer_auth_result = *cur;

                // send auth result to joiner
                ur_stop_timer(&g_auth_context.auth_timer, NULL);
                g_auth_context.auth_state = AUTH_RELAY_AUTH_RESULT;
                send_auth_response(network, &g_auth_context.peer);
                ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_response_interval,
                               handle_auth_timer, NULL);
            }
            break;

         default:
            MESH_LOG_DEBUG("unknown cmd\n");
            break;
    }
}

static void handle_auth_timer(void *args)
{
    network_context_t *network = get_default_network_context();

    MESH_LOG_INFO("handle auth timer");

    switch (g_auth_context.auth_state) {
        case AUTH_REQUEST_START:
            if (g_auth_context.auth_retry_times < AUTH_REQUEST_RETRY_TIMES) {
                ++g_auth_context.auth_retry_times;
                if (umesh_mm_get_device_state() == DEVICE_STATE_LEADER) {
                    const mac_address_t *mac;
                    mac = umesh_mm_get_mac_address();

                    // send id2 to sp server
                    socket_sendmsg(udp_sock.socket, mac->addr, mac->addr,
                                   ID2_AUTH_REQUEST, g_auth_context.local_id2, TFS_ID2_LEN,
                                   ID2_SERVER_ADDR, ID2_SERVER_PORT);

                    MESH_LOG_INFO("leader -> sp server: id2");
                } else if (!g_auth_context.is_auth_busy) {
                    MESH_LOG_INFO("joiner -> joiner router: auth request");
                    send_auth_request(network);
                    g_auth_context.is_auth_busy = true;
                }

                ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_request_interval,
                               handle_auth_timer, NULL);
            }
            break;

        case AUTH_RELAY_CHALLENGE:
            if (g_auth_context.auth_retry_times < AUTH_RELAY_RETRY_TIMES) {
                ++g_auth_context.auth_retry_times;
                send_auth_relay(network, &g_auth_context.peer);
                ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_relay_interval,
                               handle_auth_timer, NULL);
            }
            break;

        case AUTH_RELAY_AUTH_CODE:
            if (g_auth_context.auth_retry_times < AUTH_RELAY_RETRY_TIMES) {
                ++g_auth_context.auth_retry_times;
                send_auth_relay(network, NULL);
                ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_relay_interval,
                               handle_auth_timer, NULL);
            }
            break;

        case AUTH_RELAY_AUTH_RESULT:
            if (g_auth_context.auth_retry_times < AUTH_RELAY_RETRY_TIMES) {
                ++g_auth_context.auth_retry_times;
                send_auth_response(network, &g_auth_context.peer);
                ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_response_interval,
                               handle_auth_timer, NULL);
            }
            break;

        case AUTH_RECV_CHALLENGE:
             if (g_auth_context.auth_retry_times < AUTH_REQUEST_RETRY_TIMES) {
                ++g_auth_context.auth_retry_times;

                const mac_address_t *mac;
                mac = umesh_mm_get_mac_address();

                socket_sendmsg(udp_sock.socket, mac->addr, mac->addr,ID2_AUTH_CODE,
                               g_auth_context.auth_code, g_auth_context.auth_code_len,
                               ID2_SERVER_ADDR, ID2_SERVER_PORT);

                ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_response_interval,
                               handle_auth_timer, NULL);
                MESH_LOG_INFO("leader -> sp server: auth code");
             }
             break;

        case AUTH_DONE:
        default:
            break;
    }
}

ur_error_t auth_init(void)
{
    memset(g_auth_context.local_id2, 0, sizeof(g_auth_context.local_id2));
    memset(g_auth_context.peer_id2, 0, sizeof(g_auth_context.peer_id2));
    memset(g_auth_context.auth_code, 0, sizeof(g_auth_context.auth_code));

    // read ID2
    tfs_get_ID2(g_auth_context.local_id2, &g_auth_context.local_id2_len);

    MESH_LOG_DEBUG("id2: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\
                         %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                   g_auth_context.local_id2[0], g_auth_context.local_id2[1], g_auth_context.local_id2[2],
                   g_auth_context.local_id2[3], g_auth_context.local_id2[4], g_auth_context.local_id2[5],
                   g_auth_context.local_id2[6], g_auth_context.local_id2[7], g_auth_context.local_id2[8],
                   g_auth_context.local_id2[9], g_auth_context.local_id2[10], g_auth_context.local_id2[11],
                   g_auth_context.local_id2[12], g_auth_context.local_id2[13], g_auth_context.local_id2[14],
                   g_auth_context.local_id2[15], g_auth_context.local_id2[16], g_auth_context.local_id2[17],
                   g_auth_context.local_id2[18], g_auth_context.local_id2[19], g_auth_context.local_id2[20],
                   g_auth_context.local_id2[21], g_auth_context.local_id2[22], g_auth_context.local_id2[23]);
 
    udp_sock.socket = socket_init(handle_udp_socket);
    return UR_ERROR_NONE; 
}

void auth_enable(void)
{
    if (!g_auth_context.is_auth_enable) {
        g_auth_context.is_auth_enable = true;
    }
}

void auth_disable(void)
{
    if (g_auth_context.is_auth_enable) {
        g_auth_context.is_auth_enable = false;
    }
}

auth_state_t get_auth_state(void)
{
    return g_auth_context.auth_state;
}

void set_auth_state(auth_state_t state)
{
    g_auth_context.auth_state = state;
}

bool get_auth_result(void)
{
    return g_auth_context.is_auth_success;
}

neighbor_t *get_auth_candidate(void)
{
    return g_auth_context.auth_candidate;
}

bool is_auth_enabled(void)
{
    return g_auth_context.is_auth_enable;
}

bool is_auth_busy(void)
{
    return g_auth_context.is_auth_busy;
}

ur_error_t start_auth(neighbor_t *nbr, auth_handler_t handler)
{
    uint32_t random;
    network_context_t *network = get_default_network_context();

    if (g_auth_context.is_auth_busy ||
        g_auth_context.auth_candidate ||
        (g_auth_context.auth_state != AUTH_IDLE &&
         g_auth_context.auth_state != AUTH_DONE)) {
         return UR_ERROR_BUSY;
    }

    if (nbr) {
        if (umesh_mm_get_channel(network->hal) != nbr->channel) {
            umesh_mm_set_prev_channel();
            umesh_mm_set_channel(network->hal, nbr->channel);
        }

        network->candidate_meshnetid = nbr->netid;
    }

    if (!g_auth_context.is_auth_enable) {
        return UR_ERROR_FAIL;
    }

    if (g_auth_context.auth_timer) {
        ur_stop_timer(&g_auth_context.auth_timer, NULL);
    }

    g_auth_context.auth_candidate = nbr;
    g_auth_context.auth_state = AUTH_REQUEST_START;

    random = umesh_get_random();
    ur_start_timer(&g_auth_context.auth_timer,
                   (random % network->hal->auth_request_interval + 1),
                   handle_auth_timer, NULL);
    g_auth_context.auth_handler = handler;

    return UR_ERROR_NONE;
}

ur_error_t stop_auth(void)
{
    if (!g_auth_context.is_auth_enable) {
        return UR_ERROR_FAIL;
    }

    if (g_auth_context.auth_timer) {
        ur_stop_timer(&g_auth_context.auth_timer, NULL);
    }

    g_auth_context.is_auth_success = false;
    g_auth_context.is_auth_busy = false;
    return UR_ERROR_NONE;
}

ur_error_t send_auth_request(network_context_t *network)
{
    ur_error_t error = UR_ERROR_NONE;
    uint16_t length;
    mm_node_id2_tv_t *id2;
    uint8_t *data;
    uint8_t *data_orig;
    message_t *message = NULL;
    message_info_t *info;

    length = sizeof(mm_header_t) + sizeof(mm_node_id2_tv_t);
    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);

    // ID2
    id2 = (mm_node_id2_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)id2, TYPE_NODE_ID2);
    memcpy(id2->device_id, g_auth_context.local_id2, TFS_ID2_LEN);
    data += sizeof(mm_node_id2_tv_t);

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_AUTH_REQUEST,
                               data_orig, length, NETWORK_MGMT_2);
    if (message == NULL) {
        ur_mem_free(data_orig, length);
        return UR_ERROR_MEM;
    }

    info = message->info;
    info->network = network;

    if (g_auth_context.auth_candidate) {
        MESH_LOG_DEBUG("send to auth candidate: %04x, netid: %04x", g_auth_context.auth_candidate->sid,
                       g_auth_context.auth_candidate->netid);
        set_mesh_short_addr(&info->dest, g_auth_context.auth_candidate->netid,
                            g_auth_context.auth_candidate->sid);
    } else {
        MESH_LOG_DEBUG("send to network");
        set_mesh_short_addr(&info->dest, network->candidate_meshnetid,
                            BCAST_SID);
    }

    error = mf_send_message(message);
    ur_mem_free(data_orig, length);

    MESH_LOG_INFO("send authenticate request in channel %d, len %d",
                   umesh_mm_get_channel(network->hal), length);

    return error;
}

ur_error_t send_auth_response(network_context_t *network,
                              ur_addr_t *dest)
{
    ur_error_t error = UR_ERROR_NONE;
    message_t *message;
    uint8_t *data;
    uint16_t length;
    message_info_t *info;
    uint8_t *data_orig;
    mm_node_id2_tv_t *id2;
    mm_id2_auth_result_tv_t *id2_auth_result;

    length = sizeof(mm_header_t) + sizeof(mm_node_id2_tv_t) +
             sizeof(mm_id2_auth_result_tv_t);
    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);

    // ID2
    id2 = (mm_node_id2_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)id2, TYPE_NODE_ID2);
    memcpy(id2->device_id, g_auth_context.peer_id2, TFS_ID2_LEN);
    data += sizeof(mm_node_id2_tv_t);

    // auth result
    id2_auth_result = (mm_id2_auth_result_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)id2_auth_result, TYPE_ID2_AUTH_RESULT);
    id2_auth_result->result = g_auth_context.peer_auth_result;
    data += sizeof(mm_id2_auth_result_tv_t);

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_AUTH_RESPONSE,
                               data_orig, length, NETWORK_MGMT_2);
    if (message) {
        info = message->info;
        info->network = network;
        memcpy(&info->dest, dest, sizeof(info->dest));
        info->dest.netid = BCAST_NETID;
        info->dest.addr.len = EXT_ADDR_SIZE;
        error = mf_send_message(message);
    }
    ur_mem_free(data_orig, length);

    MESH_LOG_DEBUG("send authenticate response, len %d", length);

    return error;
}

ur_error_t send_auth_relay(network_context_t *network, ur_addr_t *dest)
{
    ur_error_t error = UR_ERROR_NONE;
    uint16_t length;
    mm_node_id2_tv_t *id2;
    mm_id2_challenge_tv_t *id2_challenge;
    mm_id2_auth_code_tlv_t *id2_auth_code;
    uint8_t *data;
    uint8_t *data_orig;
    message_t *message = NULL;
    message_info_t *info;

    length = sizeof(mm_header_t) + sizeof(mm_node_id2_tv_t);
    if (dest) {
        length += sizeof(mm_id2_challenge_tv_t);
    } else {
        length += sizeof(mm_id2_auth_code_tlv_t) + g_auth_context.auth_code_len;
    }
 
    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);

    // ID2
    id2 = (mm_node_id2_tv_t *)data;
    umesh_mm_init_tv_base((mm_tv_t *)id2, TYPE_NODE_ID2);
    memcpy(id2->device_id, (dest ? g_auth_context.peer_id2 : g_auth_context.local_id2), TFS_ID2_LEN);
    data += sizeof(mm_node_id2_tv_t);

    if (dest) {
        // challenge
        id2_challenge = (mm_id2_challenge_tv_t *)data;
        umesh_mm_init_tv_base((mm_tv_t *)id2_challenge, TYPE_ID2_CHALLENGE);
        memcpy(id2_challenge->challenge, g_auth_context.id2_challenge, TFS_CHALLENGE_LEN);
        data += sizeof(mm_id2_challenge_tv_t);
    } else {
        // auth code
        id2_auth_code = (mm_id2_auth_code_tlv_t *)data;
        umesh_mm_init_tlv_base((mm_tlv_t *)id2_auth_code, TYPE_ID2_AUTH_CODE, g_auth_context.auth_code_len);
        data += sizeof(mm_id2_auth_code_tlv_t);
        memcpy(data, g_auth_context.auth_code, g_auth_context.auth_code_len);
        data += g_auth_context.auth_code_len;
    }

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_AUTH_RELAY,
                               data_orig, length, NETWORK_MGMT_2);
    if (message == NULL) {
        ur_mem_free(data_orig, length);
        return UR_ERROR_MEM;
    }

    info = message->info;
    info->network = network;

    if (dest) {
        memcpy(&info->dest, dest, sizeof(info->dest));
        MESH_LOG_DEBUG("relay dest: %02x:%02x:%02x:%02x:%02x:%02x\n",
                       dest->addr.addr[0], dest->addr.addr[1], dest->addr.addr[2],
                       dest->addr.addr[3], dest->addr.addr[4], dest->addr.addr[5]);
        info->dest.netid = BCAST_NETID;
        info->dest.addr.len = EXT_ADDR_SIZE;
    } else {
        if (g_auth_context.auth_candidate) {
            MESH_LOG_DEBUG("send to auth candidate: %04x, netid: %04x", g_auth_context.auth_candidate->sid,
                           g_auth_context.auth_candidate->netid);
            set_mesh_short_addr(&info->dest, g_auth_context.auth_candidate->netid,
                                g_auth_context.auth_candidate->sid);
        } else {
            MESH_LOG_DEBUG("send to network");
            set_mesh_short_addr(&info->dest, network->candidate_meshnetid,
                                BCAST_SID);
        }
    }

    error = mf_send_message(message);
    ur_mem_free(data_orig, length);

    MESH_LOG_INFO("send authenticate relay in channel %d, len %d",
                   umesh_mm_get_channel(network->hal), length);

    return error;
}

ur_error_t send_auth_ack(network_context_t *network)
{
    ur_error_t error = UR_ERROR_NONE;
    uint16_t length;
    uint8_t *data;
    uint8_t *data_orig;
    message_t *message = NULL;
    message_info_t *info;

    length = sizeof(mm_header_t);
    data = ur_mem_alloc(length);
    if (data == NULL) {
        return UR_ERROR_MEM;
    }
    data_orig = data;
    data += sizeof(mm_header_t);

    message = mf_build_message(MESH_FRAME_TYPE_CMD, COMMAND_AUTH_ACK,
                               data_orig, length, NETWORK_MGMT_2);
    if (message == NULL) {
        ur_mem_free(data_orig, length);
        return UR_ERROR_MEM;
    }

    info = message->info;
    info->network = network;

    if (g_auth_context.auth_candidate) {
        MESH_LOG_DEBUG("send to auth candidate: %04x, netid: %04x", g_auth_context.auth_candidate->sid,
                       g_auth_context.auth_candidate->netid);
        set_mesh_short_addr(&info->dest, g_auth_context.auth_candidate->netid,
                            g_auth_context.auth_candidate->sid);
    } else {
        MESH_LOG_DEBUG("send to network");
        set_mesh_short_addr(&info->dest, network->candidate_meshnetid,
                            BCAST_SID);
    }

    error = mf_send_message(message);
    ur_mem_free(data_orig, length);

    MESH_LOG_INFO("send authenticate ack in channel %d, len %d",
                   umesh_mm_get_channel(network->hal), length);

    return error;
}

ur_error_t handle_auth_request(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
    mm_node_id2_tv_t *id2;
    uint8_t *tlvs;
    uint16_t tlvs_length;
    const mac_address_t *mac;
    message_info_t *info;

    MESH_LOG_INFO("recv auth request");

    info = message->info;
    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    tlvs = ur_mem_alloc(tlvs_length);
    if (tlvs == NULL) {
        return UR_ERROR_MEM;
    }
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);

    if (update_neighbor(info, tlvs, tlvs_length, false) == NULL) {
        error = UR_ERROR_FAIL;
        goto exit;
    }

    id2 = (mm_node_id2_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                              TYPE_NODE_ID2);
    if (id2 == NULL) {
        error = UR_ERROR_FAIL;
        goto exit;
    } else {
        memcpy(g_auth_context.peer_id2, id2->device_id, TFS_ID2_LEN);
    }

    mac = umesh_mm_get_mac_address();

    // send id2 to sp server
    socket_sendmsg(udp_sock.socket, message->info->src_mac.addr.addr, mac->addr,
                   ID2_AUTH_REQUEST, id2->device_id, TFS_ID2_LEN,
                   ID2_SERVER_ADDR, ID2_SERVER_PORT);
 
    MESH_LOG_INFO("send auth request to sp server");

exit:
    ur_mem_free(tlvs, tlvs_length);
    return error;
}

ur_error_t handle_auth_response(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
    mm_node_id2_tv_t *id2;
    mm_id2_auth_result_tv_t *id2_auth_result;
    uint8_t *tlvs;
    uint16_t tlvs_length;
    network_context_t *network;
    message_info_t *info;

    MESH_LOG_INFO("recv the auth response message");

    info = message->info;
    network = (network_context_t *)info->network;
    send_auth_ack(network);

    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    tlvs = ur_mem_alloc(tlvs_length);
    if (tlvs == NULL) {
        return UR_ERROR_MEM;
    }
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);

    if (update_neighbor(info, tlvs, tlvs_length, false) == NULL) {
        error = UR_ERROR_FAIL;
        goto exit;
    }

    id2 = (mm_node_id2_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                              TYPE_NODE_ID2);
    // check ID2
    if (memcmp(g_auth_context.local_id2, id2->device_id, TFS_ID2_LEN) != 0) {
        error = UR_ERROR_FAIL;
        goto exit;
    }

    id2_auth_result = (mm_id2_auth_result_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                                 TYPE_ID2_AUTH_RESULT);
    if (id2_auth_result != NULL) {
        g_auth_context.is_auth_success = id2_auth_result->result;
        g_auth_context.auth_handler(g_auth_context.auth_candidate, id2_auth_result->result);
    }
    
    ur_stop_timer(&g_auth_context.auth_timer, NULL);
    //set_auth_state(AUTH_DONE);
    //network->auth_candidate = NULL;
    g_auth_context.is_auth_busy = false;

exit:
    ur_mem_free(tlvs, tlvs_length);
    return error;
}

ur_error_t handle_auth_ack(message_t *message)
{
    MESH_LOG_INFO("recv auth ack");

    set_auth_state(AUTH_DONE);
    //network_context_t *network = get_default_network_context();
    ur_stop_timer(&g_auth_context.auth_timer, NULL);
    return UR_ERROR_NONE;
}

ur_error_t handle_auth_relay(message_t *message)
{
    ur_error_t error = UR_ERROR_NONE;
    mm_node_id2_tv_t *id2;
    mm_id2_challenge_tv_t *id2_challenge;
    mm_id2_auth_code_tlv_t *id2_auth_code;
    uint8_t *tlvs;
    uint16_t tlvs_length;
    network_context_t *network;
    message_info_t *info;
    bool joiner = false;

    info = message->info;
    network = (network_context_t *)info->network;

    if (!get_auth_result() &&
        umesh_mm_get_device_state() != DEVICE_STATE_LEADER) {
        joiner = true;
    }

    MESH_LOG_INFO("recv the auth relay message");

    tlvs_length = message_get_msglen(message) - sizeof(mm_header_t);
    tlvs = ur_mem_alloc(tlvs_length);
    if (tlvs == NULL) {
        return UR_ERROR_MEM;
    }
    message_copy_to(message, sizeof(mm_header_t), tlvs, tlvs_length);
 
    if (update_neighbor(info, tlvs, tlvs_length, false) == NULL) {
        error = UR_ERROR_FAIL;
        goto exit;
    }

    // found id2_auth_code and act as joiner router
    if (!joiner &&
        (id2_auth_code = (mm_id2_auth_code_tlv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                                   TYPE_ID2_AUTH_CODE))) {
        const mac_address_t *mac;
        mac = umesh_mm_get_mac_address();
        uint8_t *data = (uint8_t *)id2_auth_code + sizeof(mm_tlv_t);
        
        // send joiner's auth code to sp server
        socket_sendmsg(udp_sock.socket, info->src_mac.addr.addr, mac->addr,
                       ID2_AUTH_CODE, data, id2_auth_code->base.length,
                       ID2_SERVER_ADDR, ID2_SERVER_PORT);

        MESH_LOG_INFO("joiner router -> sp server: joiner's auth code");

        error = UR_ERROR_NONE;
        goto exit;
    }

    id2 = (mm_node_id2_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                              TYPE_NODE_ID2);
    // check ID2
    if (joiner && (memcmp(g_auth_context.local_id2, id2->device_id, TFS_ID2_LEN) == 0)) {
        id2_challenge = (mm_id2_challenge_tv_t *)umesh_mm_get_tv(tlvs, tlvs_length,
                                                                 TYPE_ID2_CHALLENGE);
        MESH_LOG_INFO("joiner router -> joiner: challenge");

        if (id2_challenge) {
            // calculate the auth code
            tfs_id2_get_challenge_auth_code(id2_challenge->challenge, NULL, 0,
                                            g_auth_context.auth_code,
                                            &g_auth_context.auth_code_len);
            MESH_LOG_INFO("auth code len: %d\n", g_auth_context.auth_code_len);
        } else {
            error = UR_ERROR_FAIL;
            goto exit;
        }

        ur_stop_timer(&g_auth_context.auth_timer, NULL);
        set_auth_state(AUTH_RELAY_AUTH_CODE);
        send_auth_relay(network, NULL);
        ur_start_timer(&g_auth_context.auth_timer, network->hal->auth_relay_interval,
                       handle_auth_timer, NULL);

        MESH_LOG_INFO("joiner -> joiner router: auth code");
    }

exit:
    ur_mem_free(tlvs, tlvs_length);
    return error;
}
