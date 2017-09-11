/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_YLOOP_H
#define AOS_YLOOP_H

#include <aos/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include <aos/internal/event_type_code.h>

#ifndef AOS_DOXYGEN_MODE

/** special event filter */
#define EV_ALL                       0
#define EV_FLAG_URGENT               0x8000

/** system event */
#define EV_SYS                       0x0001
#define CODE_SYS_ON_STARTING         1
#define CODE_SYS_ON_START_COMPLETED  2
#define CODE_SYS_ON_START_FAILED     4
#define CODE_SYS_ON_IDLE             3
#define CODE_SYS_ON_START_FOTA       5
#define CODE_SYS_ON_ALINK_ONLINE     6
#define CODE_SYS_ON_ALINK_OFFLINE    7
#define CODE_SYS_ON_MQTT_READ        8

/** WiFi event */
#define  EV_WIFI                  0x0002
#define  CODE_WIFI_CMD_RECONNECT  1
#define  CODE_WIFI_ON_CONNECTED   2
#define  CODE_WIFI_ON_DISCONNECT  3
#define  CODE_WIFI_ON_PRE_GOT_IP  4
#define  CODE_WIFI_ON_GOT_IP      5

/** Mesh event */
#define  EV_MESH                  0x0003
#define  CODE_MESH_STARTED        1
#define  CODE_MESH_ATTACHED       2
#define  CODE_MESH_DETACHED       3
#define  CODE_MESH_CONNECTED      4
#define  CODE_MESH_DISCONNECTED   5

/** user app start 0x1000 - 0x7fff */
#define EV_USER     0x1000

#endif

/**
 * @struct input_event_t
 * @brief aos event structure
 */
typedef struct {
    /** The time event is generated, auto filled by aos event system */
    uint32_t time;
    /** Event type, value < 0x1000 are used by aos system */
    uint16_t type;
    /** Defined according to type */
    uint16_t code;
    /** Defined according to type/code */
    unsigned long value;
    /** Defined according to type/code */
    unsigned long extra;
} input_event_t;

/** Event callback */
typedef void (*aos_event_cb)(input_event_t *event, void *private_data);
/** Delayed execution callback */
typedef void (*aos_call_t)(void *arg);
/** Delayed execution callback */
typedef void (*aos_poll_call_t)(int fd, void *arg);

/**
 * @brief Register system event filter callback
 * @param type event type interested
 * @param aos_event_cb system event callback
 * @param private_data data to be bypassed to cb
 * @return None
 */
int aos_register_event_filter(uint16_t type, aos_event_cb cb, void *priv);

/**
 * @brief Unregister native event callback
 * @param aos_event_cb system event callback
 */
int aos_unregister_event_filter(uint16_t type, aos_event_cb cb, void *priv);

/**
 * @brief Post local event.
 * @param type event type
 * @param code event code
 * @param value event value
 * @retval >0 success
 * @retval <=0 failure
 * @see input_event_t
 */
int aos_post_event(uint16_t type, uint16_t code, unsigned long  value);

/**
 * @brief Register a poll event in main loop
 * @param fd poll fd
 * @param action action to be executed
 * @param param private data past to action
 * @return ==0 succeed
 * @return !=0 failed
 */
int aos_poll_read_fd(int fd, aos_poll_call_t action, void *param);

/**
 * @brief Cancel a poll event to be executed in main loop
 * @param fd poll fd
 * @param action action to be executed
 * @param param private data past to action
 * @returns None
 * @note all the parameters must be the same as aos_poll_read_fd
 */
void aos_cancel_poll_read_fd(int fd, aos_poll_call_t action, void *param);

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
int aos_post_delayed_action(int ms, aos_call_t action, void *arg);

/**
 * @brief Cancel a delayed action to be executed in main loop
 * @param ms milliseconds to wait, -1 means don't care
 * @param action action to be executed
 * @param arg private data past to action
 * @returns None
 * @note all the parameters must be the same as aos_post_delayed_action
 */
void aos_cancel_delayed_action(int ms, aos_call_t action, void *arg);

/**
 * @brief Schedule a callback in next event loop
 * @param action action to be executed
 * @param arg private data past to action
 * @retval >=0 success
 * @retval <0  failure
 * @note Unlike aos_post_delayed_action,
 *       this function can be called from non-aos-main-loop context.
 */
int aos_schedule_call(aos_call_t action, void *arg);

typedef void *aos_loop_t;

/**
 * @brief init a per-task event loop
 * @param None
 * @retval ==NULL failure
 * @retval !=NULL success
 */
aos_loop_t aos_loop_init(void);

/**
 * @brief get current event loop
 * @param None
 * @retval default event loop
 */
aos_loop_t aos_current_loop(void);

/**
 * @brief start event loop
 * @param None
 * @retval None
 * @note this function won't return until aos_loop_exit called
 */
void aos_loop_run(void);

/**
 * @brief exit event loop, aos_loop_run() will return
 * @param None
 * @retval None
 * @note this function must be called from the task runninng the event loop
 */
void aos_loop_exit(void);

/**
 * @brief free event loop resources
 * @param None
 * @retval None
 * @note this function should be called after aos_loop_run() return
 */
void aos_loop_destroy(void);

/**
 * @brief Schedule a callback specified event loop
 * @param loop event loop to be scheduled, NULL for default main loop
 * @param action action to be executed
 * @param arg private data past to action
 * @retval >=0 success
 * @retval <0  failure
 * @note Unlike aos_post_delayed_action,
 *       this function can be called from non-aos-main-loop context.
 */
int aos_loop_schedule_call(aos_loop_t *loop, aos_call_t action, void *arg);

/**
 * @brief Schedule a work to be executed in workqueue
 * @param ms milliseconds to delay before execution, 0 means immediately
 * @param action action to be executed
 * @param arg1 private data past to action
 * @param fini_cb finish callback to be executed after action is done in current event loop
 * @param arg2 private data past to fini_cb
 * @retval  0 failure
 * @retval !0 work handle
 * @note  this function can be called from non-aos-main-loop context.
 */
void *aos_loop_schedule_work(int ms, aos_call_t action, void *arg1,
                             aos_call_t fini_cb, void *arg2);

/**
 * @brief Cancel a work
 * @param work work to be cancelled
 * @param action action to be cancelled
 * @param arg1 private data past to action
 * @retval None
 */
void aos_cancel_work(void *work, aos_call_t action, void *arg1);

#ifdef __cplusplus
}
#endif

#endif /* AOS_YLOOP_H */

