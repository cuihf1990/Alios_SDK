/**
 ****************************************************************************************
 *
 * @file arch_main.c
 *
 * @brief Main loop of the application.
 *
 * Copyright (C) Beken Corp 2011-2020
 *
 ****************************************************************************************
 */
#include "include.h"
#include "driver_pub.h"
#include "func_pub.h"
#include "mem_pub.h"
#include "rwnx_config.h"
#include "ll.h"
#include "uart_pub.h"
#include "app.h"
#include "ke_event.h"
#include "k_api.h"

#if 1
#define RECORD_COUNT 128
int rec_id = 0;
int rec_array[RECORD_COUNT] = {0};

void rec_playback(void)
{
	int id = rec_id;
	int i;

	for(i = 0; i < 64; i ++)
	{
		id --;
		os_printf("%x,%x\r\n", (id) & (RECORD_COUNT - 1), rec_array[(id) & (RECORD_COUNT - 1)]);
	}
}

void rec_func(int val)
{
	rec_array[(rec_id ++) & (RECORD_COUNT - 1)] = val;
}
#endif

mico_mutex_t stdio_tx_mutex;

static void init_app_thread( void *arg )
{
	mico_rtos_init_mutex( &stdio_tx_mutex );
	application_start();
}

ktask_t task_test22;
cpu_stack_t task_stk[512];

ktask_t *task_test_obj;
ktask_t *task_test_obj2;

extern void fclk_init(void);
extern void test_case_task_start(void);
extern void test_mtbf_task_start(void);

static int test_cnt;

extern void bk_send_byte(UINT8 data);

_ssize_t _write_r(struct _reent *r, int fd, void *buf, size_t len)
{
    int i;
    UINT8 *t;
    t = buf;

    if (fd == 1) {
        for (i = 0; i < len; i++) {

            bk_send_byte(*(t + i));
        }

        return len;
    }

    return 0;
}


void task_test2(void *arg)
{
    /* step 2: function layer initialization*/
    func_init();

    fclk_init();

	app_start();
    return;

    test_case_task_start();

    while (1) {

        //bk_printf("test_cnt$$$666 is %d\r\n", test_cnt++);
       // printf("test_cnt$$$9999 is %d\r\n", test_cnt++);

        yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);

    }
}

void task_test3(void *arg)
{
	mico_semaphore_t sem;

	mico_rtos_init_semaphore(&sem, 0);
    while (1) {

       printf("test_cnt is %d, state %d\r\n", test_cnt++, ke_state_get(0));
		mico_rtos_get_semaphore(&sem, 1000);
        //yunos_task_sleep(YUNOS_CONFIG_TICKS_PER_SECOND);

    }
}



void entry_main(void)
{
    yunos_init();
	/* step 1: driver layer initialization*/
    driver_init();
    /* step 0: system basic component initialization*/
    os_mem_init();

    yunos_task_dyn_create(&task_test_obj, "task_test", 0, 10, 0, 512, task_test2, 1);

    yunos_task_dyn_create(&task_test_obj2, "task_test2", 0, 20, 0, 512, task_test3, 1);
	//app_start();
    yunos_start();
}
// eof

