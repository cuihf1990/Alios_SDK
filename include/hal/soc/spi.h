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


#ifndef YOS_SPI_H
#define YOS_SPI_H

#pragma once
#include "common.h"
#include "board_platform.h"
#include "platform_peripheral.h"

/** @addtogroup HAL_PLATFORM
* @{
*/
/** @defgroup hal_SPI MICO SPI Driver
* @brief  Serial Peripheral Interface (SPI) Functions
* @{
*/
/******************************************************
 *                    Constants
 ******************************************************/
/* SPI mode constants */
#define SPI_CLOCK_RISING_EDGE  ( 1 << 0 )
#define SPI_CLOCK_FALLING_EDGE ( 0 << 0 )
#define SPI_CLOCK_IDLE_HIGH    ( 1 << 1 )
#define SPI_CLOCK_IDLE_LOW     ( 0 << 1 )
#define SPI_USE_DMA            ( 1 << 2 )
#define SPI_NO_DMA             ( 0 << 2 )
#define SPI_MSB_FIRST          ( 1 << 3 )
#define SPI_LSB_FIRST          ( 0 << 3 )


/******************************************************
 *                   Enumerations
 ******************************************************/


/******************************************************
 *                    Structures
 ******************************************************/

typedef enum
{
    HAL_SPI_0,
    HAL_SPI_1,
    HAL_SPI_2,
    HAL_SPI_3,
    HAL__SPI_MAX,
} hal_spi_t;

typedef struct
{
    hal_spi_t  port;
    hal_gpio_t chip_select;
    uint32_t   speed;
    uint8_t    mode;
    uint8_t    bits;
} hal_spi_device_t;

typedef platform_spi_message_segment_t hal_spi_msg_segment_t;

/******************************************************
 *                     Variables
 ******************************************************/
 
/******************************************************
 *                 Function Declarations
 ******************************************************/



/**@brief Initialises the SPI interface for a given SPI device
 *
 * @note  Prepares a SPI hardware interface for communication as a master
 *
 * @param  spi : the SPI device to be initialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if the SPI device could not be initialised
 */
int hal_spi_init(const hal_spi_device_t *spi);


/**@brief Transmits and/or receives data from a SPI device
 *
 * @param  spi      : the SPI device to be initialised
 * @param  segments : a pointer to an array of segments
 * @param  number_of_segments : the number of segments to transfer
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred
 */
int hal_spi_transfer(const hal_spi_device_t *spi, const hal_spi_msg_segment_t *segments, uint16_t num);


/**@brief De-initialises a SPI interface
 *
 * @note Turns off a SPI hardware interface
 *
 * @param  spi : the SPI device to be de-initialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred
 */
int hal_spi_finalize(const hal_spi_device_t *spi);

/** @} */
/** @} */

#endif
