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

/* Magic number for yloop_cancel_timeout() */
#define YOC_LOOP_ALL_DATA (void *) -1

typedef void (*yloop_free_cb)(void *yloop_data, void *private_data);
typedef void (*yloop_sock_cb)(int sock, void *yloop_data, void *private_data);
typedef void (*yloop_timeout_cb)(void *yloop_data, void *private_data);

/* Initialize global event loop data - must be called before any other yloop_*
 * function.
 */
void yloop_init(void);

/* Register handler for read event */
int yloop_register_read_sock(int sock,
        yloop_sock_cb cb, void *yloop_data, void *private_data);

void yloop_unregister_read_sock(int sock);

/* Register timeout */
int yloop_register_timeout(
        unsigned int secs, unsigned int usecs,
        yloop_timeout_cb cb, yloop_free_cb free_cb,
        void *yloop_data, void *private_data);

/* Cancel timeouts matching <handler,yloop_data,private_data>.
 * YLOOP_ALL_CTX can be used as a wildcard for cancelling all timeouts
 * regardless of yloop_data/private_data. */
int yloop_cancel_timeout(
        yloop_timeout_cb cb, void *yloop_data, void *private_data);

/* Start event loop and continue running as long as there are any registered
 * event handlers. */
void yloop_run(void);

/* Terminate event loop even if there are registered events. */
void yloop_terminate(void);

/* Free any reserved resources. After calling yloop_destoy(), other yloop_*
 * functions must not be called before re-running yloop_init(). */
void yloop_destroy(void);

/* Check whether event loop has been terminated. */
int yloop_terminated(void);

void yloop_set_eventfd(int fd);
int yloop_get_eventfd(void);

#endif /* YLOOP_H */

