/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_YLOOP_H
#define AOS_YLOOP_H

#include <aos/types.h>

#ifdef __cplusplus
extern "C" {
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
 * Register system event filter callback
 *
 * @param[in]  type  event type interested
 * @param[in]  cb    system event callback
 * @param[in]  priv  data to be bypassed to cb
 *
 * @return  the operation status, 0 is OK, others is error
 */
int aos_register_event_filter(uint16_t type, aos_event_cb cb, void *priv);

/**
 * Unregister native event callback
 *
 * @param[in]  type  event type interested
 * @param[in]  cb    system event callback
 * @param[in]  priv  data to be bypassed to cb
 *
 * @return  the operation status, 0 is OK, others is error
 */
int aos_unregister_event_filter(uint16_t type, aos_event_cb cb, void *priv);

/**
 * Post local event
 *
 * @param[in]  type   event type
 * @param[in]  code   event code
 * @param[in]  value  event value
 *
 * @return  the operation status, 0 is OK,others is error
 */
int aos_post_event(uint16_t type, uint16_t code, unsigned long  value);

/**
 * Register a poll event in main loop
 *
 * @param[in]  fd      poll fd
 * @param[in]  action  action to be executed
 * @param[in]  param   private data past to action
 *
 * @return  the operation status, 0 is OK,others is error
 */
int aos_poll_read_fd(int fd, aos_poll_call_t action, void *param);

/**
 * Cancel a poll event to be executed in main loop
 *
 * @param[in]  fd      poll fd
 * @param[in]  action  action to be executed
 * @param[in]  param   private data past to action
 */
void aos_cancel_poll_read_fd(int fd, aos_poll_call_t action, void *param);

/**
 * Post a delayed action to be executed in main loop
 *
 * @param[in]  ms      milliseconds to wait
 * @param[in]  action  action to be executed
 * @param[in]  arg     private data past to action
 *
 * @return  the operation status, 0 is OK,others is error
 */
int aos_post_delayed_action(int ms, aos_call_t action, void *arg);

/**
 * Cancel a delayed action to be executed in main loop
 *
 * @param[in]  ms      milliseconds to wait, -1 means don't care
 * @param[in]  action  action to be executed
 * @param[in]  arg     private data past to action
 */
void aos_cancel_delayed_action(int ms, aos_call_t action, void *arg);

/**
 * @brief Schedule a callback in next event loop
 * @param action action to be executed
 * @param arg private data past to action
 *
 * @return  >=0 success, <0  failure
 *
 * @note Unlike aos_post_delayed_action,
 *       this function can be called from non-aos-main-loop context.
 */
/**
 * Schedule a callback in next event loop
 *
 * @param[in]  action  action to be executed
 * @param[in]  arg     private data past to action
 *
 * @return  the operation status, <0 is error,others is OK
 */
int aos_schedule_call(aos_call_t action, void *arg);

typedef void *aos_loop_t;

/**
 * Init a per-task event loop
 *
 * @return the handler of aos_loop_t,NULL failure,others success
 */
aos_loop_t aos_loop_init(void);

/**
 * Get current event loop
 *
 * @return  default event loop
 */
aos_loop_t aos_current_loop(void);

/**
 * Start event loop
 */
void aos_loop_run(void);

/**
 * Exit event loop, aos_loop_run() will return
 */
void aos_loop_exit(void);

/**
 * Free event loop resources
 */
void aos_loop_destroy(void);

/**
 * Schedule a callback specified event loop
 *
 * @param[in]  loop    event loop to be scheduled, NULL for default main loop
 * @param[in]  action  action to be executed
 * @param[in]  arg     private data past to action
 *
 * @return  the operation status, <0 is error,others is OK
 */
int aos_loop_schedule_call(aos_loop_t *loop, aos_call_t action, void *arg);

/**
 * Schedule a work to be executed in workqueue
 *
 * @param[in]  ms       milliseconds to delay before execution, 0 means immediately
 * @param[in]  action   action to be executed
 * @param[in]  arg1     private data past to action
 * @param[in]  fini_cb  finish callback to be executed after action is done in current event loop
 * @param[in]  private  data past to fini_cb
 *
 * @return  work handle,NULL failure,others is OK
 */
void *aos_loop_schedule_work(int ms, aos_call_t action, void *arg1,
                             aos_call_t fini_cb, void *arg2);

/**
 * Cancel a work
 *
 * @param[in]  work    work to be cancelled
 * @param[in]  action  action to be executed
 * @param[in]  arg1    private data past to action
 */
void aos_cancel_work(void *work, aos_call_t action, void *arg1);

#ifdef __cplusplus
}
#endif

#endif /* AOS_YLOOP_H */

