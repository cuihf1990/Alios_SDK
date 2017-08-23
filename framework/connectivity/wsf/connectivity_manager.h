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

#ifndef CONNECTIVITY_MANAGER_H
#define CONNECTIVITY_MANAGER_H

#include "connectivity.h"

int cm_init();
int cm_add_conn(connectivity_t *conn);
int cm_del_conn(connectivity_t *conn);
int cm_get_conn_state(connectivity_t *conn);
int cm_query_conn();
connectivity_t *cm_get_conn(char *conn_name);
int cm_bind_conn(char *conn_name, connectivity_cb listen);
int cm_release_conn(char *conn_name, connectivity_cb listen);
char *cm_code2string(int state);

#endif
