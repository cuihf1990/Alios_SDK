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


NAME := Lib_lib_lorawan

$(NAME)_SOURCES := Lora/Crypto/aes.c                \
				   Lora/Crypto/cmac.c               \
				   Lora/Utilities/timeServer.c      \
				   Lora/Utilities/low_power.c       \
				   Lora/Utilities/utilities.c       \
				   Lora/Utilities/delay.c           \
                   Lora/Mac/region/Region.c         \
                   Lora/Mac/region/RegionAS923.c    \
                   Lora/Mac/region/RegionAU915.c    \
                   Lora/Mac/region/RegionCN470.c    \
                   Lora/Mac/region/RegionCN779.c    \
                   Lora/Mac/region/RegionCommon.c   \
                   Lora/Mac/region/RegionEU433.c    \
                   Lora/Mac/region/RegionEU868.c    \
                   Lora/Mac/region/RegionIN865.c    \
                   Lora/Mac/region/RegionKR920.c    \
                   Lora/Mac/region/RegionUS915.c    \
                   Lora/Mac/region/RegionUS915-Hybrid.c \
                   Lora/Mac/LoRaMaC.c               \
                   Lora/Mac/LoRaMacCrypto.c    \
                   BSP/EML3047/eml3047.c    \
				   BSP/sx1276/sx1276.c              \

GLOBAL_INCLUDES +=  .            \
				    BSP/EML3047  \
				    BSP/sx1276       \
				    Lora/Crypto      \
					Lora/Phy         \
					Lora/Mac         \
					Lora/Core        \
					Lora/Mac/region  \
					Lora/Utilities

$(NAME)_INCLUDES := \
../app_lora_demo/inc \
../../platform/mcu/stm32l0xx/stm32l071kb/Drivers/STM32L0xx_HAL_Driver/Inc \
../../platform/mcu/stm32l0xx/stm32l071kb/Drivers/STM32L0xx_HAL_Driver/Inc/Legacy \
../../platform/mcu/stm32l0xx/stm32l071kb/Drivers/CMSIS/Device/ST/STM32L0xx/Include \
../../platform/mcu/stm32l0xx/stm32l071kb/Drivers/CMSIS/Include

$(NAME)_DEFINES := \
USE_HAL_DRIVER \
STM32L071xx