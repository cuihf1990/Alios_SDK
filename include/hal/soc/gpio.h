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
 * @file hal/soc/gpio.h
 * @brief GPIO HAL
 * @version since 5.5.0
 */

#ifndef YOS_GPIO_H
#define YOS_GPIO_H

/**
 * Pin configuration
 */
typedef enum {
    INPUT_PULL_UP,             /* Input with an internal pull-up resistor - use with devices that actively drive the signal low - e.g. button connected to ground */
    INPUT_PULL_DOWN,           /* Input with an internal pull-down resistor - use with devices that actively drive the signal high - e.g. button connected to a power rail */
    INPUT_HIGH_IMPEDANCE,      /* Input - must always be driven, either actively or by an external pullup resistor */
    OUTPUT_PUSH_PULL,          /* Output actively driven high and actively driven low - must not be connected to other active outputs - e.g. LED output */
    OUTPUT_OPEN_DRAIN_NO_PULL, /* Output actively driven low but is high-impedance when set high - can be connected to other open-drain/open-collector outputs. Needs an external pull-up resistor */
    OUTPUT_OPEN_DRAIN_PULL_UP, /* Output actively driven low and is pulled high with an internal resistor when set high - can be connected to other open-drain/open-collector outputs. */
} gpio_config_t;


/**
 * GPIO interrupt trigger
 */
typedef enum {
    IRQ_TRIGGER_RISING_EDGE  = 0x1, /* Interrupt triggered at input signal's rising edge  */
    IRQ_TRIGGER_FALLING_EDGE = 0x2, /* Interrupt triggered at input signal's falling edge */
    IRQ_TRIGGER_BOTH_EDGES   = IRQ_TRIGGER_RISING_EDGE | IRQ_TRIGGER_FALLING_EDGE,
} gpio_irq_trigger_t;


typedef struct {
    uint8_t       port;    /* gpio port */
    gpio_config_t config;  /* gpio config */
    void         *priv;    /* priv data */
} gpio_dev_t;


/**
 * GPIO interrupt callback handler
 */
typedef void (*gpio_irq_handler_t)(void *arg);


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
 * @return    0         : on success.
 * @return    EIO       : if an error occurred with any step
 */
int32_t hal_gpio_init(gpio_dev_t *gpio);

/**@brief Sets an output GPIO pin high
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param gpio    : the gpio pin which should be set high
 *
 * @return    0   : on success.
 * @return    EIO : if an error occurred with any step
 */
int32_t hal_gpio_output_high(gpio_dev_t *gpio);


/**@brief Sets an output GPIO pin low
 *
 * @note  Using this function on a gpio pin which is set to input mode is undefined.
 *
 * @param gpio      : the gpio pin which should be set low
 *
 * @return    0     : on success.
 * @return    EIO   : if an error occurred with any step
 */
int32_t hal_gpio_output_low(gpio_dev_t *gpio);

/** Trigger an output GPIO pin
 *
 * Trigger an output GPIO pin's output. Using this function on a
 * gpio pin which is set to input mode is undefined.
 *
 * @param gpio      : the gpio pin which should be set low
 *
 * @return    0     : on success.
 * @return    EIO   : if an error occurred with any step
 */
int32_t hal_gpio_output_toggle(gpio_dev_t *gpio);



/**@brief Get the state of an input GPIO pin
 *
 * @note Get the state of an input GPIO pin. Using this function on a
 * gpio pin which is set to output mode will return an undefined value.
 *
 * @param gpio     : the gpio pin which should be read
 * @param value    : gpio value
 * @return    0    : on success
 * @return    EIO  : if an error occurred with any step
 */
int8_t hal_gpio_input_get(gpio_dev_t *gpio, uint32_t *value);


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
 * @return    0   : on success.
 * @return    EIO : if an error occurred with any step
 */
int32_t hal_gpio_enable_irq(gpio_dev_t *gpio, gpio_irq_trigger_t trigger,
                            gpio_irq_handler_t handler, void *arg);


/**@brief Disables an interrupt trigger for an input GPIO pin
 *
 * @note Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using @ref hal_gpio_input_irq_enable is undefined.
 *
 * @param gpio      : the gpio pin which provided the interrupt trigger
 *
 * @return    0     : on success.
 * @return    EIO   : if an error occurred with any step
 */
int32_t hal_gpio_disable_irq(gpio_dev_t *gpio);


/**@brief Disables an interrupt trigger for an input GPIO pin
 *
 * @note Disables an interrupt trigger for an input GPIO pin.
 * Using this function on a gpio pin which has not been set up
 * using @ref hal_gpio_input_irq_enable is undefined.
 *
 * @param gpio     : the gpio pin which provided the interrupt trigger
 *
 * @return    0    : on success.
 * @return    EIO  : if an error occurred with any step
 */
int32_t hal_gpio_clear_irq(gpio_dev_t *gpio);


/**@brief DeInitialises a GPIO pin
 *
 * @note  Set a GPIO pin in default state.
 *
 * @param  gpio     : the gpio pin which should be deinitialised
 *
 * @return    0     : on success.
 * @return    EIO   : if an error occurred with any step
 */
int32_t hal_gpio_finalize(gpio_dev_t *gpio);

#endif


