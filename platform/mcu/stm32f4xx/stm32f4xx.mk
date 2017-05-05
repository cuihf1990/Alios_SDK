#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := STM32F4xx

HOST_OPENOCD := stm32f4x

$(NAME)_COMPONENTS += platform/arch/arm/armv7m

GLOBAL_INCLUDES += include \
                   init    \
                   driver  \
                   aos

GLOBAL_CFLAGS += -mcpu=cortex-m4     \
                 -mthumb             \
                 -mfloat-abi=soft

GLOBAL_CFLAGS += -DSTM32F429xx         \
                 -DCONFIG_YOC_UT_RHINO \
                 -DHAL_UART_MODULE_ENABLED

GLOBAL_LDFLAGS += -Wl,--no-whole-archive \
                  -mcpu=cortex-m4        \
                  -mthumb                \
                  -mfloat-abi=soft       \
                  -lc

GLOBAL_LDFLAGS += -Tplatform/mcu/stm32f4xx/STM32F429ZI_FLASH.ld

$(NAME)_SOURCES := startup/startup_stm32f429xx.S \
                   cmsis/system_stm32f4xx.c      \
                   driver/stm32f4xx_hal_adc.c    \
                   driver/stm32f4xx_hal_adc_ex.c \
                   driver/stm32f4xx_hal_rcc.c    \
                   driver/stm32f4xx_hal_uart.c   \
                   driver/stm32f4xx_hal.c        \
                   driver/stm32f4xx_hal_usart.c  \
                   driver/stm32f4xx_hal_gpio.c   \
                   driver/stm32f4xx_hal_dma.c    \
                   driver/stm32f4xx_hal_pwr_ex.c \
                   driver/stm32f4xx_nucleo_144.c \
                   driver/stm32f4xx_hal_cortex.c \
                   init/main.c                   \
                   init/stm32f4xx_hal_msp.c      \
                   init/stm32f4xx_it.c           \
                   aos/newlib_stub.c             \
                   aos/soc_impl.c                \
                   aos/soc_entry.c

