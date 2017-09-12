ifeq (1,${BINS})
	GLOBAL_CFLAGS += -DSYSINFO_OS_BINS
endif
CURRENT_TIME = $(shell /bin/date +%Y%m%d.%H%M)
CONFIG_SYSINFO_APP_VERSION = APP-1.0.0-$(CURRENT_TIME)
$(info app_version:${CONFIG_SYSINFO_APP_VERSION})
	GLOBAL_CFLAGS += -DSYSINFO_APP_VERSION=\"$(CONFIG_SYSINFO_APP_VERSION)\"

NAME := otatest

CONFIG_COAP_DTLS_SUPPORT := y
#CONFIG_COAP_ONLINE := y

ifeq ($(CONFIG_COAP_ONLINE), y)
$(NAME)_DEFINES += COAP_ONLINE
endif
ifeq ($(CONFIG_COAP_DTLS_SUPPORT), y)
$(NAME)_DEFINES += COAP_DTLS_SUPPORT
endif

$(NAME)_COMPONENTS  := cli
#ifeq ($(LWIP),1)
#$(NAME)_COMPONENTS  += protocols.net
#endif

CONFIG_OTA_CH := coap
$(NAME)_SOURCES     := ota_coap_test.c

$(NAME)_CFLAGS += \
    -Wno-unused-function \
    -Wno-implicit-function-declaration \
    -Wno-unused-function \
    -Werror

$(NAME)_COMPONENTS  += fota.platform.coap
$(NAME)_COMPONENTS  += connectivity.coap
