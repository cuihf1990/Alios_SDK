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
 * @file hal/soc/pwm.h
 * @brief PWM HAL
 * @version since 5.5.0
 */

#ifndef YOS_PWM_H
#define YOS_PWM_H

typedef struct {
    float    duty_cycle;  /* the pwm duty_cycle */
    uint32_t freq;        /* the pwm freq */
} pwm_config_t;

typedef struct {
    uint8_t      port;    /* pwm port */
    pwm_config_t config;  /* spi config */
    void        *priv;    /* priv data */
} pwm_dev_t;

/**@brief Initialises a PWM pin
 *
 * @note  Prepares a Pulse-Width Modulation pin for use.
 * Does not start the PWM output (use @ref MicoPwmStart).
 *
 * @param     pwm         : the PWM device
 * @return    kNoErr      : on success.
 * @return    kGeneralErr : if an error occurred with any step
 */
int32_t hal_pwm_init(pwm_dev_t *pwm);


/**@brief Starts PWM output on a PWM interface
 *
 * @note  Starts Pulse-Width Modulation signal output on a PWM pin
 *
 * @param     pwm         : the PWM device
 * @return    kNoErr      : on success.
 * @return    kGeneralErr : if an error occurred with any step
 */
int32_t hal_pwm_start(pwm_dev_t *pwm);


/**@brief Stops output on a PWM pin
 *
 * @note  Stops Pulse-Width Modulation signal output on a PWM pin
 *
 * @param     pwm         : the PWM device
 * @return    kNoErr      : on success.
 * @return    kGeneralErr : if an error occurred with any step
 */
int32_t hal_pwm_stop(pwm_dev_t *pwm);

#endif
