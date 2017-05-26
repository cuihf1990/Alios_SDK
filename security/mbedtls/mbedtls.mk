NAME := mbedtls

DEBUG := no

GLOBAL_INCLUDES     += include

$(NAME)_CFLAGS      += -Wall -Werror -Os

$(NAME)_SOURCES     := mbedtls_ssl.c

ifeq ($(DEBUG), yes)
GLOBAL_DEFINES      += CONFIG_SSL_DEBUG
endif

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libmbedtls.a
else
$(error "not find correct platform!")
endif

