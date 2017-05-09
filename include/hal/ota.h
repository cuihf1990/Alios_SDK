/**
 ******************************************************************************
 * @file    ota.h
 * @author  ting.guit
 * @version V1.0.0
 * @date    1-may-2017
 * @brief   This file provides all the headers of ota operation functions.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2014 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */

#ifndef __OTA_H__
#define __OTA_H__

#pragma once
#include "common.h"
#include "platform.h"


 /******************************************************
 *                 Function Declarations
 ******************************************************/


/**@brief  init ota partition
 *
 * @note   when ota start ,maybe it need init something
 *
 * @return kNoErr        : On success.
 * @return kGeneralErr   : If an error occurred with any step
 */
hal_stat_t hal_ota_init();

/**@brief  Write data to an area on ota partition
 *
 * @param  off_set        : Point to the start address that the data is written to, and
 *                          point to the last unwritten address after this function is 
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param  inbuf          : point to the data buffer that will be written to flash
 * @param  in_buf_len     : The length of the buffer
 *
 * @return kNoErr         : On success.
 * @return kGeneralErr    : If an error occurred with any step
 */
hal_stat_t hal_ota_write(volatile uint32_t* off_set, uint8_t* in_buf ,uint32_t in_buf_len);

/**@brief  Read data from an area on ota Flash to data buffer in RAM
 *
 * @param  in_partition   : The target flash logical partition which should be read
 * @param  off_set        : Point to the start address that the data is read, and
 *                          point to the last unread address after this function is 
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param  out_buf        : Point to the data buffer that stores the data read from flash
 * @param  in_buf_len     : The length of the buffer
 *
 * @return kNoErr         : On success.
 * @return kGeneralErr    : If an error occurred with any step
 */
hal_stat_t hal_ota_read(volatile uint32_t* off_set, uint8_t* out_buf, uint32_t in_buf_len);

/**@brief  Set boot options when ota reboot
 *
 * @return kNoErr         : On success.
 * @return kGeneralErr    : If an error occurred with any step
 */
hal_stat_t hal_ota_set_boot();

#endif


