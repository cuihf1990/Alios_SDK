HOST_OPENOCD := esp32

NAME := esp32

$(NAME)_COMPONENTS += rhino protocols.net

GLOBAL_INCLUDES  += system/include ../../arch/xtensa
GLOBAL_CFLAGS    += -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls
GLOBAL_LDFLAGS   += -nostdlib

GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.ld.S
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.common.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.rom.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.peripherals.ld
GLOBAL_LDS_FILES += platform/mcu/esp32/system/ld/esp32.rom.spiram_incompatible_fns.ld
GLOBAL_LDFLAGS   += -L platform/mcu/esp32/system/ld

$(NAME)_SOURCES  := system/cpu_start.c
$(NAME)_INCLUDES := soc/include soc/esp32/include
$(NAME)_CFLAGS   := -std=gnu99
