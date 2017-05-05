/****************************************************************************
 * Copyright (C) 2015 The YunOS Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ****************************************************************************/
#ifndef _TRAN_CHANNEL_H_
#define _TRAN_CHANNEL_H_
#include "mxd_type.h"

#define BT_COMMAND_CHANNEL 1
#define BT_EVENT_CHANNEL   2
#define BT_ACL_IN_CHANNEL  3
#define BT_ACL_OUT_CHANNEL 4

typedef void (*bt_transport_listener_t)(u8 channel, u8* buffer, u16 size);

typedef struct _bt_transport_t {
    char* name;
    int (*open)(bt_transport_listener_t listener);
    int (*close)();
    void (*request_to_write)(u8 channel);
    void (*write)(u8 channel,u16 size);
    void (*data_send)(u8 channel,u8* buffer,u16 size);
    int (*poll)();
} bt_transport_t;

const bt_transport_t * get_ble_trans_adaptor(void);

#endif