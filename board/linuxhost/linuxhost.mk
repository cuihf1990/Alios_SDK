
NAME := board_linuxhost

MODULE              := 1062
HOST_ARCH           := linux
HOST_MCU_FAMILY     := linux

$(NAME)_COMPONENTS  := yloop vfs wrapper hal log vcall ysh

$(NAME)_INCLUDES    += .
GLOBAL_INCLUDES     += include csp/lwip/include
GLOBAL_LDFLAGS      += -lpthread -lm -lcrypto
GLOBAL_DEFINES      += CONFIG_YOS_RHINO_MMREGION
GLOBAL_CFLAGS       += -Wno-missing-field-initializers

$(NAME)_SOURCES     := main/arg_options.c
$(NAME)_SOURCES     += main/main.c
$(NAME)_SOURCES     += main/hw.c
$(NAME)_SOURCES     += main/sensor.c
$(NAME)_SOURCES     += csp/csp_rhino.c
$(NAME)_SOURCES     += soc/soc_impl.c
$(NAME)_SOURCES     += soc/hook_impl.c
$(NAME)_SOURCES     += soc/ysh_impl.c

$(info $(COMPONENTS))
ifneq (,$(filter protocols.net,$(COMPONENTS)))
$(NAME)_DEFINES     += CONFIG_YOS_NET_PROTOCOL

$(info "net is enabled")
$(NAME)_SOURCES     += \
    csp/lwip/netif/delif.c \
    csp/lwip/netif/fifo.c \
    csp/lwip/netif/list.c \
    csp/lwip/netif/pcapif.c \
    csp/lwip/netif/sio.c \
    csp/lwip/netif/tapif.c \
    csp/lwip/netif/tcpdump.c \
    csp/lwip/netif/tunif.c

$(NAME)_SOURCES     += csp/lwip/lwip_linuxhost.c
endif
