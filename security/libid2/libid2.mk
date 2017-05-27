NAME := libid2

$(NAME)_CFLAGS      += -Wall -Werror -Os

GLOBAL_INCLUDES     += include

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
$(NAME)_DEFINES          += CONFIG_LINUX_HOST
$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libid2.a
else
$(error "not find correct platform!")
endif

