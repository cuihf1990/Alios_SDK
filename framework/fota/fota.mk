NAME := fota

$(NAME)_CFLAGS += -Wall -Werror

$(NAME)_SOURCES += \
    ota_util.c \
    ota_update_manifest.c \
    ota_service.c \
    ota_download.c
 
$(NAME)_COMPONENTS += fota.platform digest_algorithm

$(NAME)_INCLUDES := \
    ./ \
    ../../include/hal \
