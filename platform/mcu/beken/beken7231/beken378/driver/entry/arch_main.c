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

void entry_main(void)
{  
    /* step 1: driver layer initialization*/
    driver_init();

    /* step 2: function layer initialization*/
    func_init();  

	app_start();
         
    vTaskStartScheduler();
}
// eof

