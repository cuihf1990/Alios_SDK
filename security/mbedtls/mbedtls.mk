NAME := mbedtls

DEBUG := no

GLOBAL_INCLUDES     += include

$(NAME)_CFLAGS      += -Wall -Werror -Os

$(NAME)_SOURCES     := mbedtls_ssl.c

ifeq ($(DEBUG), yes)
$(NAME)_DEFINES     += CONFIG_SSL_DEBUG
endif

$(NAME)_COMPONENTS := alicrypto

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
#$(NAME)_COMPONENTS += libmbedtls
$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libmbedtls.a
else
$(error "not find correct platform!")
endif

