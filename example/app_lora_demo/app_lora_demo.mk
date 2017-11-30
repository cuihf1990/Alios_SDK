############################################################################### 
#
#  The MIT License
#  Copyright (c) 2016 MXCHIP Inc.
#
#  Permission is hereby granted, free of charge, to any person obtaining a copy 
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights 
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is furnished
#  to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
#  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
############################################################################### 


NAME := App_LoRa_demo

GLOBAL_INCLUDES +=  .            \
				    inc     \

GLOBAL_DEFINES          += USE_FULL_LL_DRIVER
GLOBAL_DEFINES          += USE_B_EML3047
GLOBAL_DEFINES          += REGION_CN470


$(NAME)_SOURCES := src/main.c              \
				   src/debug.c             \
				   src/hw_gpio.c           \
				   src/hw_spi.c            \
				   src/hw_rtc.c            \
				   src/lora.c              \
				   src/eml3047_hw.c      \
				   src/eml3047_it.c      \
				   src/vcom.c              \
				     
#$(NAME)_LINK_FILES := src/eml3047_it.o

$(NAME)_COMPONENTS := lib_lorawan \

$(NAME)_INCLUDES := \
../../platform/mcu/stm32l0xx/stm32l071kb/Drivers/STM32L0xx_HAL_Driver/Inc \
../../platform/mcu/stm32l0xx/stm32l071kb/Drivers/STM32L0xx_HAL_Driver/Inc/Legacy \
../../platform/mcu/stm32l0xx/stm32l071kb/Drivers/CMSIS/Device/ST/STM32L0xx/Include \
../../platform/mcu/stm32l0xx/stm32l071kb/Drivers/CMSIS/Include

$(NAME)_DEFINES := \
USE_HAL_DRIVER \
STM32L071xx
		