NAME := stm32l4xx
HOST_OPENOCD := stm32l433
$(NAME)_TYPE := kernel

$(NAME)_COMPONENTS += platform/arch/arm/armv7m
$(NAME)_COMPONENTS += libc rhino hal vfs digest_algorithm

GLOBAL_DEFINES += CONFIG_AOS_KV_MULTIPTN_MODE
GLOBAL_DEFINES += CONFIG_AOS_KV_PTN=6
GLOBAL_DEFINES += CONFIG_AOS_KV_SECOND_PTN=7
GLOBAL_DEFINES += CONFIG_AOS_KV_PTN_SIZE=4096
GLOBAL_DEFINES += CONFIG_AOS_KV_BUFFER_SIZE=8192

GLOBAL_INCLUDES += \
                   src/common/csp/lwip/include \
                   src/common/csp/wifi/inc     \
                   src/include   \
                   Drivers/STM32L4xx_HAL_Driver/Inc \
                   Drivers/STM32L4xx_HAL_Driver/Inc/Legacy \
                   Drivers/BSP/Components/es_wifi \
                   Drivers/BSP/Components/hts221 \
                   Drivers/BSP/Components/lis3mdl \
                   Drivers/BSP/Components/lps22hb \
                   Drivers/BSP/Components/lsm6dsl \
                   Drivers/BSP/Components/vl53l0x \
                   Drivers/CMSIS/Include \
                   Drivers/CMSIS/Device/ST/STM32L4xx/Include \
                   src/STM32L433RC-Nucleo/helloworld
                   
$(NAME)_SOURCES := Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c  \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c  \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c    \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c    \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_qspi.c   \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rng.c    \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c    \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c    \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c    \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c   \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c  \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c   \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c    \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c \
                   Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c

$(NAME)_SOURCES += aos/soc_impl.c \
                   aos/trace_impl.c
                   

GLOBAL_CFLAGS += -DSTM32L433xx
                   
ifeq ($(COMPILER), armcc)
$(NAME)_SOURCES += src/STM32L433RC-Nucleo/startup_stm32l433xx_keil.s
     
else ifeq ($(COMPILER), iar)
$(NAME)_SOURCES += src/STM32L433RC-Nucleo/startup_stm32l433xx_iar.s

else
$(NAME)_SOURCES += src/STM32L433RC-Nucleo/startup_stm32l433xx.s

endif
     
ifeq ($(HOST_MCU_NAME), STM32L433RC-Nucleo)
$(NAME)_SOURCES += src/STM32L433RC-Nucleo/helloworld/soc_init.c \
                   src/STM32L433RC-Nucleo/helloworld/stm32l4xx_hal_msp.c \
                   src/STM32L433RC-Nucleo/helloworld/stm32l4xx_it.c \
                   src/STM32L433RC-Nucleo/helloworld/system_stm32l4xx.c \
                   src/STM32L433RC-Nucleo/helloworld/aos.c  \
                   src/STM32L433RC-Nucleo/hal/hal_uart_stm32l4.c \
                   src/STM32L433RC-Nucleo/hal/hw.c
endif

ifeq ($(COMPILER),armcc)
GLOBAL_CFLAGS   += --c99 --cpu=7E-M -D__MICROLIB -g --apcs=interwork --split_sections
else ifeq ($(COMPILER),iar)
GLOBAL_CFLAGS += --cpu=Cortex-M4 \
                 --cpu_mode=thumb \
                 --endian=little
else
GLOBAL_CFLAGS += -mcpu=cortex-m4 \
                 -march=armv7-m  \
                 -mlittle-endian \
                 -mthumb -mthumb-interwork \
                 -w
endif

ifeq ($(COMPILER),armcc)
GLOBAL_ASMFLAGS += --cpu=7E-M -g --apcs=interwork --library_type=microlib --pd "__MICROLIB SETA 1"
else ifeq ($(COMPILER),iar)
GLOBAL_ASMFLAGS += --cpu Cortex-M4 \
                   --cpu_mode thumb \
                   --endian little
else
GLOBAL_ASMFLAGS += -mcpu=cortex-m4 \
                   -march=armv7-m  \
                   -mlittle-endian \
                   -mthumb -mthumb-interwork \
                   -w
endif

ifeq ($(COMPILER),armcc)
GLOBAL_LDFLAGS += -L --cpu=7E-M   \
                  -L --strict \
                  -L --xref -L --callgraph -L --symbols \
                  -L --info=sizes -L --info=totals -L --info=unused -L --info=veneers -L --info=summarysizes
else ifeq ($(COMPILER),iar)
GLOBAL_LDFLAGS += --silent --cpu=Cortex-M4.vfp

else
GLOBAL_LDFLAGS += -mcpu=cortex-m4  \
                  -mlittle-endian  \
                  -mthumb -mthumb-interwork \
                  --specs=nosys.specs \
                  $(CLIB_LDFLAGS_NANO_FLOAT)
endif

ifeq ($(COMPILER),armcc)
GLOBAL_LDFLAGS += -L --scatter=platform/mcu/stm32l475/B-L475E-IOT01.sct
else ifeq ($(COMPILER),iar)
GLOBAL_LDFLAGS += --config platform/mcu/stm32l475/STM32L475.icf
else
GLOBAL_LDFLAGS += -T platform/mcu/stm32l475/STM32L475VGTx_FLASH.ld
endif


ifeq ($(COMPILER),armcc)
$(NAME)_LINK_FILES := src/STM32L433RC-Nucleo/startup_stm32l433xx_keil.o
$(NAME)_LINK_FILES += src/STM32L433RC-Nucleo/helloworld/stm32l4xx_it.o
endif
