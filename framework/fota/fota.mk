NAME := fota

$(NAME)_CFLAGS += -Wall -Werror

$(NAME)_SOURCES += \
    ota_util.c \
    ota_update_manifest.c \
    ota_service.c \
    ota_download.c \
    ota_version.c 
 
$(NAME)_COMPONENTS += fota.platform digest_algorithm

$(NAME)_INCLUDES := \
    ./platform \
    ../../include/hal \

GLOBAL_INCLUDES += .
