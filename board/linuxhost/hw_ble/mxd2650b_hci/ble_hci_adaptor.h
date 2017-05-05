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
#ifndef _BLE_HCI_ADAPTOR_H_
#define _BLE_HCI_ADAPTOR_H_

#include "mxd_type.h"

int hci_setup(void);
void hci_down(void);
void hci_api_acl_send(u8 *buffer,u16 size);
void hci_data_send(u8 *buffer,u16 size);
void hci_api_cmd_send_and_wait_compt(u8 *buffer,u16 size,u16 timeout);

int mxd_fw_init(void);

#endif
