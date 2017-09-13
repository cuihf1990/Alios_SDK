/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/soc/spi.h
 * @brief SPI HAL
 * @version since 5.5.0
 */

#ifndef AOS_SPI_H
#define AOS_SPI_H

typedef struct {
    uint32_t mode;
    uint32_t freq;
} spi_config_t;

typedef struct {
    uint8_t      port;    /* spi port */
    spi_config_t config;  /* spi config */
    void        *priv;    /* priv data */
} spi_dev_t;


/**
 * Initialises the SPI interface for a given SPI device
 *
 * @param[in]  spi  the spi device
 *
 * @return     0    on success.
 * @return     EIO   if the SPI device could not be initialised
 */
int32_t hal_spi_init(spi_dev_t *spi);


/**
 * spi_send
 *
 * @param[in]  spi      the spi device
 * @param[in]  data     spi send data
 * @param[in]  size     spi send data size
 * @param[in]  timeout  timeout in ms
 *
 * @return     0        on success.
 * @return     EIO      if the SPI device could not be initialised
 */
int32_t hal_spi_send(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * spi_recv
 *
 * @param[in]  spi      the spi device
 * @param[out] data     spi recv data
 * @param[in]  size     spi recv data size
 * @param[in]  timeout  timeout in ms
 *
 * @return     0        on success.
 * @return     EIO      if the SPI device could not be initialised
 */
int32_t hal_spi_recv(spi_dev_t *spi, uint8_t *data, uint16_t size, uint32_t timeout);

/**
 * spi send data and recv
 *
 * @param[in]  spi      the spi device
 * @param[in]  tx_data  spi send data
 * @param[in]  rx_data  spi recv data
 * @param[in]  tx_size  spi data to be sent
 * @param[in]  rx_size  spi data to be recv
 * @param[in]  timeout  timeout in ms
 *
 * @return     0        on success.
 * @return     EIO      if the SPI device could not be initialised
 */
int32_t hal_spi_send_recv(spi_dev_t *spi, uint8_t *tx_data, uint16_t tx_size, uint8_t *rx_data, uint16_t rx_size,
                          uint32_t timeout);

/**
 * De-initialises a SPI interface
 *
 * @param[in]  spi  the SPI device to be de-initialised
 *
 * @return     0    on success.
 * @return     EIO  if an error occurred
 */
int32_t hal_spi_finalize(spi_dev_t *spi);

#endif
