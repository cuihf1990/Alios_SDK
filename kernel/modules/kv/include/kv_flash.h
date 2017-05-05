/****************************************************************************
 * id2kernel/rhino/fs/kv/kv_flash.h
 *
 *
 * Copyright (C) 2015-2016 The YunOS Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 ****************************************************************************/

#ifndef KV_FLASH
#define KV_FLASH

#include <unistd.h>

/****************************************************************************
 * Included Files
 ****************************************************************************/
#define EBLOCK_NUM    3
#define EBLOCK_START  488
#define EBLOCK_SIZE   512
#define EBLOCK_FOR_GC 1

/**
 * This function will read the flash by bytes
 * @param[in]  bytesize data length to read
 * @param[in]  *buff    poniter to buffer to read data
 * @param[in]  off      offset inside the block
 * @param[in]  blk_num  idx of reading block
 * @return  the operation status, 0 is OK, -1 is error
 */
ssize_t yoc_flash_read(int32_t bytesize, void *buff, int32_t off, int32_t blk_num);

/**
 * This function will write the flash by bytes
 * @param[in]  bytesize data length to write
 * @param[in]  *buff    poniter to buffer to write data
 * @param[in]  off      offset inside the block
 * @param[in]  blk_num  idx of writting block
 * @return  the operation status, 0 is OK, -1 is error
 */
ssize_t yoc_flash_write(int32_t bytesize, void *buff, int32_t off, int32_t blk_num);

/**
 * This function will erase the flash by blocks
 * @param[in]  blk_num  idx of block to erase
 * @return  the operation status, 0 is OK, -1 is error
 */
ssize_t yoc_flash_erase(int32_t blk_num);

#endif /* KV_FLASH */

