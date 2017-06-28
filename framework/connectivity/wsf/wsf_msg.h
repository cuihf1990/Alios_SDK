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

#ifndef WSF_MSG_H
#define WSF_MSG_H
#include "wsf_client.h"
#include "wsf_list.h"
#include "yos/list.h"

//NOTE: Must strictly assure that fields address follow the defination of WSF protocol
//and fileds access is aligned.
typedef struct wsf_msg_header_t {
    char magic_code[4];
    uint8_t msg_length[4];
    /* LSB 4bits -- msg_type, MSB 4bits -- version */
    unsigned char msg_type_version;
    char msg_id[4];
} wsf_msg_header_t;

#define set_msg_type_version(msg, type, version) \
do { \
    (msg)->msg_type_version = (type) & 0x0F; \
    (msg)->msg_type_version |= ((version) & 0x0F) << 4; \
} while (0)

#define get_msg_type(msg)       (((msg)->msg_type_version & 0x0F) >> 0)
#define get_msg_version(msg)    (((msg)->msg_type_version & 0xF0) >> 4)

typedef struct wsf_msg_t {
    wsf_msg_header_t header;
    uint8_t payload[0];
} wsf_msg_t;

//the 1st para maybe: wsf_msg_t/wsf_response_t
typedef void *(*wsf_async_cb_t)(void *, void *);
typedef struct wsf_msg_session_t {
    void            *psem;
    wsf_async_cb_t  cb;
    void            *extra;
    wsf_msg_t       *request;
    wsf_msg_t       *response;
    unsigned int    id;
} wsf_msg_session_t;

typedef struct wsf_request_node_t {
    dlist_t list_head;
    wsf_msg_session_t session;
} wsf_request_node_t;

typedef int (*cb_wsf_recv_t)(int , void *);
typedef struct {
    int             sock;
    int             timeout;//ms
    cb_wsf_recv_t   cb_recv;
    yos_call_t      cb_timeout;
    yos_poll_call_t cb_close;
    void            *extra;
} cb_network;


extern void wsf_msg_session_init(wsf_msg_session_t *session);

extern void wsf_msg_session_wait(wsf_msg_session_t *session);

extern int wsf_msg_session_timewait(wsf_msg_session_t *session, int timeout);

extern void wsf_msg_session_signal(wsf_msg_session_t *session);

extern void wsf_msg_session_destroy(wsf_msg_session_t *session);

extern wsf_msg_t *wsf_msg_request_create(const char *servcie_name,
                                         wsf_list_t *param_list);

extern wsf_msg_t *wsf_msg_response_create(int msg_id, invoke_result_code result,
                                          const char *data, int length);

extern void wsf_msg_heartbeat_request_init(wsf_msg_t *hb_req);

/**
 * return 1 if it is a wsf msg, else 0
 */
extern int wsf_msg_accept(const char *buf, int length);

/**
 * return 1 if the msg_lengh in the header is equal to buf length, else 0
 */
extern int wsf_msg_complete(const char *buf, int length);

/**
 * return buf with the header decoded
 */
extern wsf_msg_header_t *wsf_msg_header_decode(const char *buf, int length);
extern wsf_msg_header_t *wsf_msg_header_encode(const char *buf, int length);

extern int wsf_msg_id_get();

extern wsf_msg_t *wsf_msg_register_request_create();

#endif
