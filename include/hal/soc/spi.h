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

/**
 * @file hal/soc/spi.h
 * @brief SPI HAL
 * @version since 5.5.0
 */

#ifndef YOS_SPI_H
#define YOS_SPI_H

typedef struct {
    uint32_t mode;
    uint32_t freq;
} spi_config_t;

typedef struct {
    uint8_t      port;    /* spi port */
    spi_config_t config;  /* spi config */
    void        *priv;    /* priv data */
} spi_dev_t;


/**@brief Initialises the SPI interface for a given SPI device
 * @note  Prepares a SPI hardware interface for communication as a master
 *
 * @param     spi         : the spi device
 * @return    kNoErr      : on success.
 * @return    kGeneralErr : if the SPI device could not be initialised
 */
int32_t hal_spi_init(spi_dev_t *spi);


/**@brief spi_send
 *
 * @param     spi           : the spi device
 * @param     data          : spi send data
 * @param     size          : spi send data size
 * @param     timeout       : timeout in ms
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if the SPI device could not be initialised
 */
int32_t hal_spi_send(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout);

/**@brief spi_recv
 * @param     spi         : the spi device
 * @param     data        : spi recv data
 * @param     size        : spi recv data size
 * @param     timeout     : timeout in ms
 * @return    kNoErr      : on success.
 * @return    kGeneralErr : if the SPI device could not be initialised
 */
int32_t hal_spi_recv(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout);

/**@brief spi send data and recv
 * @param     spi         : the spi device
 * @param     tx_data     : spi send data
 * @param     rx_data     : spi recv data
 * @param     tx_size     : spi data to be sent
 * @param     rx_size     : spi data to be recv
 * @param     timeout     : timeout in ms
 * @return    kNoErr      : on success.
 * @return    kGeneralErr : if the SPI device could not be initialised
 */
int32_t hal_spi_send_recv(spi_dev_t *spi, uint8_t *tx_data, uint16_t tx_size, uint8_t *rx_data, uint16_t rx_size, uint32_t timeout);

/**@brief De-initialises a SPI interface
 *
 * @note Turns off a SPI hardware interface
 *
 * @param  spi : the SPI device to be de-initialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred
 */
int32_t hal_spi_finalize(spi_dev_t *spi);

#endif
