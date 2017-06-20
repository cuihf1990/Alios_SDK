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

NAME := board_mk3060

JTAG := jlink

MODULE               := EMW3060
HOST_ARCH            := ARM968E-S
HOST_MCU_FAMILY      := beken

$(NAME)_SOURCES := board.c

GLOBAL_INCLUDES += .

CURRENT_TIME = $(shell date +%Y%m%d.%H%M)
define get-os-version
"YOS-R"-$(CURRENT_TIME)
endef

CONFIG_SYSINFO_OS_VERSION := $(call get-os-version)
CONFIG_SYSINFO_PRODUCT_MODEL := ALI_YOS_MK3060
CONFIG_SYSINFO_DEVICE_NAME := MK3060

GLOBAL_CFLAGS += -DSYSINFO_OS_VERSION=\"$(CONFIG_SYSINFO_OS_VERSION)\"
GLOBAL_CFLAGS += -DSYSINFO_PRODUCT_MODEL=\"$(CONFIG_SYSINFO_PRODUCT_MODEL)\"
GLOBAL_CFLAGS += -DSYSINFO_DEVICE_NAME=\"$(CONFIG_SYSINFO_DEVICE_NAME)\"

GLOBAL_LDFLAGS  += -L $(SOURCE_ROOT)/board/mk3060

# Extra build target in mico_standard_targets.mk, include bootloader, and copy output file to eclipse debug file (copy_output_for_eclipse)
EXTRA_TARGET_MAKEFILES +=  $(MAKEFILES_PATH)/yos_standard_targets.mk
EXTRA_TARGET_MAKEFILES +=  $(SOURCE_ROOT)/platform/mcu/$(HOST_MCU_FAMILY)/gen_crc_bin.mk
