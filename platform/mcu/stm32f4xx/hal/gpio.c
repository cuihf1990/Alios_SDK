#include <hal/soc/soc.h>
#include "platform_mcu_peripheral.h"
#include <aos/aos.h>

extern const platform_gpio_t platform_gpio_pins[];

int32_t hal_gpio_init(gpio_dev_t *gpio)
{
    if (!gpio)
        return -1;
    return platform_gpio_init(&platform_gpio_pins[gpio->port], gpio->config);
}

int32_t hal_gpio_deinit(gpio_dev_t *gpio)
{
    if (!gpio)
        return -1;
    return platform_gpio_deinit(&platform_gpio_pins[gpio->port], gpio->config);
}

int32_t hal_gpio_output_high(gpio_dev_t *gpio)
{
    if (!gpio)
        return -1;
    return platform_gpio_output_high(&platform_gpio_pins[gpio->port]);
}

int32_t hal_gpio_output_low(gpio_dev_t *gpio)
{
    if (!gpio)
        return -1;
    return platform_gpio_output_low(&platform_gpio_pins[gpio->port]);
}

int32_t hal_gpio_output_toggle(gpio_dev_t *gpio)
{
    if( !gpio)
        return -1;
    return platform_gpio_output_trigger(&platform_gpio_pins[gpio->port]);
}

int32_t hal_gpio_input_get(gpio_dev_t *gpio, uint32_t *value)
{
    if (!gpio)
        return -1;
    *value = platform_gpio_input_get(&platform_gpio_pins[gpio->port]);
    return 0;

}

int32_t hal_gpio_enable_irq(gpio_dev_t *gpio, gpio_irq_trigger_t trigger, gpio_irq_handler_t handler, void *arg)
{
    if(!gpio)
        return -1;
    return platform_gpio_irq_enable(&platform_gpio_pins[gpio->port], trigger, handler, arg);
}

int32_t hal_gpio_clear_irq(gpio_dev_t *gpio)
{
    if (!gpio)
        return -1;
    return platform_gpio_irq_disable(&platform_gpio_pins[gpio->port]);
}
