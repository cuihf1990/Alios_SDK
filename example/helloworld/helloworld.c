#include "hal/soc/soc.h"
#include "helloworld.h"

const hal_uart_config_t config = 
{
    .baud_rate = 115200,
    .data_width = DATA_WIDTH_8BIT,
    .parity = NO_PARITY,
    .stop_bits = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
    .rx_buf_size = 256,
};

void application_start(void)
{
    uint32_t rx_size;
    static char rx_buf[1024];

    strcpy(rx_buf, "hello world\r\n");
    rx_size = strlen(rx_buf);

    hal_uart_init(0, &config);

    for(;;)
    {
        rx_size = 12;
        hal_uart_recv(0, rx_buf, &rx_size, 0xffffffffu);

        hal_uart_send(0, rx_buf, rx_size);
    }

    hal_uart_finalize(0);
}

