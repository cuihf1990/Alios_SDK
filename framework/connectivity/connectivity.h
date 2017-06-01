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

#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include "yos/list.h"
#include "os.h"
#include "yos/log.h"

#define NAME_LENGTH 12

enum CONNECT_CODE {
    CONNECT_UNKNOWN      = -2,
    CONNECT_RESULT_ERR   = -1,
    CONNECT_RESULT_OK    =  0,

    CONNECT_CODE_BEGIN   = 0x0200,
    CONNECT_EVENT,
    CONNECT_DATA,
    CONNECT_BIND,
    CONNECT_RELEASE,
    CONNECT_SEND_FAIL,
    CONNECT_SEND_TIMEOUT,
    CONNECT_STATE_OPEN,
    CONNECT_STATE_READY,
    CONNECT_STATE_CLOSE,
    CONNECT_CODE_END   = 0x02FF,
};

enum {
    EVENT_CONSUMED = 0, /* event broadcast stop here */
    EVENT_IGNORE /* event will broadcast to next listener */
};

typedef int (*connectivity_cb)(int, void*, int, void*, int*);

typedef struct connectivity_rsp {
    int result;
    int len;
    uint8_t data[0];
} connectivity_rsp_t;

typedef struct connectivity {
    dlist_t list_head;
    char name[NAME_LENGTH];
    int state;
    int (*_init)(void);
    void (*init_buff)(uint8_t**, uint8_t**);
    int (*connect)(void);
    int (*disconnect)(void);
    connectivity_rsp_t* (*send)(void *, int);
    int (*send_async)(void*, int,void *(*p)(connectivity_rsp_t* rsp,void *),void *);
    int (*add_listener)(connectivity_cb);
    int (*del_listener)(connectivity_cb);
} connectivity_t;

typedef struct connectivity_listener {
    dlist_t list_head;
    char name[NAME_LENGTH];
    connectivity_cb listen;
} connectivity_listener_t;

#define CONNECTIVITY_DELCARE(conn) \
    extern connectivity_t conn; \
    extern int _##conn##_init();

#define CONNECTIVITY_DEFINE(conn) \
    connectivity_t conn; \
    int _##conn##_init() { \
        sprintf(conn.name, #conn); \
        conn.state = CONNECT_STATE_CLOSE; \
        conn.init_buff = &conn##_init_buff; \
        conn.connect = &conn##_connect; \
        conn.disconnect = &conn##_disconnect; \
        conn.send = &conn##_send; \
        conn.send_async = &conn##_send_async; \
        conn.add_listener = &conn##_add_listener; \
        conn.del_listener = &conn##_del_listener; \
        dlist_init(&g_##conn##_listener_list); \
        LOGI("connectivity",#conn " initial"); \
        return CONNECT_RESULT_OK; \
    }

#endif
