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


#ifndef YOS_ADC_H
#define YOS_ADC_H

#pragma once
#include "common.h"
#include "board_platform.h"


/**@biref Initialises an ADC interface
 *
 * Prepares an ADC hardware interface for sampling
 *
 * @param adc            : the interface which should be initialised
 * @param sampling_cycle : sampling period in number of ADC clock cycles. If the
 *                         MCU does not support the value provided, the closest
 *                         supported value is used.
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_adc_init(hal_adc_t adc, uint32_t sampling_cycle);


/**@biref Takes a single sample from an ADC interface
 *
 * Takes a single sample from an ADC interface
 *
 * @param adc    : the interface which should be sampled
 * @param output : pointer to a variable which will receive the sample
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_adc_task_sample(hal_adc_t adc, uint16_t *output );


/**@biref Takes multiple samples from an ADC interface
 *
 * Takes multiple samples from an ADC interface and stores them in
 * a memory buffer
 *
 * @param adc           : the interface which should be sampled
 * @param buffer        : a memory buffer which will receive the samples
 *                        Each sample will be uint16_t little endian.
 * @param buffer_length : length in bytes of the memory buffer.
 *
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_adc_take_sample_streram(hal_adc_t adc, void *buf, uint16_t buf_len);


/**@biref     De-initialises an ADC interface
 *
 * @abstract Turns off an ADC hardware interface
 *
 * @param  adc : the interface which should be de-initialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int  hal_adc_finalize(hal_adc_t adc);

/** @} */
/** @} */
#endif
