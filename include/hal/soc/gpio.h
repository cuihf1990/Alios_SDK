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


#ifndef YOS_GPIO_H
#define YOS_GPIO_H

typedef enum
{
    HAL_GPIO_0,
    HAL_GPIO_1 ,
    HAL_GPIO_2,
    HAL_GPIO_3,
    HAL_GPIO_4,
    HAL_GPIO_5,
    HAL_GPIO_6,
    HAL_GPIO_7,
    HAL_GPIO_8,
    HAL_GPIO_9,
    HAL_GPIO_10,
    HAL_GPIO_11,
    HAL_GPIO_12,
    HAL_GPIO_13,
    HAL_GPIO_14,
    HAL_GPIO_15,
    HAL_GPIO_16,
    HAL_GPIO_17,
    HAL_GPIO_18,
    HAL_GPIO_19,
    HAL_GPIO_20,
    HAL_GPIO_21,
    HAL_GPIO_22,
    HAL_GPIO_23,
    HAL_GPIO_24,
    HAL_GPIO_25,
    HAL_GPIO_26,
    HAL_GPIO_27,
    HAL_GPIO_28,
    HAL_GPIO_29,
    HAL_GPIO_30,
    HAL_GPIO_31,
    HAL_GPIO_32,
    HAL_GPIO_33,
    HAL_GPIO_34,
    HAL_GPIO_35,
    HAL_GPIO_36,
    HAL_GPIO_37,
    HAL_GPIO_38,
    HAL_GPIO_39,
    HAL_GPIO_40,
    HAL_GPIO_41,
    HAL_GPIO_MAX,
} hal_gpio_t;


/**
 * Pin configuration
 */
typedef enum
{
    INPUT_PULL_UP,             /* Input with an internal pull-up resistor - use with devices that actively drive the signal low - e.g. button connected to ground */
    INPUT_PULL_DOWN,           /* Input with an internal pull-down resistor - use with devices that actively drive the signal high - e.g. button connected to a power rail */
    INPUT_HIGH_IMPEDANCE,      /* Input - must always be driven, either actively or by an external pullup resistor */
    OUTPUT_PUSH_PULL,          /* Output actively driven high and actively driven low - must not be connected to other active outputs - e.g. LED output */
    OUTPUT_OPEN_DRAIN_NO_PULL, /* Output actively driven low but is high-impedance when set high - can be connected to other open-drain/open-collector outputs. Needs an external pull-up resistor */
    OUTPUT_OPEN_DRAIN_PULL_UP, /* Output actively driven low and is pulled high with an internal resistor when set high - can be connected to other open-drain/open-collector outputs. */
} hal_gpio_config_t;


/**
 * GPIO interrupt trigger
 */
typedef enum
{
    IRQ_TRIGGER_RISING_EDGE  = 0x1, /* Interrupt triggered at input signal's rising edge  */
    IRQ_TRIGGER_FALLING_EDGE = 0x2, /* Interrupt triggered at input signal's falling edge */
    IRQ_TRIGGER_BOTH_EDGES   = IRQ_TRIGGER_RISING_EDGE | IRQ_TRIGGER_FALLING_EDGE,
} hal_gpio_irq_trigger_t;


 /**
  * GPIO interrupt callback handler
  */
 typedef void (*hal_gpio_irq_handler_t)(void* arg);


 /******************************************************
 *                 Function Declarations
 ******************************************************/


/**@brief Initialises a GPIO pin
 *
 * @note  Prepares a GPIO pin for use.
 *
 * @param gpio          : the gpio pin which should be initialised
 * @param configuration : A structure containing the required
 *                        gpio configuration
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_gpio_init(hal_gpio_t gpio, hal_gpio_config_t config);


/**@brief DeInitialises a GPIO pin
 *
 * @note  Set a GPIO pin in default state.
 *
 * @param  gpio          : the gpio pin which should be deinitialised
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_gpio_finalize(hal_gpio_t gpio);


/**@brief Sets an output GPIO pin high
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param gpio          : the gpio pin which should be set high
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_gpio_output_high(hal_gpio_t gpio);


/**@brief Sets an output GPIO pin low
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param gpio          : the gpio pin which should be set low
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_gpio_output_low(hal_gpio_t gpio);

/** Trigger an output GPIO pin 
 *
 * Trigger an output GPIO pin's output. Using this function on a
 * gpio pin which is set to input mode is undefined.
 *
 * @param gpio          : the gpio pin which should be set low
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_gpio_output_trigger(hal_gpio_t gpio);



/**@brief Get the state of an input GPIO pin
 *
 * @note Get the state of an input GPIO pin. Using this function on a
 * gpio pin which is set to output mode will return an undefined value.
 *
 * @param gpio          : the gpio pin which should be read
 *
 * @return    true  : if high
 * @return    fasle : if low
 */
char hal_gpio_inputget(hal_gpio_t gpio);


/**@brief Enables an interrupt trigger for an input GPIO pin
 *
 * @note Enables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which is set to
 * output mode is undefined.
 *
 * @param gpio    : the gpio pin which will provide the interrupt trigger
 * @param trigger : the type of trigger (rising/falling edge)
 * @param handler : a function pointer to the interrupt handler
 * @param arg     : an argument that will be passed to the
 *                  interrupt handler
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_gpio_enable_irq(hal_gpio_t gpio, hal_gpio_irq_trigger_t trigger, hal_gpio_irq_handler_t handler, void *arg);


/**@brief Disables an interrupt trigger for an input GPIO pin
 *
 * @note Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using @ref hal_gpio_input_irq_enable is undefined.
 *
 * @param gpio    : the gpio pin which provided the interrupt trigger
 *
 * @return    kNoErr        : on success.
 * @return    kGeneralErr   : if an error occurred with any step
 */
int hal_gpio_disable_irq(hal_gpio_t gpio);

/** @} */
/** @} */

#endif


