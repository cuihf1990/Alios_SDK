/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/soc/uart.h
 * @brief UART HAL
 * @version since 5.5.0
 */

#ifndef YOS_UART_H
#define YOS_UART_H

/**
 * UART data width
 */
typedef enum {
    DATA_WIDTH_5BIT,
    DATA_WIDTH_6BIT,
    DATA_WIDTH_7BIT,
    DATA_WIDTH_8BIT,
    DATA_WIDTH_9BIT
} hal_uart_data_width_t;

/**
 * UART stop bits
 */
typedef enum {
    STOP_BITS_1,
    STOP_BITS_2,
} hal_uart_stop_bits_t;

/**
 * UART flow control
 */
typedef enum {
    FLOW_CONTROL_DISABLED,
    FLOW_CONTROL_CTS,
    FLOW_CONTROL_RTS,
    FLOW_CONTROL_CTS_RTS
} hal_uart_flow_control_t;

/**
 * UART parity
 */
typedef enum {
    NO_PARITY,
    ODD_PARITY,
    EVEN_PARITY,
} hal_uart_parity_t;

/**
 * UART configuration
 */
typedef struct {
    uint32_t                baud_rate;
    hal_uart_data_width_t   data_width;
    hal_uart_parity_t       parity;
    hal_uart_stop_bits_t    stop_bits;
    hal_uart_flow_control_t flow_control;
} uart_config_t;

typedef struct {
    uint8_t       port;    /* uart port */
    uart_config_t config;  /* uart config */
    void         *priv;    /* priv data */
} uart_dev_t;


/**@brief Initialises a UART interface
 *
 * @note Prepares an UART hardware interface for communications
 *
 * @param  uart     : the interface which should be initialised
 * @param  config   : UART configuration structure
 * @param  optional_rx_buffer : Pointer to an optional RX ring buffer
 *
 * @return    0     : on success.
 * @return    EIO   : if an error occurred with any step
 */
int32_t hal_uart_init(uart_dev_t *uart);


/**@brief Transmit data on a UART interface
 *
 * @param  uart     : the UART interface
 * @param  data     : pointer to the start of data
 * @param  size     : number of bytes to transmit
 *
 * @return 0        : on success.
 * @return EIO      : if an error occurred with any step
 */
int32_t hal_uart_send(uart_dev_t *uart, void *data, uint32_t size, uint32_t timeout);


/**@brief Receive data on a UART interface
 *
 * @param  uart     : the UART interface
 * @param  data     : pointer to the buffer which will store incoming data
 * @param  size     : number of bytes to receive
 * @param  timeout  : timeout in milisecond
 *
 * @return 0        : on success.
 * @return EIO      : if an error occurred with any step
 */
int32_t hal_uart_recv(uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout);


/**@brief Deinitialises a UART interface
 *
 * @param  uart : the interface which should be deinitialised
 *
 * @return 0    : on success.
 * @return EIO  : if an error occurred with any step
 */
int32_t hal_uart_finalize(uart_dev_t *uart);

#endif
