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

ktask_t *syst_init_obj;
ktask_t *task_test_obj2;

extern void fclk_init(void);
extern void test_case_task_start(void);
extern void test_mtbf_task_start(void);

static int test_cnt;

void system_init(void *arg)
{
    /* step 2: function layer initialization*/
    func_init();

    fclk_init();

    platform_init();

#ifdef YOS_NO_WIFI
    main();
#else
	app_start();
#endif

    return;
}

void task_test3(void *arg)
{
	mico_semaphore_t sem;

	mico_rtos_init_semaphore(&sem, 0);
    while (1) {
        printf("test_cnt is %d\r\n", test_cnt++);
		mico_rtos_get_semaphore(&sem, 1000);
    }
}



void entry_main(void)
{
    yunos_init();

    /* step 1: driver layer initialization*/
    driver_init();

    yunos_task_dyn_create(&syst_init_obj, "system_init", 0, 10, 0, 512, system_init, 1);

    /* yunos_task_dyn_create(&task_test_obj2, "task_test2", 0, 20, 0, 512, task_test3, 1); */
	//app_start();
    yunos_start();
}
// eof

