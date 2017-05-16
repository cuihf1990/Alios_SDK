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

#ifndef __OTA_H__
#define __OTA_H__

#pragma once
fdef __cplusplus
extern "C" {
#endif

typedef struct hal_ota_module_s hal_ota_module_t;

struct hal_ota_module_s
{
    hal_module_base_t base;

    /* Link to HW */
    int (*init)(hal_ota_module_t *m, void *something);
    int (*write)(hal_ota_module_t *m, volatile uint32_t* off_set, uint8_t* in_buf ,uint32_t in_buf_len);
    int (*read)(hal_ota_module_t *m,  volatile uint32_t* off_set, uint8_t* out_buf ,uint32_t out_buf_len);
    int (*set_boot)(hal_ota_module_t *m, void *something);
};
 /******************************************************
 *                 Function Declarations
 ******************************************************/


/**
 * @brief Arch register a new module before HAL startup
 */
void hal_ota_register_module(hal_ota_module_t *module);

/**@brief  init ota partition
 *
 * @note   when ota start ,maybe it need init something
 * @return 0   : On success.
 * @return 1   : If an error occurred with any step
 */
hal_stat_t hal_ota_init();

/**@brief  Write data to an area on ota partition
 *
 * @param  m              :Refer the ota module which will be used,default module will be used if value is NULL
 * @param  off_set        : Point to the start address that the data is written to, and
 *                          point to the last unwritten address after this function is 
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param  inbuf          : point to the data buffer that will be written to flash
 * @param  in_buf_len     : The length of the buffer
 *
 * @return 0         : On success.
 * @return 1    : If an error occurred with any step
 */
hal_stat_t hal_ota_write(hal_ota_module_t *m, volatile uint32_t* off_set, uint8_t* in_buf ,uint32_t in_buf_len);

/**@brief  Read data from an area on ota Flash to data buffer in RAM
 *
 * @param  m              :Refer the ota module which will be used,default module will be used if value is NULL
 * @param  off_set        : Point to the start address that the data is read, and
 *                          point to the last unread address after this function is 
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param  out_buf        : Point to the data buffer that stores the data read from flash
 * @param  in_buf_len     : The length of the buffer
 *
 * @return 0         : On success.
 * @return 1    : If an error occurred with any step
 */
hal_stat_t hal_ota_read(hal_ota_module_t *m, volatile uint32_t* off_set, uint8_t* out_buf, uint32_t in_buf_len);

/**@brief  Set boot options when ota reboot
 *

 * @param  m              :Refer the ota module which will be used,default module will be used if value is NULL
 * @param  something      : boot parms
 * @return kNoErr         : On success.
 * @return kGeneralErr    : If an error occurred with any step
 */
hal_stat_t hal_ota_set_boot(hal_ota_module_t *m, void *something);


/**
 * @brief Get the default ota module
 *
 * @return
 *     return the first registered ota module ,which is the head of module list
 */
hal_ota_module_t *hal_ota_get_default_module(void);

/**
 * Get the next ota HAL
 *
 * The system may have more than 1 ota HAL instances.
 *
 * @return
 *     Instance pointer or NULL
 */
hal_ota_module_t *hal_ota_get_next_module(hal_ota_module_t *m);

#ifdef __cplusplus
}
#endif

#endif


