NAME := fota

$(NAME)_CFLAGS += \
	-Wall \
	-Werror \
	-Wno-unused-function

$(NAME)_SOURCES += \
    ota_service.c

ifneq (,$(filter protocol.alink,$(COMPONENTS)))
$(NAME)_SOURCES += \
    ota_util.c \
    ota_update_manifest.c \
    ota_download.c \
    ota_version.c 
else
ifneq (,$(filter connectivity.mqtt,$(COMPONENTS)))
$(NAME)_SOURCES += \
    ota_util.c \
    ota_update_manifest.c \
    ota_download.c \
    ota_version.c 
else
ifneq (,$(filter connectivity.coap,$(COMPONENTS)))
$(NAME)_DEFINES += CONNECTIVITY_COAP
endif
endif
endif

$(NAME)_COMPONENTS += fota.platform digest_algorithm

$(NAME)_INCLUDES := \
    ./platform \
    ../../include/hal \

GLOBAL_INCLUDES += .
