#include "hal/soc/soc.h"
#include "ringbuf.h"
#include "mico_rtos.h"

typedef unsigned char  		  UINT8;          /* Unsigned  8 bit quantity        */
typedef signed   char  		  INT8;           /* Signed    8 bit quantity        */
typedef unsigned short 		  UINT16;         /* Unsigned 16 bit quantity        */
typedef signed   short 		  INT16;          /* Signed   16 bit quantity        */
typedef uint32_t   		      UINT32;         /* Unsigned 32 bit quantity        */
typedef int32_t   		      INT32;          /* Signed   32 bit quantity        */

#include "uart_pub.h"

#define MAX_UART_NUM 1

enum _uart_status_e
{
    _UART_STATUS_CLOSED = 0,
    _UART_STATUS_OPENED,
};

typedef struct
{
    uint8_t             uart;
    uint8_t             status;
    uint8_t             *rx_buf;
    uint32_t            rx_size;
    ring_buffer_t       rx_ringbuf;
    mico_semaphore_t    rx_semphr;
    mico_semaphore_t    tx_semphr;
    mico_mutex_t        tx_mutex;
} _uart_drv_t;

static _uart_drv_t _uart_drv[MAX_UART_NUM];

extern int32_t uart_read_byte( uint8_t *rx_byte );
extern void uart_write_byte( uint8_t byte );
extern uint8_t uart_is_tx_fifo_empty( void );
extern uint8_t uart_is_tx_fifo_full( void );
extern void uart_set_tx_stop_end_int( uint8_t set );

int32_t hal_uart_init(uint8_t uart, const hal_uart_config_t *config)
{
    _uart_drv_t *pdrv = &_uart_drv[uart];

    if(pdrv->status == _UART_STATUS_CLOSED)
    {
        pdrv->rx_buf = (uint8_t *)malloc(config->rx_buf_size);
        ring_buffer_init(&pdrv->rx_ringbuf, pdrv->rx_buf, config->rx_buf_size);

        mico_rtos_init_semaphore( &pdrv->tx_semphr, 0 );
        mico_rtos_init_semaphore( &pdrv->rx_semphr, 0 );
        mico_rtos_init_mutex( &pdrv->tx_mutex );
    }

    while(!uart_is_tx_fifo_empty());

    uart_open(0);
    uart_ctrl(CMD_UART_INIT, config);

    return 0;
}

int32_t hal_stdio_uart_init(const hal_uart_config_t *config)
{

}

int32_t hal_uart_finalize(uint8_t uart)
{
    _uart_drv_t *pdrv = &_uart_drv[uart];

    while(!uart_is_tx_fifo_empty());

    uart_ctrl(CMD_UART_RESET, NULL);
    uart_close();

    ring_buffer_deinit(&pdrv->rx_ringbuf);
    free(pdrv->rx_buf);

    mico_rtos_deinit_semaphore(&pdrv->rx_semphr);
    mico_rtos_deinit_semaphore(&pdrv->tx_semphr);
    mico_rtos_deinit_mutex(&pdrv->tx_mutex);
}

int32_t hal_uart_send(uint8_t uart, const void *data, uint32_t size)
{
    uint32_t i = 0;
    _uart_drv_t *pdrv = &_uart_drv[uart];

    mico_rtos_lock_mutex( &pdrv->tx_mutex );

    for( i = 0; i < size; i++ )
    {
        if( uart_is_tx_fifo_full() )
        {
            uart_set_tx_stop_end_int( 1 );
            /* The data in Tx FIFO may have been sent out before enable TX_STOP_END interrupt */
            /* So double check the FIFO status */
            if( !uart_is_tx_fifo_empty() )
                mico_rtos_get_semaphore( &pdrv->tx_semphr, MICO_WAIT_FOREVER );

            uart_set_tx_stop_end_int( 0 );
        }

        uart_write_byte( ((uint8_t *)data)[i] );
    }

    mico_rtos_unlock_mutex( &pdrv->tx_mutex );

    return 0;
}

int32_t hal_uart_recv(uint8_t uart, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout)
{
    uint32_t read_size, actual_size, tmp;
    uint32_t ringbuf_size;
    uint32_t start_time, expired_time;
    _uart_drv_t *pdrv = &_uart_drv[uart];

    recv_size = recv_size == NULL ? &actual_size : recv_size;

    start_time = mico_rtos_get_time();
    expired_time = 0;

    *recv_size = 0;
    ringbuf_size = ring_buffer_total_size(&pdrv->rx_ringbuf);

    for(;;)
    {
        read_size = expect_size > ringbuf_size ? ringbuf_size : expect_size;

        if(read_size > ring_buffer_used_space( &pdrv->rx_ringbuf ))
        {
            pdrv->rx_size = read_size;

            if ( mico_rtos_get_semaphore( &pdrv->rx_semphr, timeout - expired_time) != kNoErr )
            {
                pdrv->rx_size = 0;
                read_size = ring_buffer_used_space( &pdrv->rx_ringbuf );
                ring_buffer_read(&pdrv->rx_ringbuf, data, read_size, &tmp);
                *recv_size += read_size;
                return -1;
            }
        }

        ring_buffer_read(&pdrv->rx_ringbuf, data, read_size, &tmp);

        data += read_size;
        *recv_size += read_size;
        expect_size -= read_size;

        if(expect_size == 0)
        {
            break;
        }

        expired_time = mico_rtos_get_time() - start_time;
        if(expired_time > timeout)
        {
            return -1;
        }
    }

    return 0;
}

uint32_t hal_uart_get_len_in_buf(int uart)
{

}

void uart_rx_cb(void)
{
    uint8_t rx_byte;
    uint8_t uart = 0;
    _uart_drv_t *pdrv = &_uart_drv[uart];

    while(uart_read_byte(&rx_byte) == 0)
    {
        ring_buffer_write( &pdrv->rx_ringbuf, &rx_byte,1 );
    }

    // Notify thread if sufficient data are available
    if ( pdrv->rx_size > 0 && ring_buffer_used_space( &pdrv->rx_ringbuf ) >= pdrv->rx_size )
    {
        mico_rtos_set_semaphore( &pdrv->rx_semphr );
        pdrv->rx_size = 0;
    }  
}

void uart_tx_cb(void)
{
    uint8_t uart = 0;
    _uart_drv_t *pdrv = &_uart_drv[uart];

    mico_rtos_set_semaphore( &pdrv->tx_semphr );
}