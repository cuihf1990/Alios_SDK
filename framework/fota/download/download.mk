NAME := fota_download

$(NAME)_TYPE := kernel
GLOBAL_INCLUDES += ./

$(NAME)_CFLAGS += -Wall -Werror

$(NAME)_SOURCES := download_common.c

ifeq ($(DOWNLOAD_COAP), true)
$(NAME)_COMPONENTS += fota.download.coap
else
$(NAME)_COMPONENTS += fota.download.http
endif

