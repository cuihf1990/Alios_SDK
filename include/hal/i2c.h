/**
 ******************************************************************************
 * @file    MicoDriverI2C.h
 * @author  William Xu
 * @version V1.0.0
 * @date    16-Sep-2014
 * @brief   This file provides all the headers of I2C operation functions.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#ifndef __MICODRIVERI2C_H__
#define __MICODRIVERI2C_H__

#pragma once
#include "common.h"
#include "platform.h"
#include "platform_peripheral.h"

/** @addtogroup MICO_PLATFORM
* @{
*/

/** @defgroup MICO_I2C MICO I2C Driver
* @brief  Inter-IC bus (I2C) Functions
* @{
*/

/******************************************************
 *                   Macros
 ******************************************************/  

/******************************************************
 *                   Enumerations
 ******************************************************/

// typedef enum
// {
//     I2C_ADDRESS_WIDTH_7BIT,     /**< I2C device has 7bit address */
//     I2C_ADDRESS_WIDTH_10BIT,    /**< I2C device has 10bit address */
//     I2C_ADDRESS_WIDTH_16BIT,    /**< I2C device has 16bit address */
// } hal_i2c_bus_address_width_t;

// typedef enum
// {
//     I2C_LOW_SPEED_MODE,         /**< I2C clock speed for 10Khz devices */
//     I2C_STANDARD_SPEED_MODE,    /**< I2C clock speed for 100Khz devices */
//     I2C_HIGH_SPEED_MODE         /**< I2C clock speed for 400Khz devices */
// } hal_i2c_speed_mode_t;

/******************************************************
 *                    Structures
 ******************************************************/

typedef platform_i2c_bus_address_width_t hal_i2c_bus_address_width_t;

typedef platform_i2c_speed_mode_t        hal_i2c_speed_mode_t;

typedef platform_i2c_msg_t               hal_i2c_msg_t;

typedef struct
{
   hal_i2c_t                   port;           /**< Platform I2C port that is connected to the target I2C device, - e.g. MICO_I2C_1 */
   uint16_t                    address;        /**< The address of the device on the I2C bus */
   hal_i2c_bus_address_width_t address_width;  /**< I2C device's address length */
   hal_i2c_speed_mode_t        speed_mode;     /**< Speed mode the device operates in */
} hal_i2c_device_t;

// typedef struct
// {
//     const void*  tx_buffer;  /**< A pointer to the data to be transmitted. If NULL, the message is an RX message when 'combined' is FALSE */
//     void*        rx_buffer;  /**< A pointer to the data to be transmitted. If NULL, the message is an TX message when 'combined' is FALSE */
//     uint16_t     tx_length;  /**< Number of bytes to transmit */
//     uint16_t     rx_length;  /**< Number of bytes to receive */
//     uint16_t     retries;    /**< Number of times to retry the message */
//     bool combined;           /**< If set, this message is used for both tx and rx. */
// } mico_i2c_message_t;

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
hal_stat_t hal_i2c_init(hal_i2c_device_t *device);


/**@brief Checks whether the device is available on a bus or not
 *
 * @param  device : the i2c device to be probed
 * @param  retries    : the number of times to attempt to probe the device
 *
 * @return    true : device is found.
 * @return    false: device is not found
 */
bool hal_i2c_probe_device(hal_i2c_device_t *device, int retries);


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
hal_stat_t hal_i2c_build_tx_msg(hal_i2c_msg_t *msg, const void *tx_buf, uint16_t tx_buf_len, uint16_t retries);

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
hal_stat_t hal_i2c_build_rx_msg(hal_i2c_msg_t *msg, void* rx_buf, uint16_t rx_buf_len, uint16_t retries);


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
hal_stat_t hal_i2c_build_combined_msg(hal_i2c_msg_t *msg, const void *tx_buf, void *rx_buf, uint16_t tx_buf_len, uint16_t rx_buf_len, uint16_t retries);


/**@brief Transmits and/or receives data over an I2C interface
 *
 * @param  device             : the i2c device to communicate with
 * @param  message            : a pointer to a message (or an array of messages) to be transmitted/received
 * @param  number_of_messages : the number of messages to transfer. [1 .. N] messages
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred during message transfer
 */
hal_stat_t hal_i2c_transfer(hal_i2c_device_t *device, hal_i2c_msg_t *msg, uint16_t num);


/**@brief Deinitialises an I2C device
 *
 * @param  device : the device for which the i2c port should be deinitialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred during deinitialisation
 */
hal_stat_t hal_i2c_finalize(hal_i2c_device_t *device);


/** @} */
/** @} */

#endif


