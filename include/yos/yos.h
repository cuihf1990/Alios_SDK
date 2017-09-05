/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#ifndef YOS_H
#define YOS_H

#include <yos/kv.h>
#include <yos/list.h>
#include <yos/log.h>
#include <yos/vfs.h>
#include <yos/yloop.h>
#include <yos/version.h>

/*
#include <yos/kernel.h>
#include <yos/alink.h>
#include <yos/cli.h>
#include <yos/cloud.h>
#include <yos/debug.h>
#include <yos/network.h>
#include <yos/types.h>
*/

/**@brief Transmit data on a UART interface
 *
 * @param  data     : pointer to the start of data
 * @param  size     : number of bytes to transmit
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int32_t yos_uart_send(void *data, uint32_t size, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* YOS_H */

