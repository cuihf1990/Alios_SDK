NAME := libid2

$(NAME)_CFLAGS      += -Wall -Werror -Os

GLOBAL_INCLUDES     += include

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libid2.a
else ifeq ($(findstring armhflinux, $(BUILD_STRING)), armhflinux)
$(NAME)_PREBUILT_LIBRARY := lib/armhflinux/libid2.a
else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)
$(NAME)_PREBUILT_LIBRARY := lib/mk3060/libid2.a
else
$(error "not find correct platform!")
endif

