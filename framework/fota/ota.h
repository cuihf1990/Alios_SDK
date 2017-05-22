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

#ifndef OTA_H_
#define OTA_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief enable sensor with type
 *
 * @m[in] Refer the sensor module which will be used,default module will be used if value is NULL
 * @type[in] The type of the sensor which must be supplied
 *
 * @return
 *    return 0 if enable ok, negative value indicates an error
 */

/**
 * HAL ota init .
 *
 * @return
 *     return result, 0 if init success, -1 if fail
 */
int  hal_ota_init();


/**@brief  Write ota data to an area on a Flash logical partition
 *
 * @param  off_set        : Point to the start address that the data is written to, and
 *                          point to the last unwritten address after this function is
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param  in_buf       : point to the data buffer that will be written to flash
 * @param  in_buf_len : The length of the buffer
 *
 * @return  0        : On success.
 * @return  1   : If an error occurred with any step
 */
int hal_ota_write(volatile uint32_t* off_set, uint8_t* in_buf ,uint32_t in_buf_len);

/**@brief    Read ota data from an area on a Flash to data buffer in RAM
 *
 * @param    off_set        : Point to the start address that the data is read, and
 *                          point to the last unread address after this function is
 *                          returned, so you can call this function serval times without
 *                          update this start address.
 * @param    out_buf      : Point to the data buffer that stores the data read from flash
 * @param    out_buf_len : The length of the buffer
 *
 * @return    0        : On success.
 * @return    1   : If an error occurred with any step
 */
int hal_ota_read(volatile uint32_t* off_set, uint8_t* out_buf ,uint32_t out_buf_len);

/**@brief    set boot parms after downlowd ota file
 *
 *
 * @return    0        : On success.
 * @return    1   : If an error occurred with any step
 */
int hal_ota_set_boot();

#ifdef __cplusplus
}
#endif


#endif /* OTA_H_ */
