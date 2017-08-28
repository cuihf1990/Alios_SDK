/**
 * @file yos/cloud.h
 * @brief cloud API
 * @version since 1.0.0
 */

#ifndef __YOS_CLOUD_H__
#define __YOS_CLOUD_H__

enum {
    CLOUD_CONNECTED,
    CLOUD_DISCONNECTED,
    GET_DEVICE_STATUS,
    SET_DEVICE_STATUS,
    GET_DEVICE_RAWDATA,
    SET_DEVICE_RAWDATA,
    UPGRADE_DEVICE,
    CANCEL_UPGRADE_DEVICE,
    GET_SUB_DEVICE_STATUS,
    SET_SUB_DEVICE_STATUS,
    MAX_EVENT_TYPE,
};

typedef void (*yos_cloud_cb_t)(int event, const char *json_buffer);

/**
 * @brief Register cloud event callback
 * @param cb_type event type interested
 * @param cb cloud event callback
 * @retval 0 success
 * @retval <0 failure
 */
int yos_cloud_register_callback(int cb_type, yos_cloud_cb_t cb);

/**
 * @brief Report event to cloud
 * @param method remote method name
 * @param json_buffer method's payload
 * @param done_cb report done callback
 * @param arg private data passed to done_cb
 * @retval 0 success
 * @retval <0 failure
 */
int yos_cloud_report(const char *method,
                     const char *json_buffer,
                     void (*done_cb)(void *),
                     void *arg);

/**
 * @brief trigger specific event, used by Cloud Backend
 * @param cb_type event type
 * @param json_buffer payload
 * @return None
 */
void yos_cloud_trigger(int cb_type, const char *json_buffer);

/**
 * @brief register Cloud Backend
 * @param report called when user do yos_cloud_report
 * @return None
 */
void yos_cloud_register_backend(int (*report)(const char *method,
                                              const char *json_buffer));
#endif
