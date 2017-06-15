#include "hal/soc/soc.h"
#include "mico_rtos.h"

typedef uint32_t UINT32;
#define VOID void
#define ASSERT(exp)

#include "gpio_pub.h"

int32_t hal_gpio_init(uint8_t gpio, hal_gpio_config_t config)
{

}

int32_t hal_gpio_finalize(uint8_t gpio)
{

}

int32_t hal_gpio_output_high(uint8_t gpio)
{

}

int32_t hal_gpio_output_low(uint8_t gpio)
{

}

int32_t hal_gpio_output_trigger(uint8_t gpio)
{

}

int8_t hal_gpio_inputget(uint8_t gpio)
{

}

int32_t hal_gpio_enable_irq(uint8_t gpio, hal_gpio_irq_trigger_t trigger, hal_gpio_irq_handler_t handler, void *arg)
{

}

int32_t hal_gpio_disable_irq(uint8_t gpio)
{

}