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

#ifndef YOS_UART_H
#define YOS_UART_H

#pragma once
#include "common.h"
#include "RingBufferUtils.h"
#include "board_platform.h"
#include "platform_peripheral.h"

/** @addtogroup HAL_PLATFORM
* @{
*/

/** @defgroup hal_UART MICO UART Driver
* @brief  Universal Asynchronous Receiver Transmitter (UART) Functions
* @{
*/

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
 typedef platform_uart_config_t                  hal_uart_config_t;

/******************************************************
 *                 Function Declarations
 ******************************************************/

typedef enum
{
    HAL_UART_0,
    HAL_UART_1,
    HAL_UART_2,
    HAL_UART_3,
    HAL_UART_4,
    HAL_UART_MAX,
} hal_uart_t;



/**@brief Initialises a UART interface
 *
 * @note Prepares an UART hardware interface for communications
 *
 * @param  uart     : the interface which should be initialised
 * @param  config   : UART configuration structure
 * @param  optional_rx_buffer : Pointer to an optional RX ring buffer
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_uart_init(hal_uart_t uart, const hal_uart_config_t *config, ring_buf_t *optional_rx_buf);


/**@brief Initialises a STDIO UART interface, internal use only
 *
 * @note Prepares an UART hardware interface for stdio communications
 *
 * @param  config   : UART configuration structure
 * @param  optional_rx_buffer : Pointer to an optional RX ring buffer
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_stdio_uart_init(const hal_uart_config_t *config, ring_buf_t *optional_rx_buf);


/**@brief Deinitialises a UART interface
 *
 * @param  uart : the interface which should be deinitialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_uart_finalize(hal_uart_t uart);


/**@brief Transmit data on a UART interface
 *
 * @param  uart     : the UART interface
 * @param  data     : pointer to the start of data
 * @param  size     : number of bytes to transmit
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_uart_send(hal_uart_t uart, const void *data, uint32_t size);


/**@brief Receive data on a UART interface
 *
 * @param  uart     : the UART interface
 * @param  data     : pointer to the buffer which will store incoming data
 * @param  size     : number of bytes to receive
 * @param  timeout  : timeout in milisecond
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_uart_recv(hal_uart_t uart, void *data, uint32_t size, uint32_t timeout);

/**@brief Read the length of the data that is already recived by uart driver and stored in buffer
 *
 * @param uart     : the UART interface
 *
 * @return    Data length
 */
uint32_t hal_uart_get_len_in_buf(hal_uart_t uart); 

/** @} */
/** @} */

#endif
