/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

/**
 * @file hal/soc/adc.h
 * @brief adc HAL
 * @version since 5.5.0
 */

#ifndef YOS_ADC_H
#define YOS_ADC_H

typedef struct {
    uint32_t sampling_cycle; /* sampling period in number of ADC clock cycles */
} adc_config_t;

typedef struct {
    uint8_t      adc;            /* the interface which should be initialised */
    adc_config_t config;         /* adc config */
    void        *priv;           /* priv data */
} adc_dev_t;

/**@biref Initialises an ADC interface
 *
 * Prepares an ADC hardware interface for sampling
 *
 * @param     adc   : the interface which should be initialised
 *
 * @return    0     : on success.
 * @return    EIO   : if an error occurred with any step
 */
int32_t hal_adc_init(adc_dev_t *adc);


/**@biref Takes a single sample from an ADC interface
 *
 * Takes a single sample from an ADC interface
 *
 * @param  adc      : the interface which should be sampled
 * @param  output   : pointer to a variable which will receive the sample
 * @param  timeout  : ms timeout
 * @return 0        : on success.
 * @return EIO      : if an error occurred with any step
 */
int32_t hal_adc_value_get(adc_dev_t *adc, void *output, uint32_t timeout);


/**@biref     De-initialises an ADC interface
 *
 * @abstract Turns off an ADC hardware interface
 *
 * @param  adc : the interface which should be de-initialised
 *
 * @return 0   : on success.
 * @return EIO : if an error occurred with any step
 */
int32_t hal_adc_finalize(adc_dev_t *adc);

/** @} */
/** @} */
#endif
