NAME = stm32l071kb

$(NAME)_INCLUDES := \
Inc \
Drivers/STM32L0xx_HAL_Driver/Inc \
Drivers/STM32L0xx_HAL_Driver/Inc/Legacy \
Drivers/CMSIS/Device/ST/STM32L0xx/Include \
Drivers/CMSIS/Include

$(NAME)_SOURCES := \
Src/main.c \
startup_stm32l071xx.s \
Src/stm32l0xx_hal_msp.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart_ex.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_gpio.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_dma.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_tim_ex.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c_ex.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal.c \
Src/system_stm32l0xx.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ramfunc.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_rcc_ex.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr_ex.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_flash_ex.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_uart.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_i2c.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_pwr.c \
Src/stm32l0xx_it.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_hal_cortex.c  \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_adc.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_spi.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_usart.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_gpio.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_rtc.c \
Drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_rcc.c

$(NAME)_DEFINES := \
USE_HAL_DRIVER \
STM32L071xx

GLOBAL_LDFLAGS += -T platform/mcu/stm32l0xx/stm32l071kb/STM32L071KBUx_FLASH.ld
