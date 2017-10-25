HOST_OPENOCD := esp32

NAME := esp32

$(NAME)_COMPONENTS := framework.common

GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/freertos/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/soc/esp32/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/soc/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/driver/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/heap/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/tcpip_adapter/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/lwip/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/lwip/include/lwip
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/log/include
GLOBAL_CFLAGS    += -I $(IDF_PATH)/components/nvs_flash/include
GLOBAL_INCLUDES  += system/include ../../arch/xtensa
GLOBAL_CFLAGS    += -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls
GLOBAL_LDFLAGS   += -nostdlib -Lplatform/mcu/esp32/ -lc
GLOBAL_LDFLAGS   += -lgcc -lstdc++ -lgcov -lm

GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.ld.S
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.common.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.rom.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.peripherals.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.rom.spiram_incompatible_fns.ld
GLOBAL_LDFLAGS   += -L platform/mcu/esp32/system/ld

$(NAME)_SOURCES  := bsp/entry.c bsp/hal.c #system/cpu_start.c
$(NAME)_INCLUDES := soc/include soc/esp32/include
$(NAME)_CFLAGS   := -std=gnu99
ifeq ($(wifi),1)
$(NAME)_CFLAGS   += -DENABLE_WIFI
endif

libs := $(wildcard platform/mcu/esp32/lib/lib*.a)
libs := $(foreach lib,$(libs),lib/$(notdir $(lib)))
$(NAME)_PREBUILT_LIBRARY := $(libs)
