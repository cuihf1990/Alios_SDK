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

/**
 * @file hal/soc/adc.h
 * @brief adc HAL
 * @version since 5.5.0
 */

#ifndef YOS_ADC_H
#define YOS_ADC_H

typedef struct
{
    uint32_t sampling_cycle; /* sampling period in number of ADC clock cycles */
} adc_config_t;

typedef struct
{
    uint8_t      adc;            /* the interface which should be initialised */
    adc_config_t config;         /* adc config */
    void        *priv;           /* priv data */
} adc_dev_t;

/**@biref Initialises an ADC interface
 *
 * Prepares an ADC hardware interface for sampling
 *
 * @param     adc           : the interface which should be initialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int32_t hal_adc_init(adc_dev_t *adc);


/**@biref Takes a single sample from an ADC interface
 *
 * Takes a single sample from an ADC interface
 *
 * @param  adc           : the interface which should be sampled
 * @param  output        : pointer to a variable which will receive the sample
 * @param  timeout       : ms timeout
 * @return kNoErr        : on success.
 * @return kGeneralErr   : if an error occurred with any step
 */
int32_t hal_adc_value_get(adc_dev_t *adc, void *output, uint32_t timeout);


/**@biref     De-initialises an ADC interface
 *
 * @abstract Turns off an ADC hardware interface
 *
 * @param  adc : the interface which should be de-initialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int32_t hal_adc_finalize(adc_dev_t *adc);

/** @} */
/** @} */
#endif
