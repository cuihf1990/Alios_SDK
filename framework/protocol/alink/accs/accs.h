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

#ifndef ACCS_H
#define ACCS_H

#include "service.h"

void start_accs_work(int delay);
void stop_accs_work();
int accs_prepare();
int accs_start();
int accs_stop();
int accs_put(void *, int);
int accs_put_async(void *, int, void *(*cb)(void *), void *arg);
int accs_get(void *, int, void *, int);
int accs_add_listener(service_cb);
int accs_del_listener(service_cb);
int cloud_is_connected(void);
#endif
