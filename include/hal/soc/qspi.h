/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef HAL_QSPI_H
#define HAL_QSPI_H

typedef struct {
    uint32_t freq;        /* communication frequency Hz */
} qspi_config_t;

typedef struct {
    uint8_t       port;    /* qspi port */
    qspi_config_t config;  /* qspi config */
    void         *priv;    /* priv data */
} qspi_dev_t;

/**
 * Initialises the QSPI interface for a given QSPI device
 *
 * @param[in]  qspi  the spi device
 *
 * @return  0 : on success, EIO : if the QSPI device could not be initialised
 */
int32_t hal_qspi_init(qspi_dev_t *qspi);

/**
 * Qspi send
 *
 * @param[in]  qspi     the qspi device
 * @param[in]  data     spi send data
 * @param[in]  size     spi send data size
 * @param[in]  timeout  timeout in ms
 *
 * @return  0 : on success, EIO : if the QSPI device could not be initialised
 */
int32_t hal_qspi_send(qspi_dev_t *qspi, const uint8_t *data, uint32_t timeout);

/**
 * qspi_recv
 *
 * @param[in]   qspi     the spi device
 * @param[out]  data     qspi recv data
 * @param[in]   size     qspi recv data size
 * @param[in]   timeout  timeout in ms
 *
 * @return  0 : on success, EIO : if the QSPI device could not be initialised
 */
int32_t hal_qspi_recv(qspi_dev_t *qspi, uint8_t *data, uint32_t timeout);


/**
 * De-initialises a QSPI interface
 *
 *
 * @param[in]  qspi the QSPI device to be de-initialised
 *
 * @return  0 : on success, EIO : if an error occurred
 */
int32_t hal_qspi_finalize(qspi_dev_t *qspi);

#endif /* HAL_QSPI_H */

