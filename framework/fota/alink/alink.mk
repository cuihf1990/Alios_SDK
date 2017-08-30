NAME := ota_alink

$(NAME)_CFLAGS += -Wall -Werror

$(NAME)_SOURCES += \
    ota_util.c \
    ota_update_manifest.c \
    ota_service_alink.c \
    ota_download.c \
    ota_version.c 
 
$(NAME)_COMPONENTS += fota.alink.platform digest_algorithm

$(NAME)_INCLUDES := \
    ./platform \
    ../../../include/hal \

GLOBAL_INCLUDES += \
    ./ \
    ../
