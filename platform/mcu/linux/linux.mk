HOST_OPENOCD := linux

NAME := linuximpl
ARCH_LINUX := ../../arch/linux/

GLOBAL_INCLUDES += . $(ARCH_LINUX)

$(NAME)_COMPONENTS  := vflash

$(NAME)_INCLUDES    += .
GLOBAL_INCLUDES     += include include/yos csp/lwip/include
GLOBAL_LDFLAGS      += -lpthread -lm -lcrypto
GLOBAL_DEFINES      += CONFIG_YOS_RHINO_MMREGION
GLOBAL_DEFINES      += CONFIG_YSH_CMD_DUMPSYS
GLOBAL_DEFINES      += CONFIG_YOS_KVFILE=\"/dev/flash0\"
GLOBAL_DEFINES      += CONFIG_YOS_KVFILE_BACKUP=\"/dev/flash1\"
GLOBAL_CFLAGS       += -Wall -Wno-missing-field-initializers -Wno-strict-aliasing -Wno-address -Wno-unused-result
GLOBAL_DEFINES      += CSP_LINUXHOST

# arch linux
$(NAME)_SOURCES := $(ARCH_LINUX)/cpu_impl.c
# mcu
$(NAME)_SOURCES     += main/arg_options.c
$(NAME)_SOURCES     += main/main.c
$(NAME)_SOURCES     += main/hw.c
$(NAME)_SOURCES     += main/wifi_port.c
$(NAME)_SOURCES     += main/ota_port.c
$(NAME)_SOURCES     += csp/csp_rhino.c
$(NAME)_SOURCES     += soc/soc_impl.c
$(NAME)_SOURCES     += soc/hook_impl.c
$(NAME)_SOURCES     += soc/ysh_impl.c
$(NAME)_SOURCES     += soc/trace_impl.c
$(NAME)_SOURCES     += soc/trace_hal.c
$(NAME)_SOURCES     += soc/fifo.c

$(info $(COMPONENTS))
ifneq (,$(filter protocols.net,$(COMPONENTS)))
$(NAME)_SOURCES     += \
    csp/lwip/netif/delif.c \
    csp/lwip/netif/fifo.c \
    csp/lwip/netif/list.c \
    csp/lwip/netif/tapif.c \
    csp/lwip/netif/tcpdump.c \
    csp/lwip/netif/tunif.c

$(NAME)_SOURCES     += csp/lwip/lwip_linuxhost.c
endif
