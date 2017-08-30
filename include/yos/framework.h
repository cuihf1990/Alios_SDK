/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
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
#define CODE_SYS_ON_START_FOTA       5
#define CODE_SYS_ON_ALINK_ONLINE     6
#define CODE_SYS_ON_ALINK_OFFLINE    7
#define CODE_SYS_ON_CONNECT          8
#define CODE_SYS_ON_DISCONNECT       9

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
#define  CODE_MESH_ATTACHED       2
#define  CODE_MESH_DETACHED       3
#define  CODE_MESH_CONNECTED      4
#define  CODE_MESH_DISCONNECTED   5

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
void *yos_loop_schedule_work(int ms, yos_call_t action, void *arg1,
                             yos_call_t fini_cb, void *arg2);

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

/**
 * @brief open the file or device by its path @path.
 *
 * @param[in] @path , the path of the file or device you want to open.
 * @param[in] @flags,  the mode of open operation
 *
 * @retval  >=0 on success.
 * @retval <0 failure.
 */
int yos_open(const char *path, int flags);

/**
 * @brief close the file or device by its fd @fd.
 *
 * @param[in] @fd , the handle of the file or device.
 *
 * @retval  0 on success.
 * @retval <0 failure.
 */
int yos_close(int fd);

/**
 * @brief read the contents of a file or device into a buffer.
 *
 * @param[in]   @fd , the handle of the file or device.
 * @param[in]   @nbytes, the number of bytes to read.
 * @param[out] @buf, the buffer to read in to.
 *
 * @retval  The number of bytes read, 0 at end of file, negative error on failure
 */
ssize_t yos_read(int fd, void *buf, size_t nbytes);

/**
 * @brief write the contents of a buffer to file or device
 *
 * @param[in]   @fd , the handle of the file or device.
 * @param[in]   @nbytes, the number of bytes to write.
 * @param[in]   @buf, the buffer to write from.
 *
 * @retval    The number of bytes written, negative error on failure.
 */
ssize_t yos_write(int fd, const void *buf, size_t nbytes);

/**
 * @brief This is a wildcard API for sending controller specific commands.
 *
 * @param[in]  @fd, the handle of the file or device
 * @param[in]  @cmd, A controller specific command.
 * @param[in]  @arg , Argument to the command; interpreted according to the command.
 * @retval           any return from the command.
 */
int yos_ioctl(int fd, int cmd, unsigned long arg);

/**
 * @brief A mechanism to multiplex input/output over a set of file handles(file descriptors).
 * For every file handle provided, poll() examines it for any events registered for that particular
 * file handle.
 *
 * @param[in]  @fds      a point to the array of PollFh struct carrying a FileHandle and bitmasks of events
 * @param[in]  @nfhs    number of file handles
 * @param[in]  @timeout timer value to timeout or -1 for loop forever
 *
 * @retval number of file handles selected (for which revents is non-zero). 0 if timed out with nothing selected. -1 for error.
 */
int yos_poll(struct pollfd *fds, int nfds, int timeout);

/**
 * @brief  performs one of the operations described below on the open file descriptor @fd.
 *            The operation is determined by @cmd.
 *
 * @param[in]  @fd       the handle of the file or device
 * @param[in]  @cmd    the operation of the file or device
 * @param[in]  @val      it depends on whether @cmd need params.
 *
 * @retval  0 on success, otherwise -1 will be returned
 */
int yos_fcntl(int fd, int cmd, int val);

/**@brief Transmit data on a UART interface
 *
 * @param  data     : pointer to the start of data
 * @param  size     : number of bytes to transmit
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int32_t yos_uart_send(void *data, uint32_t size, uint32_t timeout);

/** @} */ //end of Framework API

#ifdef __cplusplus
}
#endif

#endif /* YOS_FRAMEWORK_API_H */

