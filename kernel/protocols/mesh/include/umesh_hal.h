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

#ifndef UMESH_HAL_H
#define UMESH_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "umesh_types.h"

struct ur_mesh_hal_module_s;

/**
 * Callback function when ucast frame send done
 *
 * @param[out] context The context
 * @param[in]  frame   The frame that has been sent
 * @param[out] error   The error code for this sent frame
 */
typedef void (*ur_mesh_handle_sent_ucast_t)(void *context,
                                            frame_t *frame, int error);

/**
 * Callback function when bcast frame send done
 *
 * @param[out] context The context
 * @param[in]  frame   The frame that has been sent
 * @param[out] error   The error code for this sent frame
 */
typedef void (*ur_mesh_handle_sent_bcast_t)(void *context,
                                            frame_t *frame, int error);

/**
 * Callback function when frame received
 *
 * @param[out] context The context
 * @param[in]  frame      The received frame
 * @param[out] frame_info The frame info that needs by uppder layer
 * @param[out] error      The error code for this received data frame
 */
typedef void (*ur_mesh_handle_received_frame_t)(void *context, frame_t *frame,
                                                frame_info_t *frame_info,
                                                int error);

/**
 * Callback function when received commands
 *
 * @param[in] buf    The received commands
 * @param[in] length The commands length
 */
typedef void (*ur_mesh_cli_input_t)(const uint8_t *buf, uint16_t length);

typedef struct ur_mesh_hal_module_s {
    hal_module_base_t base;
    media_type_t type;
    ur_mesh_handle_received_frame_t receiver;

    int (*ur_mesh_hal_init)(struct ur_mesh_hal_module_s *module, void *config);
    int (*ur_mesh_hal_enable)(struct ur_mesh_hal_module_s *module);
    int (*ur_mesh_hal_disable)(struct ur_mesh_hal_module_s *module);

    /* send ucast frame */
    int (*ur_mesh_hal_send_ucast_request)(struct ur_mesh_hal_module_s *module,
                                          frame_t *frame, mac_address_t *dest,
                                          ur_mesh_handle_sent_ucast_t sent,
                                          void *context);
    /* send bcast frame */
    int (*ur_mesh_hal_send_bcast_request)(struct ur_mesh_hal_module_s *module,
                                          frame_t *frame,
                                          ur_mesh_handle_sent_bcast_t sent,
                                          void *context);

    /* register call back when received packet */
    int (*ur_mesh_hal_register_receiver)(struct ur_mesh_hal_module_s *module,
                                         ur_mesh_handle_received_frame_t received, void *context);

    /* request low layer to transmit beacons intervally*/
    int (*ur_mesh_hal_start_beacons)(struct ur_mesh_hal_module_s *module,
                                     frame_t *data, mac_address_t *dest,
                                     uint16_t max_interval);
    int (*ur_mesh_hal_stop_beacons)(struct ur_mesh_hal_module_s *module);

    int (*ur_mesh_hal_set_bcast_mtu)(struct ur_mesh_hal_module_s *module,
                                     uint16_t mtu);
    int (*ur_mesh_hal_get_bcast_mtu)(struct ur_mesh_hal_module_s *module);
    int (*ur_mesh_hal_set_ucast_mtu)(struct ur_mesh_hal_module_s *module,
                                     uint16_t mtu);
    int (*ur_mesh_hal_get_ucast_mtu)(struct ur_mesh_hal_module_s *module);

    int (*ur_mesh_hal_set_bcast_channel)(struct ur_mesh_hal_module_s *module,
                                         uint8_t channel);
    int (*ur_mesh_hal_get_bcast_channel)(struct ur_mesh_hal_module_s *module);
    int (*ur_mesh_hal_get_bcast_chnlist)(struct ur_mesh_hal_module_s *module,
                                         const uint8_t **chnlist);
    int (*ur_mesh_hal_set_ucast_channel)(struct ur_mesh_hal_module_s *module,
                                         uint8_t channel);
    int (*ur_mesh_hal_get_ucast_channel)(struct ur_mesh_hal_module_s *module);
    int (*ur_mesh_hal_get_ucast_chnlist)(struct ur_mesh_hal_module_s *module,
                                         const uint8_t **chnlist);

    int (*ur_mesh_hal_set_txpower)(struct ur_mesh_hal_module_s *module,
                                   int8_t txpower);
    int (*ur_mesh_hal_get_txpower)(struct ur_mesh_hal_module_s *module);

    int (*ur_mesh_hal_set_meshnetid)(struct ur_mesh_hal_module_s *module,
                                     const meshnetid_t *meshnetid);
    const meshnetid_t *(*ur_mesh_hal_get_meshnetid)(
        struct ur_mesh_hal_module_s *module);
    int (*ur_mesh_hal_set_mac_address)(struct ur_mesh_hal_module_s *module,
                                       const mac_address_t *addr);
    const mac_address_t *(*ur_mesh_hal_get_mac_address)(
        struct ur_mesh_hal_module_s *module);

    int (*ur_mesh_hal_set_key)(struct ur_mesh_hal_module_s *module,
                               uint8_t index, uint8_t *key, uint8_t length);
    int (*ur_mesh_hal_activate_key)(struct ur_mesh_hal_module_s *module,
                                    uint8_t index);
    int (*ur_mesh_hal_is_sec_enabled)(struct ur_mesh_hal_module_s *module);

    const frame_stats_t *(*ur_mesh_hal_get_stats)(struct ur_mesh_hal_module_s
                                                  *module);
} ur_mesh_hal_module_t;

/**
 * Initialize all registed HAL modules.
 *
 * @return
 *     Initalization result, 0 if success, nonzero if fail
 */
int hal_ur_mesh_init(void);

/**
 * Get the defaut uRadar mesh HAL
 *
 * The system may have more than 1 mesh HAL instances.
 *
 * @return
 *     Instance pointer or NULL
 */
ur_mesh_hal_module_t *hal_ur_mesh_get_default_module(void);

/**
 * Get the next uRadar mesh HAL
 *
 * The system may have more than 1 mesh HAL instances.
 *
 * @return
 *     Instance pointer or NULL
 */
ur_mesh_hal_module_t *hal_ur_mesh_get_next_module(ur_mesh_hal_module_t *cur);

/**
 * Register one or more mesh instances to HAL.
 *
 * @param[in] module The HAL module to be registered
 */
void hal_ur_mesh_register_module(ur_mesh_hal_module_t *module);

/**
 * Enable a uRadar HAL module, which usually powers on its hardware
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     Enable result, 0 if success, -1 if fail
 */
int hal_ur_mesh_enable(ur_mesh_hal_module_t *module);

/**
 * Disable a uRadar HAL module, which usually power off its hardware
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     Disable result, 0 if success, -1 if fail
 */
int hal_ur_mesh_disable(ur_mesh_hal_module_t *module);

/**
 * Send HAL ucast frame request
 *
 * @param[in] module  The HAL module to send data to; if NULL, the default module will be used
 * @param[in] frame   The frame buffer that contains the data
 * @param[in] dest    The destination of this frame.
 * @param[in] sent    The callback function to be called after hardware send data finish
 * @param[in] context The context
 *
 * @return
 *     Send frame request result, 0 if success, -1 if fail
 */
int hal_ur_mesh_send_ucast_request(ur_mesh_hal_module_t *module,
                                   frame_t *frame, mac_address_t *dest,
                                   ur_mesh_handle_sent_ucast_t sent, void *context);

/**
 * Send HAL bcast frame request
 *
 * @param[in] module  The HAL module to send data to; if NULL, the default module will be used
 * @param[in] frame   The frame buffer that contains the data
 * @param[in] sent    The callback function to be called after hardware send data finish
 * @param[in] context The context
 *
 * @return
 *     Send frame request result, 0 if success, -1 if fail
 */
int hal_ur_mesh_send_bcast_request(ur_mesh_hal_module_t *module,
                                   frame_t *frame,
                                   ur_mesh_handle_sent_bcast_t sent, void *context);

/**
 * Register data frame receiver callback function
 *
 * @param[in] module   The HAL module to receive data from; if NULL, the default module will be used
 * @param[in] received The callback function to be called after hardware received data
 * @param[in] context  The context
 *
 * @return
 *     Register receiver result, 0 if success, -1 if fail
 */
int hal_ur_mesh_register_receiver(ur_mesh_hal_module_t *module,
                                  ur_mesh_handle_received_frame_t received, void *context);

/**
 * Request HAL to send beacons
 *
 * @param[in] module       The HAL module to send data to; if NULL, the default module will be used
 * @param[in] frame        The frame buffer that contains the data and config information
 * @param[in] dest         The destination of this frame.
 * @param[in] max_interval The max interval that low layer needs to transmit beacon
 *
 * @return
 *     The result, 0 if success, -1 if fail
 */
int hal_ur_mesh_start_beacons(ur_mesh_hal_module_t *module,
                              frame_t *frame, mac_address_t *dest,
                              uint16_t max_interval);

/**
 * Stop HAL to send beacons
 *
 * @param[in] module The HAL module to send data to; if NULL, the default module will be used
 *
 * @return
 *     The result, 0 if success, -1 if fail
 */
int hal_ur_mesh_stop_beacons(ur_mesh_hal_module_t *module);

/**
 * Set HAL broadcast MTU, MTU is normally decided by HAL.
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] mtu    The MTU to be set
 *
 * @return
 *     Set media configuration result, 0 if success, -1 if fail
 */
int hal_ur_mesh_set_bcast_mtu(ur_mesh_hal_module_t *module, uint16_t mtu);

/**
 * Get HAL broadcast MTU
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     The MTU, -1 if fail
 */
int hal_ur_mesh_get_bcast_mtu(ur_mesh_hal_module_t *module);

/**
 * Set HAL unicast MTU, MTU is normally decided by HAL.
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] mtu    The MTU to be set
 *
 * @return
 *     Set media configuration result, 0 if success, -1 if fail
 */
int hal_ur_mesh_set_ucast_mtu(ur_mesh_hal_module_t *module, uint16_t mtu);

/**
 * Get HAL unicast MTU
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     The MTU, -1 if fail
 */
int hal_ur_mesh_get_ucast_mtu(ur_mesh_hal_module_t *module);

/**
 * Set broadcast channel
 *
 * @param[in] module  The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] channel The channel to be set
 *
 * @return
 *     Set media configuration result, 0 if success, -1 if fail
 */
int hal_ur_mesh_set_bcast_channel(ur_mesh_hal_module_t *module,
                                  uint8_t channel);

/**
 * Get broadcast channel
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     The channel, -1 if fail
 */
int hal_ur_mesh_get_bcast_channel(ur_mesh_hal_module_t *module);

/**
 * Get broadcast channel list
 *
 * @param[in] module  The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] chnlist Pointer to store the broadcast channel list
 *
 * @return
 *     The number of broadcast channels, -1 if fail
 */
int hal_ur_mesh_get_bcast_chnlist(ur_mesh_hal_module_t *module,
                                  const uint8_t **chnlist);
/**
 * Set unicast channel
 *
 * @param[in] module  The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] channel The channel to be set
 *
 * @return
 *     Set media configuration result, 0 if success, -1 if fail
 */
int hal_ur_mesh_set_ucast_channel(ur_mesh_hal_module_t *module,
                                  uint8_t channel);

/**
 * Get unicast channel
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     The channel, -1 if fail
 */
int hal_ur_mesh_get_ucast_channel(ur_mesh_hal_module_t *module);

/**
 * Get unicast channel list
 *
 * @param[in] module  The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] chnlist Pointer to store the unicast channel list
 *
 * @return
 *     The number of unicast channels, -1 if fail
 */
int hal_ur_mesh_get_ucast_chnlist(ur_mesh_hal_module_t *module,
                                  const uint8_t **chnlist);

/**
 * Set transmit power
 *
 * @param[in] module  The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] txpower The transmit power to be set
 *
 * @return
 *     Set media configuration result, 0 if success, -1 if fail
 */
int hal_ur_mesh_set_txpower(ur_mesh_hal_module_t *module, int8_t txpower);

/**
 * Get transmit power
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     The transmit power, -1 if fail
 */
int hal_ur_mesh_get_txpower(ur_mesh_hal_module_t *module);

/**
 * Set meshnetid
 *
 * @param[in] module    The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] meshnetid The meshnetid to be set
 *
 * @return
 *     Set media configuration result, 0 if success, -1 if fail
 */
int hal_ur_mesh_set_meshnetid(ur_mesh_hal_module_t *module,
                              const meshnetid_t *meshnetid);

/**
 * Get meshnetid
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     The meshnetid, NULL if fail
 */
const meshnetid_t *hal_ur_mesh_get_meshnetid(ur_mesh_hal_module_t *module);

/**
 * Set HAL mac address.
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] addr   The mac address to be set
 *
 * @return
 *     Set media configuration result, 0 if success, -1 if fail
 */
int hal_ur_mesh_set_mac_address(ur_mesh_hal_module_t *module,
                                const mac_address_t *addr);

/**
 * Get HAL mac address.
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     The mac address, NULL if fail
 */
const mac_address_t *hal_ur_mesh_get_mac_address(ur_mesh_hal_module_t *module);

/**
 * Set HAL encryption key.
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] index  The key index
 * @param[in] key    The pointer to the encryption key
 * @param[in] length The key length, default value is 16
 *
 * @return
 *     Set result, 0 if success, -1 if fail
 */
int hal_ur_mesh_set_key(struct ur_mesh_hal_module_s *module,
                        uint8_t index, uint8_t *key, uint8_t length);

/**
 * Activate the specified encryption key of HAL.
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 * @param[in] index  The key index
 *
 * @return
 *     Activate result, 0 if success, -1 if fail
 */
int hal_ur_mesh_activate_key(struct ur_mesh_hal_module_s *module,
                             uint8_t index);

/**
 * Check whether security is enabled or not by the specified HAL.
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     Security state, 0 if disabled, 1 if enabled
 */
int hal_ur_mesh_is_sec_enabled(struct ur_mesh_hal_module_s *module);

/**
 * Read uRadar mesh HAL frame stats.
 *
 * @param[in] module The HAL module to be operated; if NULL, the default module will be operated
 *
 * @return
 *     The HAL frame stats, NULL if fail
 */
const frame_stats_t *hal_ur_mesh_get_stats(ur_mesh_hal_module_t *module);

#ifdef __cplusplus
}
#endif

#endif /* UMESH_HAL_H */
