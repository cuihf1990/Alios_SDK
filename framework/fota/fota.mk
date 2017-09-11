NAME := fota

$(NAME)_CFLAGS += \
	-Wall \
	-Werror \
	-Wno-unused-function

$(NAME)_SOURCES += \
    ota_service.c

ifneq (,$(filter connectivity.coap,$(COMPONENTS)))
$(NAME)_DEFINES += CONNECTIVITY_COAP
else
$(NAME)_SOURCES += \
    ota_util.c \
    ota_update_manifest.c \
    ota_download.c \
    ota_version.c
endif

$(NAME)_COMPONENTS += fota.platform digest_algorithm

$(NAME)_INCLUDES := \
    ./platform \
    ../../include/hal \

GLOBAL_INCLUDES += .
