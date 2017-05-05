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

#ifndef YLOOP_H
#define YLOOP_H

/* set loop's event fd */
void yos_loop_set_eventfd(int fd);

/* get loop's event fd */
int yos_loop_get_eventfd(void *loop);

/* init per-loop event service */
int yos_event_service_init(void);

/* deinit per-loop event service */
int yos_event_service_deinit(int fd);

#endif /* YLOOP_H */

