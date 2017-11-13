/**
 ******************************************************************************
 * @file    MicoDriverUart.h
 * @author  William Xu
 * @version V1.0.0
 * @date    16-Sep-2014
 * @brief   This file provides all the headers of UART operation functions.
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
#include "include.h"
#include "rtos_pub.h"
#include "MiCODriverUart.h"

const mico_uart_config_t test_uart_config[] =
{
    [0] =
    {
        .baud_rate     = 115200,
        .data_width   =    DATA_WIDTH_8BIT,
        .parity  = NO_PARITY,
        .stop_bits = STOP_BITS_1,
        .flow_control = FLOW_CONTROL_DISABLED,
        .flags   = 0,
    },
    [1] =
    {
        .baud_rate     = 19200,
        .data_width   =    DATA_WIDTH_8BIT,
        .parity  = NO_PARITY,
        .stop_bits = STOP_BITS_1,
        .flow_control = FLOW_CONTROL_DISABLED,
        .flags   = 0,
    },
    [2] =
    {
        .baud_rate     = 115200,
        .data_width   =    DATA_WIDTH_8BIT,
        .parity  = EVEN_PARITY,
        .stop_bits = STOP_BITS_1,
        .flow_control = FLOW_CONTROL_DISABLED,
        .flags   = 0,
    },
};

OSStatus MicoUartInitialize_test( mico_uart_t uart, uint8_t config, ring_buffer_t *optional_rx_buffer )
{
    return bk_uart_initialize(uart, &test_uart_config[config], optional_rx_buffer);
}

OSStatus MicoUartInitialize( mico_uart_t uart, const mico_uart_config_t *config, ring_buffer_t *optional_rx_buffer )
{
    return bk_uart_initialize(uart, config, optional_rx_buffer);
}

OSStatus MicoStdioUartInitialize( const mico_uart_config_t *config, ring_buffer_t *optional_rx_buffer )
{
#ifdef STDIO_UART
    return bk_stdio_uart_initialize(config, optional_rx_buffer);
#else
    return 0;
#endif
}

OSStatus MicoUartFinalize( mico_uart_t uart )
{
    return bk_uart_finalize(uart);
}

OSStatus MicoUartSend( mico_uart_t uart, const void *data, uint32_t size )
{
    return bk_uart_send(uart, data, size);
}

OSStatus MicoUartRecv( mico_uart_t uart, void *data, uint32_t size, uint32_t timeout )
{
    return bk_uart_recv(uart, data, size, timeout);
}

OSStatus MicoUartRecvPrefetch ( mico_uart_t uart, void* data, uint32_t size, uint32_t timeout )
{
    return bk_uart_recv_prefetch(uart,data,size,timeout);
}

uint32_t MicoUartGetLengthInBuffer( mico_uart_t uart )
{
    return bk_uart_get_length_in_buffer(uart);
}
// eof

