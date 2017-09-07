NAME := mbedtls

DEBUG := no

GLOBAL_INCLUDES     += include

$(NAME)_CFLAGS      += -Wall -Werror -Os

$(NAME)_SOURCES     := mbedtls_ssl.c

ifeq ($(DEBUG), yes)
$(NAME)_DEFINES     += CONFIG_SSL_DEBUG
endif

$(NAME)_COMPONENTS := alicrypto

PLATFORM := linuxhost
ifeq ($(HOST_ARCH), linux)

PLATFORM := linuxhost
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libmbedtls.a

ifeq (1,$(with_lwip))
$(info using lwip version mbedtls)
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libmbedtls.a.lwip
endif

else ifeq ($(HOST_ARCH), armhflinux)

PLATFORM := armhflinux
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libmbedtls.a

else ifeq ($(HOST_ARCH), ARM968E-S)

PLATFORM := mk108
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libmbedtls.a

else

$(error "not find correct platform!")

endif

