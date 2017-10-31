#include <stdio.h>
#include <stdint.h>

#include <hal/soc/uart.h>

int32_t hal_uart_send(uart_dev_t *uart, void *data, uint32_t size, uint32_t timeout)
{
    fwrite(data, size, 1, stdout);
    return 0;
}

int32_t hal_uart_recv(uart_dev_t *uart, void *data, uint32_t expect_size, uint32_t *recv_size, uint32_t timeout)
{
    int ttl_len = 0;
    char *buf = data;
    while (1) {
        char c;
        int ret = fread(&c, 1, 1, stdin);
        if (ret <= 0)
            break;

        *buf++ = c;
        ttl_len ++;
        if (ttl_len >= expect_size)
            break;
    }

    if (recv_size)
        *recv_size = ttl_len;

    return ttl_len > 0 ? 0 : -1;
}

extern void uart_driver_install();
int32_t hal_uart_init(uart_dev_t *uart)
{
    uart_driver_install(uart->port, 256, 0, 0, NULL, 0);
}

int32_t hal_uart_finalize(uart_dev_t *uart)
{
}

void hal_reboot(void)
{
    printf("reboot!\n");
}

