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
#ifndef _UART_TRANSPORT_
#define _UART_TRANSPORT_
#include <stdint.h>

typedef void (*uart_recv_cb_t)(uint8_t *pdata, int len);
int uart_trans_init(uart_recv_cb_t cb);
int uart_trans_destroy(void);
int uart_trans_send(uint8_t *pdata, int len);

#endif