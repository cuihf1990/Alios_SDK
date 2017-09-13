/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef AOS_H
#define AOS_H

#include <aos/cli.h>
#include <aos/cloud.h>
#include <aos/debug.h>
#include <aos/kernel.h>
#include <aos/kv.h>
#include <aos/list.h>
#include <aos/log.h>
#include <aos/types.h>
#include <aos/vfs.h>
#include <aos/version.h>
#include <aos/yloop.h>

/*
#include <aos/alink.h>
#include <aos/network.h>
*/

/**
 * Transmit data on a UART interface
 *
 * @param  data     pointer to the start of data
 * @param  size     number of bytes to transmit
 * @param  timeout  time to wait
 *
 * @return  0 success, EIO if an error occurred with any step
 */
int32_t aos_uart_send(void *data, uint32_t size, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* AOS_H */

