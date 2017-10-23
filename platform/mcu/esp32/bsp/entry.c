#include <stdio.h>
#include <stdint.h>

#include <hal/soc/uart.h>
#include <aos/aos.h>

#include <sdkconfig.h>

uart_dev_t uart_0 = {
    .port = CONFIG_CONSOLE_UART_NUM,
};

extern int aos_kernel_init(void);
static void app_entry(void *arg)
{
    aos_kernel_init();
}

extern uart_dev_t uart_0;
void app_main(void)
{
    hal_uart_init(&uart_0);
    aos_task_new("main", app_entry, 0, 4096);
}
