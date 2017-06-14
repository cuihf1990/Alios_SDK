#include <errno.h>
#include <execinfo.h>
#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include <k_api.h>
#include "hal/soc/soc.h"

typedef unsigned char  		  UINT8;          /* Unsigned  8 bit quantity        */
typedef signed   char  		  INT8;           /* Signed    8 bit quantity        */
typedef unsigned short 		  UINT16;         /* Unsigned 16 bit quantity        */
typedef signed   short 		  INT16;          /* Signed   16 bit quantity        */
typedef uint32_t   		      UINT32;         /* Unsigned 32 bit quantity        */
typedef int32_t   		      INT32;          /* Signed   32 bit quantity        */

static struct termios term_orig;

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
    kmutex_t            tx_mutex;
} _uart_drv_t;

static _uart_drv_t _uart_drv[MAX_UART_NUM];

extern int32_t uart_read_byte( uint8_t *rx_byte );
extern void uart_write_byte( uint8_t byte );
extern uint8_t uart_is_tx_fifo_empty( void );
extern uint8_t uart_is_tx_fifo_full( void );
extern void uart_set_tx_stop_end_int( uint8_t set );

int32_t hal_uart_init(uint8_t uart, const hal_uart_config_t *config)
{
    _uart_drv_t *pdrv = &_uart_drv[0];
    struct termios  term_vi;

    if(pdrv->status == _UART_STATUS_CLOSED)
    {
        yunos_mutex_create( &pdrv->tx_mutex,"TX_MUTEX" );
        pdrv->status = _UART_STATUS_OPENED;
        //system("stty raw -echo");
        tcgetattr(0, &term_orig);
        term_vi = term_orig;
        term_vi.c_lflag &= (~ICANON & ~ECHO);   // leave ISIG ON- allow intr's
        term_vi.c_iflag &= (~IXON & ~ICRNL);
        term_vi.c_oflag &= (~ONLCR);
#ifndef linux
        term_vi.c_cc[VMIN] = 1;
        term_vi.c_cc[VTIME] = 0;
#endif
        tcsetattr(0, TCSANOW, &term_vi);
    }
    return 0;
}

int32_t hal_stdio_uart_init(const hal_uart_config_t *config)
{

}

int32_t hal_uart_finalize(uint8_t uart)
{
    _uart_drv_t *pdrv = &_uart_drv[uart];


    yunos_mutex_del(&pdrv->tx_mutex);
    pdrv->status = _UART_STATUS_CLOSED;
    //system("stty raw echo");
    tcsetattr(0, TCSANOW, &term_orig);
    return 0;
}

int32_t hal_uart_send(uint8_t uart, const void *data, uint32_t size)
{
    uint32_t i = 0;
    _uart_drv_t *pdrv = &_uart_drv[uart];

    yunos_mutex_lock(&pdrv->tx_mutex, YUNOS_WAIT_FOREVER);

    for( i = 0; i < size; i++ )
    {
        putchar( ((uint8_t *)data)[i] );
    }

    yunos_mutex_unlock( &pdrv->tx_mutex );

    return 0;
}

int32_t hal_uart_recv(uint8_t uart, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout)
{
    int            in = 0;
    uint32_t       plen = 0;
    char          *pin = data;
    fd_set         rfds;
    struct timeval tv;
    int            ret;
    int            retval;
    tick_t         yield_ticks = YUNOS_CONFIG_TIME_SLICE_DEFAULT;


    //sigaddset(&sigset, SIGALRM);
    //ret = pthread_sigmask(SIG_BLOCK, &sigset, NULL);

    //if(ret != 0) {
    //    perror("sigmask failed");
    //    return NULL;
    //}


    /* watch stdin (fd 0) to see when it has input. */
    do {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);

        /* no wait */
        tv.tv_sec  = 0;
        tv.tv_usec = YUNOS_CONFIG_TIME_SLICE_DEFAULT;

        retval = select(1, &rfds, NULL, NULL, &tv);

        if (retval < 0) {
            if(errno == EINTR){
                continue;
            }
            return -1;
        }

        if (retval == 0) {
            yunos_task_sleep(10);
            continue;
        }

        /* not block for select tells */
        read(0, &in, 1);
        if (in < 0 && errno != EINTR) {
            return -1;
        }
        if(in == '\n'){
            in = '\r';
        }
        *pin = in;
        pin++;
        plen++;
        if(plen >= expect_size){
            if(recv_size){
                *recv_size = plen;
            }
            break;
        }
    }while(1);

    return 0;

}


uint32_t hal_uart_get_len_in_buf(int uart)
{
    return 0;
}

