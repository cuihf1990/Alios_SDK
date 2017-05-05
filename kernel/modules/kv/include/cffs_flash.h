/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CFFS_FLASH_H
#define CFFS_EFLASH_H
typedef int offset_t;

typedef struct flash_info
{
    uint16_t    blk_size;        /* Size of one read/write block. */
    uint32_t    eblock_size;     /* Size of one erase block. */
    uint32_t    total_eblocks;   /* Number of erase blocks. */
} flash_info_t;

typedef struct flash_dev
{
    int (*erase)(struct flash_dev *fdev, off_t eblk, size_t neblks);
    ssize_t (*bread)(struct flash_dev *fdev, off_t ioblk, size_t nblks, void *buffer);
    ssize_t (*bwrite)(struct flash_dev *fdev, off_t ioblk, size_t nblks, const void *buffer);
    ssize_t (*read)(struct flash_dev *fdev, int32_t addroffset, int bytesize, void *buff);
    ssize_t (*write)(struct flash_dev *fdev, int32_t addroffset, int bytesize, const void *buff);
    int (*get_finfo)(struct flash_dev *fdev, flash_info_t *finfo);

    void *priv;
} flash_dev_t;

/**
 * This function will read flash by blocks
 * @param[in]  fdev    pointer to the flash device descriptor
 * @param[in]  ioblk    flash block offset
 * @param[in]  nblks    number of block to erase
 * @param[out]  *buffer    poniter the buffer to save data
 * @return  the operation status, 0 is OK, -1 is error
 */
ssize_t cffs_flash_bread(flash_dev_t *fdev, off_t ioblk, size_t nblks, void *buffer);

/**
 * This function will write flash by blocks
 * @param[in]  fdev    pointer to the flash device descriptor
 * @param[in]  ioblk    flash block offset
 * @param[in]  nblks    number of block to erase
 * @param[in]  *buffer    poniter the data to write
 * @return  the operation status, 0 is OK, -1 is error
 */
ssize_t cffs_flash_bwrite(flash_dev_t *fdev, off_t ioblk, size_t nblks, const void *buffer);

/**
 * This function will read eflash by bytes
 * @param[in]  fdev    pointer to the flash device descriptor
 * @param[in]  addroffset flash address offset
 * @param[in]  bytesize    data length to read
 * @param[out]  *buff    pointer to buffer to save data
 * @return  the operation status, 0 is OK, -1 is error
 */
ssize_t cffs_flash_read(struct flash_dev *fdev, int32_t addroffset, int bytesize, void *buff);

/**
 * This function will write the flash by bytes
 * @param[in]  fdev    pointer to the flash device descriptor
 * @param[in]  addroffset    flash address offset
 * @param[in]  bytesize    data length to write
 * @param[in]  *buff    poniter to buffer to write data
 * @return  the operation status, 0 is OK, -1 is error
 */
ssize_t cffs_flash_write(struct flash_dev *fdev, int32_t addroffset, int bytesize, const void *buff);

/**
 * This function will erase the flash by blocks
 * @param[in]  fdev    pointer to the flash device descriptor
 * @param[in]  eblk    flash block offset
 * @param[in]  neblks  number of block to erase
 * @return  the operation status, 0 is OK, -1 is error
 */
int cffs_flash_erase(flash_dev_t *fdev, off_t eblk, size_t neblks);

/**
 * This function will get the flash infomation
 * @param[in]  fdev    pointer to the flash device descriptor
 * @param[in]  finfo    pointer to the flash infomation
 * @return  the operation status, 0 is OK, -1 is error
 */
int cffs_flash_get_info(flash_dev_t *fdev, flash_info_t *finfo);

#endif /* CFFS_FLASH_H */
