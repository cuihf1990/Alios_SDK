NAME := fota_download

$(NAME)_TYPE := kernel
GLOBAL_INCLUDES += ./

$(NAME)_CFLAGS += -Wall -Werror

$(NAME)_SOURCES := download_common.c

ifneq (,$(filter connectivity.coap,$(COMPONENTS)))
ifneq (,$(CONFIG_FOTA_DOWNLOAD_COAP))
$(NAME)_COMPONENTS += fota.download.coap
else
$(NAME)_COMPONENTS += fota.download.http
endif
else
$(NAME)_COMPONENTS += fota.download.http
endif
