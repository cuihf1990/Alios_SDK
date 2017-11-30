#
#  UNPUBLISHED PROPRIETARY SOURCE CODE
#  Copyright (c) 2016 MXCHIP Inc.
#
#  The contents of this file may not be disclosed to third parties, copied or
#  duplicated in any form, in whole or in part, without the prior written
#  permission of MXCHIP Corporation.
#

NAME := STM32L0xx

HOST_OPENOCD := stm32l0x

SPI_WIFI_ENABLED := true

$(NAME)_TYPE := kernel

$(NAME)_COMPONENTS += platform/arch/arm/armv6m
# $(NAME)_COMPONENTS += libc rhino hal netmgr framework.common mbedtls cjson cli digest_algorithm
$(NAME)_COMPONENTS += rhino

GLOBAL_DEFINES += STM32_USE_SPI_WIFI

GLOBAL_DEFINES += CONFIG_AOS_KV_MULTIPTN_MODE
GLOBAL_DEFINES += CONFIG_AOS_KV_PTN=6
GLOBAL_DEFINES += CONFIG_AOS_KV_SECOND_PTN=7
GLOBAL_DEFINES += CONFIG_AOS_KV_PTN_SIZE=4096
GLOBAL_DEFINES += CONFIG_AOS_KV_BUFFER_SIZE=8192

GLOBAL_INCLUDES += ../../arch/arm/armv6m/gcc/m0plus
GLOBAL_INCLUDES += include \
                   startup    \
                   driver

GLOBAL_CFLAGS += -mcpu=cortex-m0plus \
                 -march=armv6-m \
                 -mthumb -mthumb-interwork \
                 -mlittle-endian

GLOBAL_CFLAGS += -w

GLOBAL_LDFLAGS += -mcpu=cortex-m0plus        \
                  -mthumb -mthumb-interwork \
                  -mlittle-endian \
                  -nostartfiles \
                  --specs=nosys.specs \
                  $(CLIB_LDFLAGS_NANO_FLOAT)

$(NAME)_CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-parameter -Wno-implicit-function-declaration
$(NAME)_CFLAGS  += -Wno-type-limits -Wno-sign-compare -Wno-pointer-sign -Wno-uninitialized
$(NAME)_CFLAGS  += -Wno-return-type -Wno-unused-function -Wno-unused-but-set-variable
$(NAME)_CFLAGS  += -Wno-unused-value -Wno-strict-aliasing

$(NAME)_SOURCES := aos/soc_impl.c

$(NAME)_COMPONENTS += platform/mcu/stm32l0xx/stm32l071kb