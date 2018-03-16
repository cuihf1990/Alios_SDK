NAME := ywss4linkkit

$(NAME)_DEFINES := USE_LPTHREAD

$(NAME)_TYPE := framework
GLOBAL_DEFINES += CONFIG_YWSS

$(NAME)_COMPONENTS := digest_algorithm protocol.alink-ilop connectivity.link-coap connectivity.mqtt

ifeq ($(HOST_ARCH), linux)
LIB_PATH := linux
else ifeq ($(HOST_ARCH), ARM968E-S)
LIB_PATH := arm968es
else ifeq ($(HOST_ARCH), xtensa)
LIB_PATH := xtensa
else ifeq ($(HOST_ARCH), Cortex-M4)
LIB_PATH := cortex-m4
else
$(error "not find correct platform!")
endif

$(NAME)_PREBUILT_LIBRARY := lib/$(LIB_PATH)/ywss4linkkit.a

