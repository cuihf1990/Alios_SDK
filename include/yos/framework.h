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

#ifndef YOS_FRAMEWORK_API_H
#define YOS_FRAMEWORK_API_H

#include <yos/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <yos/internal/event_type_code.h>

/** @defgroup yos_framework Framework API
 *  @{
 */

#ifndef YOS_DOXYGEN_MODE

/** special event filter */
#define EV_ALL                       0
#define EV_FLAG_URGENT             0x8000

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

/** user app start 0x1000 - 0x7fff */
#define EV_USER     0x1000

#endif

/**
 * @struct input_event_t
 * @brief yos event structure
 */
typedef struct {
    /** The time event is generated, auto filled by yos event system */
    uint32_t time;
    /** Event type, value < 0x1000 are used by yos system */
    uint16_t type;
    /** Defined according to type */
    uint16_t code;
    /** Defined according to type/code */
    unsigned long value;
    /** Defined according to type/code */
    unsigned long extra;
} input_event_t;

/** Event callback */
typedef void (*yos_event_cb)(input_event_t *event, void *private_data);
/** Delayed execution callback */
typedef void (*yos_call_t)(void *arg);
/** Delayed execution callback */
typedef void (*yos_poll_call_t)(int fd, void *arg);

/**
 * @brief Register system event filter callback
 * @param type event type interested
 * @param yos_event_cb system event callback
 * @param private_data data to be bypassed to cb
 * @return None
 */
int yos_register_event_filter(uint16_t type, yos_event_cb cb, void *priv);

/**
 * @brief Unregister native event callback
 * @param yos_event_cb system event callback
 */
int yos_unregister_event_filter(uint16_t type, yos_event_cb cb, void *priv);

/**
 * @brief Post local event.
 * @param type event type
 * @param code event code
 * @param value event value
 * @retval >0 success
 * @retval <=0 failure
 * @see input_event_t
 */
int yos_post_event(uint16_t type, uint16_t code, unsigned long  value);

/**
 * @brief Register a poll event in main loop
 * @param fd poll fd
 * @param action action to be executed
 * @param param private data past to action
 * @return ==0 succeed
 * @return !=0 failed
 */
int yos_poll_read_fd(int fd, yos_poll_call_t action, void *param);

/**
 * @brief Cancel a poll event to be executed in main loop
 * @param fd poll fd
 * @param action action to be executed
 * @param param private data past to action
 * @returns None
 * @note all the parameters must be the same as yos_poll_read_fd
 */
void yos_cancel_poll_read_fd(int fd, yos_poll_call_t action, void *param);

/**
 * @brief Post a delayed action to be executed in main loop
 * @param ms milliseconds to wait
 * @param action action to be executed
 * @param arg private data past to action
 * @return ==0 succeed
 * @return !=0 failed
 * @note This function must be called under main loop context.
 *       after 'action' is fired, resource will be automatically released.
 */
int yos_post_delayed_action(int ms, yos_call_t action, void *arg);

/**
 * @brief Cancel a delayed action to be executed in main loop
 * @param ms milliseconds to wait, -1 means don't care
 * @param action action to be executed
 * @param arg private data past to action
 * @returns None
 * @note all the parameters must be the same as yos_post_delayed_action
 */
void yos_cancel_delayed_action(int ms, yos_call_t action, void *arg);

/**
 * @brief Schedule a callback in next event loop
 * @param action action to be executed
 * @param arg private data past to action
 * @retval >=0 success
 * @retval <0  failure
 * @note Unlike yos_post_delayed_action,
 *       this function can be called from non-yos-main-loop context.
 */
int yos_schedule_call(yos_call_t action, void *arg);

typedef void *yos_loop_t;

/**
 * @brief init a per-task event loop
 * @param None
 * @retval ==NULL failure
 * @retval !=NULL success
 */
yos_loop_t yos_loop_init(void);

/**
 * @brief get current event loop
 * @param None
 * @retval default event loop
 */
yos_loop_t yos_current_loop(void);

/**
 * @brief start event loop
 * @param None
 * @retval None
 * @note this function won't return until yos_loop_exit called
 */
void yos_loop_run(void);

/**
 * @brief exit event loop, yos_loop_run() will return
 * @param None
 * @retval None
 * @note this function must be called from the task runninng the event loop
 */
void yos_loop_exit(void);

/**
 * @brief free event loop resources
 * @param None
 * @retval None
 * @note this function should be called after yos_loop_run() return
 */
void yos_loop_destroy(void);

/**
 * @brief Schedule a callback specified event loop
 * @param loop event loop to be scheduled, NULL for default main loop
 * @param action action to be executed
 * @param arg private data past to action
 * @retval >=0 success
 * @retval <0  failure
 * @note Unlike yos_post_delayed_action,
 *       this function can be called from non-yos-main-loop context.
 */
int yos_loop_schedule_call(yos_loop_t *loop, yos_call_t action, void *arg);

/**
 * @brief Schedule a work to be executed in workqueue 
 * @param ms milliseconds to delay before execution, 0 means immediately
 * @param action action to be executed
 * @param arg1 private data past to action
 * @param fini_cb finish callback to be executed after action is done in current event loop
 * @param arg2 private data past to fini_cb
 * @retval  0 failure
 * @retval !0 work handle
 * @note  this function can be called from non-yos-main-loop context.
 */
void *yos_schedule_work(int ms, yos_call_t action, void *arg1, yos_call_t fini_cb, void *arg2);

/**
 * @brief Cancel a work
 * @param work work to be cancelled
 * @param action action to be cancelled
 * @param arg1 private data past to action
 * @retval None
 */
void yos_cancel_work(void *work, yos_call_t action, void *arg1);

/**
 * @brief add another KV pair .
 *
 * @param[in] @key  the key of the KV pair.
 * @param[in] @value  the value of the KV pair.
 * @param[in] @len  the length of the @value.
 * @param[in] @sync  1: save the KV pair to flash right now,
 *                   0: do not save the pair this time.
 *
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_set(const char *key, const void *value, int len, int sync);

/**
 * @brief get the KV value stored in @buffer by its key @key.
 *
 * @param[in] @key, the key of the KV you want to get.
 * @param[out] @buffer, the memory to store KV value.
 * @param[out] @buff_len, the real lenght of value.
 *
 * @note: the @buffer_len should be large enough to store the value,
 *            otherwise @buffer would be NULL.
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_get(const char *key, void *buffer, int *buffer_len);

/**
 * @brief delete the KV pair by its key @key.
 *
 * @param[in] key , the key of the KV pair you want to delete.
 *
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_kv_del(const char *key);

int yos_open(const char *path, int flags);
int yos_close(int fd);
ssize_t yos_read(int fd, void *buf, size_t nbytes);
ssize_t yos_write(int fd, const void *buf, size_t nbytes);
int yos_ioctl(int fd, int cmd, unsigned long arg);
int yos_poll(struct pollfd *fds, int nfds, int timeout);
int yos_fcntl(int fd, int cmd, int val);

/** @} */ //end of Framework API

#ifdef __cplusplus
}
#endif

#endif /* YOS_FRAMEWORK_API_H */

