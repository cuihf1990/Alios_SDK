NAME := mbedtls

$(NAME)_CFLAGS      += -Wall -Werror -Os

GLOBAL_INCLUDES     += include

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
$(NAME)_DEFINES          += CONFIG_LINUX_HOST
$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libmbedtls.a
else
$(error "not find correct platform!")
endif

