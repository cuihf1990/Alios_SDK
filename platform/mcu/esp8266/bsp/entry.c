#include <stdio.h>
#include <stdint.h>

#include <k_api.h>

#include <hal/soc/uart.h>
#include <aos/aos.h>

extern int ets_printf(const char *fmt, ...);

extern char _bss_start;
extern char _bss_end;

static ktask_t *g_aos_init;

uart_dev_t uart_0 = {
    .port = 0,
};

static kinit_t kinit = {
    .argc = 0,
    .argv = NULL,
    .cli_enable = 1
};

static void app_entry(void *arg)
{
#if 0
    aos_kernel_init(&kinit);
#else
    while (1) {
        ets_printf("%s:%d tick %lld\n", __func__, __LINE__, krhino_sys_tick_get());
        krhino_task_sleep(RHINO_CONFIG_TICKS_PER_SECOND);
    }
#endif
}

static void init_bss_data(void)
{
    int* p;

    for (p = (int*)&_bss_start; p < (int*)&_bss_end; p++) {
        *p = 0;
    }
}

extern int _text_start;

void call_user_start(void)
{
    asm volatile("wsr    %0, vecbase\n" \
                 ::"r"(&_text_start));

    init_bss_data();

    ets_printf("esp8266 mcu start\n");

    aos_init();

    aos_task_new("main", app_entry, 0, 8192);

    aos_start();

    /* Should never get here, unless there is an error in vTaskStartScheduler */
    for(;;) ;
}
