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

/**
 * @file yoc/framework.h
 * @brief YoC Framework APIs
 * @version since 5.5.0
 */

#ifndef YOS_FRAMEWORK_API_H
#define YOS_FRAMEWORK_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <yos/internal/event_type_code.h>

/** @defgroup yos_framework Framework API
 *  @{
 */

#ifndef YOS_DOXYGEN_MODE

/** system event */
#define EV_SYS                     0x0001
#define CODE_SYS_ON_STARTING         1
#define CODE_SYS_ON_START_COMPLETED  2
#define CODE_SYS_ON_START_FAILED     4
#define CODE_SYS_ON_IDLE             3

/** WiFi event */
#define  EV_WIFI                0x0002
#define  CODE_WIFI_CMD_RECONNECT  1
#define  CODE_WIFI_ON_CONNECTED   2
#define  CODE_WIFI_ON_DISCONNECT  3
#define  CODE_WIFI_ON_PRE_GOT_IP  4
#define  CODE_WIFI_ON_GOT_IP      5

/** Mesh event */
#define  EV_MESH                0x0003
#define  CODE_MESH_STARTED        1
#define  CODE_MESH_CONNECTED      2
#define  CODE_MESH_ATTACHED       3
#define  CODE_MESH_DETACHED       4

/** user app start */
#define EV_USER     0x1000

#endif

/**
 * @struct input_event_t
 * @brief yoc event structure
 */
typedef struct {
    /** The time event is generated, auto filled by yoc event system */
    uint32_t time;
    /** Event type, value < 0x1000 are used by yoc system */
    uint16_t type;
    /** Defined according to type */
    uint16_t code;
    /** Defined according to type/code */
    unsigned long value;
    /** Defined according to type/code */
    unsigned long extra;
} input_event_t;

/** Free callback  */
typedef void (*yos_free_cb)(void *private_data);
/** Event callback */
typedef void (*yos_event_cb)(input_event_t *event, void *private_data);
/** Delayed execution callback */
typedef void (*yos_call_t)(void *arg);

/**
 * @brief Register system event callback
 * @param yos_event_cb system event callback
 * @param yos_free_cb free data callback
 * @param private_data data to be bypassed to cb
 * @return None
 */
void yos_local_event_listener_register(yos_event_cb cb,
                                       yos_free_cb free_cb,
                                       void *private_data);

/**
 * @brief Unregister native event callback
 * @param yos_event_cb system event callback
 */
void yos_local_event_listener_unregister(yos_event_cb cb);

/**
 * @brief Post local event.
 * @param type event type
 * @param code event code
 * @param value event value
 * @retval >0 success
 * @retval <=0 failure
 * @see input_event_t
 */
int yos_local_event_post(uint16_t type, uint16_t code, unsigned long  value);

/**
 * @brief Register a poll event in main loop
 * @param fd poll fd
 * @param action action to be executed
 * @param param private data past to action
 * @returns None
 */
void yos_poll_read_fd(int fd, yos_call_t action, void *param);

/**
 * @brief Cancel a poll event to be executed in main loop
 * @param fd poll fd
 * @param action action to be executed
 * @param param private data past to action
 * @returns None
 * @note all the parameters must be the same as yos_poll_read_fd
 */
void yos_cancel_poll_read_fd(int fd, yos_call_t action, void *param);

/**
 * @brief Post a delayed action to be executed in main loop
 * @param ms milliseconds to wait
 * @param action action to be executed
 * @param arg private data past to action
 * @return none
 * @note This function must be called under main loop context.
 *       after 'action' is fired, resource will be automatically released.
 */
void yos_post_delayed_action(int ms, yos_call_t action, void *arg);

/**
 * @brief Cancel a delayed action to be executed in main loop
 * @param action action to be executed
 * @param arg private data past to action
 * @returns None
 * @note all the parameters must be the same as yos_post_delayed_action
 */
void yos_cancel_delayed_action(yos_call_t action, void *arg);

/**
 * @brief Schedule a callback in next event loop
 * @param action action to be executed
 * @param arg private data past to action
 * @retval >=0 success
 * @retval <0  failure
 * @note Unlike yos_post_delayed_action,
 *       this function can be called from non-yoc-main-loop context.
 */
int yos_schedule_call(yos_call_t action, void *arg);

/** @} */ //end of Framework API

#ifdef __cplusplus
}
#endif

#endif /* YOS_FRAMEWORK_API_H */

