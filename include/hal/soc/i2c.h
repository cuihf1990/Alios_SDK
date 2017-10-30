/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef HAL_I2C_H
#define HAL_I2C_H

#define I2C_MODE_MASTER 1
#define I2C_MODE_SLAVE  2
#define I2C_MODE_MEM    3

typedef struct {
    uint32_t address_width;
    uint32_t freq;
    uint8_t  mode;
} i2c_config_t;

typedef struct {
    uint8_t      port;    /* i2c port */
    i2c_config_t config;  /* i2c config */
    void        *priv;    /* priv data */
} i2c_dev_t;

/**
 * Initialises an I2C interface
 * Prepares an I2C hardware interface for communication as a master or slave
 *
 * @param[in]  i2c  the device for which the i2c port should be initialised
 *
 * @return  0 : on success, EIO : if an error occurred during initialisation
 */
int32_t hal_i2c_init(i2c_dev_t *i2c);

/**
 * I2c send
 *
 * @param[in]  i2c      the i2c device
 * @param[in]  data     i2c slave send data
 * @param[in]  size     i2c slave send data size
 * @param[in]  timeout  timeout in ms
 *
 * @return  0 : on success, EIO : if an error occurred during initialisation
 */
int32_t hal_i2c_send(i2c_dev_t *i2c, void *data, size_t size, uint32_t timeout);

/**
 * I2c receive
 *
 * @param[in]   i2c      tthe i2c device
 * @param[out]  data     i2c slave receive data
 * @param[in]   size     i2c slave receive data size
 * @param[in]   timeout  timeout in ms
 *
 * @return  0 : on success, EIO : if an error occurred during initialisation
 */
int32_t hal_i2c_recv(i2c_dev_t *i2c, void *data, size_t size, uint32_t timeout);

/**
 * Deinitialises an I2C device
 *
 * @param[in]  i2c  the i2c device
 *
 * @return  0 : on success, EIO : if an error occurred during deinitialisation
 */
int32_t hal_i2c_finalize(i2c_dev_t *i2c);

#endif /* HAL_I2C_H */

