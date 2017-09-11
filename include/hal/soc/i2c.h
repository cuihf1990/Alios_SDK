/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/soc/i2c.h
 * @brief I2C HAL
 */

#ifndef AOS_I2C_H
#define AOS_I2C_H

typedef struct {
    uint32_t address_width;
    uint32_t freq;
} i2c_config_t;

typedef struct {
    uint8_t      port;    /* i2c port */
    i2c_config_t config;  /* i2c config */
    void        *priv;    /* priv data */
} i2c_dev_t;


/**@brief Initialises an I2C interface
 * @note Prepares an I2C hardware interface for communication as a master or slave
 *
 * @param     device : the device for which the i2c port should be initialised
 * @return    0      : on success.
 * @return    EIO    : if an error occurred during initialisation
 */
int32_t hal_i2c_init(i2c_dev_t *i2c);


/**@brief     i2c master send
 * @param     i2c           : the i2c device
 * @param     dev_addr      : device address
 * @param     data          : i2c send data
 * @param     size          : i2c send data size
 * @param     timeout       : timeout in ms
 * @return    0             : on success.
 * @return    EIO           : if an error occurred during initialisation
 */
int32_t hal_i2c_master_send(i2c_dev_t *i2c, uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout);


/**@brief     i2c master recv
 *
 * @param     i2c         : the i2c device
 * @param     dev_addr    : device address
 * @param     data        : i2c receive data
 * @param     size        : i2c receive data size
 * @param     timeout     : timeout in ms
 * @return    0           : on success.
 * @return    EIO         : if an error occurred during initialisation
 */
int32_t hal_i2c_master_recv(i2c_dev_t *i2c, uint16_t dev_addr, uint8_t *data, uint16_t size, uint32_t timeout);


/**@brief hal_i2C_slave_send
 *
 * @param     i2c         : the i2c device
 * @param     data        : i2c slave send data
 * @param     size        : i2c slave send data size
 * @param     timeout     : timeout in ms
 * @return    0           : on success.
 * @return    EIO         : if an error occurred during initialisation
 */
int32_t hal_i2C_slave_send(i2c_dev_t *i2c, uint8_t *data, uint16_t size, uint32_t timeout);


/**@brief Initialises an I2C interface
 *
 * @param     i2c         : tthe i2c device
 * @param     data        : i2c slave receive data
 * @param     size        : i2c slave receive data size
 * @param     timeout     : timeout in ms
 * @return    0           : on success.
 * @return    EIO         : if an error occurred during initialisation
 */
int32_t hal_i2c_slave_recv(i2c_dev_t *i2c, uint8_t *data, uint16_t size, uint32_t timeout);


/**@brief i2c mem write
 *
 * @param     i2c           : the i2c device
 * @param     dev_addr      : device address
 * @param     mem_addr      : mem address
 * @param     mem_addr_size : mem address
 * @param     data          : i2c master send data
 * @param     size          : i2c master send data size
 * @param     timeout       : timeout in ms
 * @return    0             : on success.
 * @return    EIO           : if an error occurred during initialisation
 */
int32_t hal_i2c_mem_write(i2c_dev_t *i2c, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *data,
                          uint16_t size, uint32_t timeout);

/**@brief i2c master mem read
 * @param     i2c           : the i2c device
 * @param     dev_addr      : device address
 * @param     mem_addr      : mem address
 * @param     mem_addr_size : mem address
 * @param     data          : i2c master send data
 * @param     size          : i2c master send data size
 * @param     timeout       : timeout in ms
 * @return    0             : on success.
 * @return    EIO           : if an error occurred during initialisation
 */
int32_t hal_i2c_mem_read(i2c_dev_t *i2c, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size, uint8_t *data,
                         uint16_t size, uint32_t timeout);


/**@brief Deinitialises an I2C device
 *
 * @param     device : the i2c device
 * @return    0      : on success.
 * @return    EIO    : if an error occurred during deinitialisation
 */
int32_t hal_i2c_finalize(i2c_dev_t *i2c);

#endif


