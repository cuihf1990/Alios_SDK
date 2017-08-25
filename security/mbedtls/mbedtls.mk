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

$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libmbedtls.a

ifeq (1,$(with_lwip))
$(info using lwip version mbedtls)
$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libmbedtls.a.lwip
endif

else ifeq ($(findstring armhflinux, $(BUILD_STRING)), armhflinux)

$(NAME)_PREBUILT_LIBRARY := lib/armhflinux/libmbedtls.a

else ifeq ($(findstring mk108, $(BUILD_STRING)), mk108)

$(NAME)_PREBUILT_LIBRARY := lib/mk108/libmbedtls.a

else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)

$(NAME)_PREBUILT_LIBRARY := lib/mk3060/libmbedtls.a

else

$(error "not find correct platform!")

endif

