#include <sys/unistd.h>
#include <sys/errno.h>
#include "hal/soc/soc.h"

extern int errno;

const hal_uart_config_t uart_cfg = {
    .baud_rate = 921600,
    .data_width = DATA_WIDTH_8BIT,
    .parity = NO_PARITY,
    .stop_bits = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
    .rx_buf_size = 256,
};

void hal_init(void)
{
    hal_uart_init(0, &uart_cfg);

#ifdef BOOTLOADER
    /* init for bootloader */
#else
    /* init for application */
#endif
}

void hal_boot(hal_partition_t partition)
{
    uint32_t addr;

    intc_deinit();

    addr = hal_flash_get_info(partition)->partition_start_addr;
    __asm volatile ("BX %0" : : "r" (addr) );
}

void hal_reboot(void)
{
    hal_wdg_init(1);
}

int _write( int file, char *ptr, int len )
{
    switch ( file )
    {
        case STDOUT_FILENO: /*stdout*/
        case STDERR_FILENO: /* stderr */
            break;
        default:
            errno = EBADF;
            return -1;
    }

    for (int i = 0; i < len; i++) {
        if (*ptr == '\n')
            hal_uart_send( 0, (const void*)"\r", 1 );
        hal_uart_send( 0, (const void*)ptr, 1 );
        ptr ++;
    }

    return len;
}
