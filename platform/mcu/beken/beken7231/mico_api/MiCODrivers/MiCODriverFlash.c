/**
 ******************************************************************************
 * @file    MicoDriverFlash.h
 * @author  William Xu
 * @version V1.0.0
 * @date    16-Sep-2014
 * @brief   This file provides all the headers of Flash operation functions.
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
#include "include.h"
#include "rtos_pub.h"
#include "MiCODriverFlash.h"

mico_logic_partition_t* MicoFlashGetInfo( mico_partition_t inPartition )
{
    return bk_flash_get_info(inPartition);
}

OSStatus MicoFlashErase(mico_partition_t inPartition, uint32_t off_set, uint32_t size)
{
    return bk_flash_erase(inPartition,off_set,size);
}

OSStatus MicoFlashWrite( mico_partition_t inPartition, volatile uint32_t* off_set, uint8_t* inBuffer ,uint32_t inBufferLength)
{
    return bk_flash_write(inPartition,off_set,inBuffer,inBufferLength);
}

OSStatus MicoFlashRead( mico_partition_t inPartition, volatile uint32_t* off_set, uint8_t* outBuffer, uint32_t inBufferLength)
{
    return bk_flash_read(inPartition,off_set,outBuffer,inBufferLength);
}

OSStatus MicoFlashEnableSecurity( mico_partition_t partition, uint32_t off_set, uint32_t size )
{
    return bk_flash_enable_security(partition,off_set,size);
}

// eof



