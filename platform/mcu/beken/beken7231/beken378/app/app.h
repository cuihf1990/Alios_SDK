#ifndef _APP_H_
#define _APP_H_

#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "rtos_pub.h"

//#define APP_DEBUG

#ifdef APP_DEBUG
#define APP_PRT       os_printf
#define APP_WPRT      warning_prf
#else
#define APP_PRT       os_null_printf
#define APP_WPRT      warning_prf
#endif

enum
{
	BMSG_NULL_TYPE = 0,
	BMSG_RX_TYPE   = 1,
	BMSG_TX_TYPE   = 2,
	BMSG_IOCTL_TYPE = 3,
	BMSG_SKT_TX_TYPE   = 4,
};

typedef struct bus_message 
{
	uint32_t type;
	uint32_t arg;
	uint32_t len;
	beken_semaphore_t sema;
}BUS_MSG_T;

#define CORE_QITEM_COUNT          (64)
#define CORE_STACK_SIZE           (4 * 1024)

typedef struct _wifi_core_
{
	uint32_t queue_item_count;
	beken_queue_t io_queue;
	
	xTaskHandle handle;
	uint32_t stack_size;
}WIFI_CORE_T;

void app_start(void);
void core_thread_uninit(void);

#endif // _APP_H_
// eof

