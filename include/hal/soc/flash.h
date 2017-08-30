/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/soc/flash.h
 * @brief PWM HAL
 * @version since 5.5.0
 */

#ifndef YOS_FLASH_H
#define YOS_FLASH_H

#define PAR_OPT_READ_POS      ( 0 )
#define PAR_OPT_WRITE_POS     ( 1 )

#define PAR_OPT_READ_MASK     ( 0x1u << PAR_OPT_READ_POS )
#define PAR_OPT_WRITE_MASK    ( 0x1u << PAR_OPT_WRITE_POS )

#define PAR_OPT_READ_DIS      ( 0x0u << PAR_OPT_READ_POS )
#define PAR_OPT_READ_EN       ( 0x1u << PAR_OPT_READ_POS )
#define PAR_OPT_WRITE_DIS     ( 0x0u << PAR_OPT_WRITE_POS )
#define PAR_OPT_WRITE_EN      ( 0x1u << PAR_OPT_WRITE_POS )

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef enum {
    HAL_PARTITION_ERROR = -1,
    HAL_PARTITION_BOOTLOADER,
    HAL_PARTITION_APPLICATION,
    HAL_PARTITION_ATE,
    HAL_PARTITION_OTA_TEMP,
    HAL_PARTITION_RF_FIRMWARE,
    HAL_PARTITION_PARAMETER_1,
    HAL_PARTITION_PARAMETER_2,
    HAL_PARTITION_PARAMETER_3,
    HAL_PARTITION_PARAMETER_4,
    HAL_PARTITION_BT_FIRMWARE,
    HAL_PARTITION_MAX,
    HAL_PARTITION_NONE,
} hal_partition_t;

typedef enum {
    HAL_FLASH_EMBEDDED,
    HAL_FLASH_SPI,
    HAL_FLASH_QSPI,
    HAL_FLASH_MAX,
    HAL_FLASH_NONE,
} hal_flash_t;

typedef struct {
    hal_flash_t partition_owner;
    const char *partition_description;
    uint32_t    partition_start_addr;
    uint32_t    partition_length;
    uint32_t    partition_options;
} hal_logic_partition_t;


/******************************************************
*                 Function Declarations
******************************************************/

/**@brief   Get the infomation of the specified flash area
 *
 * @param   in_partition:  The target flash logical partition which should be erased
 *
 * @return  HAL_logi_partition struct
 */
hal_logic_partition_t *hal_flash_get_info(hal_partition_t in_partition);


/**@brief   Erase an area on a Flash logical partition
 *
 * @note    Erase on an address will erase all data on a sector that the
 *          address is belonged to, this function does not save data that
 *          beyond the address area but in the affected sector, the data
 *          will be lost.
 *
 * @param  in_partition     : The target flash logical partition which should be erased
 * @param  off_set         : Start address of the erased flash area
 * @param  size        : Size of the erased flash area
 *
 * @return  0        : On success.
 * @return  <0   : If an error occurred with any step
 */
int32_t hal_flash_erase(hal_partition_t in_partition, uint32_t off_set,
                        uint32_t size);

/**@brief  Write data to an area on a Flash logical partition
 *
 * @param  in_partition    : The target flash logical partition which should be read which should be written
 * @param  off_set        : Point to the start address that the data is written to, and
 *                          point to the last unwritten address after this function is
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param  inBuffer       : point to the data buffer that will be written to flash
 * @param  inBufferLength : The length of the buffer
 *
 * @return  0        : On success.
 * @return  <0   : If an error occurred with any step
 */
int32_t hal_flash_write(hal_partition_t in_partition, uint32_t *off_set,
                        const void *in_buf , uint32_t in_buf_len);

/**@brief    Read data from an area on a Flash to data buffer in RAM
 *
 * @param    in_partition    : The target flash logical partition which should be read
 * @param    off_set        : Point to the start address that the data is read, and
 *                          point to the last unread address after this function is
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param    outBuffer      : Point to the data buffer that stores the data read from flash
 * @param    inBufferLength : The length of the buffer
 *
 * @return  0        : On success.
 * @return  <0   : If an error occurred with any step
 */
int32_t hal_flash_read(hal_partition_t in_partition, uint32_t *off_set,
                       void *out_buf, uint32_t in_buf_len);



/**@brief    Set security options on a logical partition
 *
 * @param    partition     : The target flash logical partition
 * @param    offset        : Point to the start address that the data is read, and
 *                          point to the last unread address after this function is
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param    size          : Size of enabled flash area
 *
 * @return  0        : On success.
 * @return  <0   : If an error occurred with any step
 */
int32_t hal_flash_enable_secure(hal_partition_t partition, uint32_t off_set,
                                uint32_t size);


/**@brief    Disable security options on a logical partition
 *
 * @param    partition     : The target flash logical partition
 * @param    offset        :  Point to the start address that the data is read, and
 *                          point to the last unread address after this function is
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param    size          : Size of disabled flash area
 *
 * @return  0        : On success.
 * @return  <0   : If an error occurred with any step
 */
int32_t hal_flash_dis_secure(hal_partition_t partition, uint32_t off_set,
                             uint32_t size);

/** @} */
/** @} */

#endif


