include $(BUILD_MODULE)
NAME := coap_ota

UTIL_SOURCE := ../../../../utility/iotx-utils

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
PLATFORM_COAP := linux
else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)
PLATFORM_COAP := rhino
else ifeq ($(findstring b_l475e, $(BUILD_STRING)), b_l475e)
PLATFORM_COAP := rhino
endif

$(NAME)_INCLUDES :=  \
    ./src \
    ../ \
    $(UTIL_SOURCE)/misc \
    $(UTIL_SOURCE)/sdk-impl \
    $(UTIL_SOURCE)/sdk-impl/exports \
    $(UTIL_SOURCE)/sdk-impl/imports \
    $(UTIL_SOURCE)/LITE-log \
    $(UTIL_SOURCE)/LITE-utils \
    $(UTIL_SOURCE)/digest

$(NAME)_SOURCES := \
    ./src/ota.c \
    ./src/ota_lib.c \
	ota_service_coap.c \
    $(UTIL_SOURCE)/hal/$(PLATFORM_COAP)/HAL_OS_$(PLATFORM_COAP).c \
	ota_transport.c

$(NAME)_COMPONENTS += connectivity.coap
$(NAME)_DEFINES += OTA_CH_SIGNAL_COAP

$(NAME)_COMPONENTS += iotx-utils.misc
$(NAME)_COMPONENTS += iotx-utils.sdk-impl
$(NAME)_CFLAGS := \
    -Werror \
    -Wno-unused-function \
    -Wno-implicit-function-declaration
#$(filter-out -Werror,$(CFLAGS))

$(NAME)_DEFINES += DEBUG
GLOBAL_DEFINES      += IOTX_DEBUG

ifeq ($(CONFIG_COAP_ONLINE), y)
$(NAME)_DEFINES += COAP_ONLINE
endif
ifeq ($(CONFIG_COAP_DTLS_SUPPORT), y)
$(NAME)_DEFINES += COAP_DTLS_SUPPORT
endif
