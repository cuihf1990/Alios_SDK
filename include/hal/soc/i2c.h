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

#ifndef YOS_I2C_H
#define YOS_I2C_H

/**
 * I2C address width
 */
typedef enum {
    I2C_ADDRESS_WIDTH_7BIT,
    I2C_ADDRESS_WIDTH_10BIT,
    I2C_ADDRESS_WIDTH_16BIT,
} hal_i2c_bus_address_width_t;


/**
 * I2C speed mode
 */
typedef enum {
    I2C_LOW_SPEED_MODE,         /* 10Khz devices */
    I2C_STANDARD_SPEED_MODE,    /* 100Khz devices */
    I2C_HIGH_SPEED_MODE         /* 400Khz devices */
} hal_i2c_speed_mode_t;


/**
 * I2C message
 */
typedef struct {
    const void *tx_buffer;
    void       *rx_buffer;
    uint16_t    tx_length;
    uint16_t    rx_length;
    uint16_t    retries;    /* Number of times to retry the message */
    int8_t      combined;   /**< If set, this message is used for both tx and rx. */
    uint8_t
    flags;      /* MESSAGE_DISABLE_DMA : if set, this flag disables use of DMA for the message */
} hal_i2c_msg_t;




typedef struct {
    uint8_t
    port;           /**< Platform I2C port that is connected to the target I2C device, - e.g. MICO_I2C_1 */
    uint16_t
    address;        /**< The address of the device on the I2C bus */
    hal_i2c_bus_address_width_t address_width;  /**< I2C device's address length */
    hal_i2c_speed_mode_t
    speed_mode;     /**< Speed mode the device operates in */
} hal_i2c_device_t;


/******************************************************
 *                 Type Definitions
 ******************************************************/



/******************************************************
 *                 Function Declarations
 ******************************************************/



/**@brief Initialises an I2C interface
 *
 * @note Prepares an I2C hardware interface for communication as a master
 *
 * @param  device : the device for which the i2c port should be initialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred during initialisation
 */
int32_t hal_i2c_init(hal_i2c_device_t *device);


/**@brief Checks whether the device is available on a bus or not
 *
 * @param  device : the i2c device to be probed
 * @param  retries    : the number of times to attempt to probe the device
 *
 * @return    true : device is found.
 * @return    false: device is not found
 */
int8_t hal_i2c_probe_device(hal_i2c_device_t *device, int32_t retries);


/**@brief Initialize the mico_i2c_message_t structure for i2c tx transaction
 *
 * @param message : pointer to a message structure, this should be a valid pointer
 * @param tx_buffer : pointer to a tx buffer that is already allocated
 * @param tx_buffer_length : number of bytes to transmit
 * @param retries    : the number of times to attempt send a message in case it can't not be sent
 *
 * @return    kNoErr    : message structure was initialised properly.
 * @return    kParamErr : one of the arguments is given incorrectly
 */
int32_t hal_i2c_build_tx_msg(hal_i2c_msg_t *msg, const void *tx_buf,
                             uint16_t tx_buf_len, uint16_t retries);

/**@brief Initialize the mico_i2c_message_t structure for i2c rx transaction
 *
 * @param message : pointer to a message structure, this should be a valid pointer
 * @param rx_buffer : pointer to an rx buffer that is already allocated
 * @param rx_buffer_length : number of bytes to receive
 * @param retries    : the number of times to attempt receive a message in case device doesnt respond
 *
 * @return    kNoErr    : message structure was initialised properly.
 * @return    kParamErr : one of the arguments is given incorrectly
 */
int32_t hal_i2c_build_rx_msg(hal_i2c_msg_t *msg, void *rx_buf,
                             uint16_t rx_buf_len, uint16_t retries);


/**@brief Initialize the mico_i2c_message_t structure for i2c combined transaction
 *
 * @param  message : pointer to a message structure, this should be a valid pointer
 * @param tx_buffer: pointer to a tx buffer that is already allocated
 * @param rx_buffer: pointer to an rx buffer that is already allocated
 * @param tx_buffer_length: number of bytes to transmit
 * @param rx_buffer_length: number of bytes to receive
 * @param  retries    : the number of times to attempt receive a message in case device doesnt respond
 *
 * @return    kNoErr    : message structure was initialised properly.
 * @return    kParamErr : one of the arguments is given incorrectly
 */
int32_t hal_i2c_build_combined_msg(hal_i2c_msg_t *msg, const void *tx_buf,
                                   void *rx_buf, uint16_t tx_buf_len, uint16_t rx_buf_len, uint16_t retries);


/**@brief Transmits and/or receives data over an I2C interface
 *
 * @param  device             : the i2c device to communicate with
 * @param  message            : a pointer to a message (or an array of messages) to be transmitted/received
 * @param  number_of_messages : the number of messages to transfer. [1 .. N] messages
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred during message transfer
 */
int32_t hal_i2c_transfer(hal_i2c_device_t *device, hal_i2c_msg_t *msg,
                         uint16_t num);


/**@brief Deinitialises an I2C device
 *
 * @param  device : the device for which the i2c port should be deinitialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred during deinitialisation
 */
int32_t hal_i2c_finalize(hal_i2c_device_t *device);


/** @} */
/** @} */

#endif


