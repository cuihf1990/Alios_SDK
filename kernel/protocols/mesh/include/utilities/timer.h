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

#ifndef UR_TIMER_H
#define UR_TIMER_H

#include <stdint.h>

typedef void (* timer_handler_t)(void *args);
typedef void *ur_timer_t;

ur_timer_t ur_start_timer(uint32_t dt, timer_handler_t handler, void *args);
void ur_stop_timer(ur_timer_t *timer, void *args);
uint32_t ur_get_now(void);

#endif  /* UR_TIMER_H */
