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

GLOBAL_DEFINES   += CONFIG_AOS_KV_BUFFER_SIZE=8192

$(NAME)_SOURCES  := bsp/entry.c bsp/hal.c hal/flash.c #system/cpu_start.c
$(NAME)_SOURCES  += hal/wifi_port.c
$(NAME)_INCLUDES := soc/include soc/esp32/include
$(NAME)_CFLAGS   := -std=gnu99
ifeq ($(wifi),1)
$(NAME)_CFLAGS   += -DENABLE_WIFI
endif

ifeq (0,1)
libs := $(wildcard platform/mcu/esp32/lib/*.a)
libs := $(foreach lib,$(libs),lib/$(notdir $(lib)))
$(NAME)_PREBUILT_LIBRARY := $(libs)
endif

$(NAME)_PREBUILT_LIBRARY := lib/libesp32.a
$(NAME)_PREBUILT_LIBRARY += lib/libsoc.a
$(NAME)_PREBUILT_LIBRARY += lib/libhal.a
$(NAME)_PREBUILT_LIBRARY += lib/libnewlib.a
$(NAME)_PREBUILT_LIBRARY += lib/libvfs.a
$(NAME)_PREBUILT_LIBRARY += lib/libspi_flash.a
$(NAME)_PREBUILT_LIBRARY += lib/liblog.a
$(NAME)_PREBUILT_LIBRARY += lib/libdriver.a
$(NAME)_PREBUILT_LIBRARY += lib/libcontainer.a
$(NAME)_PREBUILT_LIBRARY += lib/librtc.a

$(NAME)_PREBUILT_LIBRARY += lib/libcoexist.a
$(NAME)_PREBUILT_LIBRARY += lib/libcore.a
$(NAME)_PREBUILT_LIBRARY += lib/libnet80211.a
$(NAME)_PREBUILT_LIBRARY += lib/libpp.a
$(NAME)_PREBUILT_LIBRARY += lib/libwpa.a
$(NAME)_PREBUILT_LIBRARY += lib/libwpa2.a
$(NAME)_PREBUILT_LIBRARY += lib/libwps.a
$(NAME)_PREBUILT_LIBRARY += lib/libphy.a
$(NAME)_PREBUILT_LIBRARY += lib/libnvs_flash.a
$(NAME)_PREBUILT_LIBRARY += lib/libcxx.a
$(NAME)_PREBUILT_LIBRARY += lib/liblwip.a
$(NAME)_PREBUILT_LIBRARY += lib/libstdcc++-cache-workaround.a
$(NAME)_PREBUILT_LIBRARY += lib/libtcpip_adapter.a
$(NAME)_PREBUILT_LIBRARY += lib/libwpa_supplicant.a

ifeq ($(vcall),freertos)
$(NAME)_PREBUILT_LIBRARY += lib/libespos.a
$(NAME)_PREBUILT_LIBRARY += lib/libfreertos.a
$(NAME)_PREBUILT_LIBRARY += lib/libheap.a
else
$(NAME)_COMPONENTS       += rhino platform/arch/xtensa
$(NAME)_SOURCES          += aos/hook_impl.c
$(NAME)_SOURCES          += aos/soc_impl.c
$(NAME)_SOURCES          += aos/trace_impl.c
$(NAME)_SOURCES          += aos/heap_wrapper.c
endif
