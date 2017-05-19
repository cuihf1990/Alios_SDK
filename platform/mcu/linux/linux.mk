HOST_OPENOCD := linux

NAME := linuximpl
ARCH_LINUX := ../../arch/linux/

GLOBAL_INCLUDES += . $(ARCH_LINUX)

$(NAME)_INCLUDES    += .
GLOBAL_INCLUDES     += include csp/lwip/include
GLOBAL_LDFLAGS      += -lpthread -lm -lcrypto
GLOBAL_DEFINES      += CONFIG_YOS_RHINO_MMREGION
GLOBAL_DEFINES      += CONFIG_YSH_CMD_DUMPSYS
GLOBAL_DEFINES      += CONFIG_YOS_KVFILE=\"/tmp/kv\"
GLOBAL_DEFINES      += CONFIG_YOS_KVFILE_BACKUP=\"/tmp/kv_bk\"
GLOBAL_CFLAGS       += -Wno-missing-field-initializers
GLOBAL_DEFINES      += CSP_LINUXHOST

# arch linux
$(NAME)_SOURCES := $(ARCH_LINUX)/cpu_impl.c
$(NAME)_SOURCES += $(ARCH_LINUX)/cpu_longjmp_32.S
# mcu
$(NAME)_SOURCES     += main/arg_options.c
$(NAME)_SOURCES     += main/main.c
$(NAME)_SOURCES     += main/hw.c
$(NAME)_SOURCES     += csp/csp_rhino.c
$(NAME)_SOURCES     += soc/soc_impl.c
$(NAME)_SOURCES     += soc/hook_impl.c
$(NAME)_SOURCES     += soc/ysh_impl.c

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
