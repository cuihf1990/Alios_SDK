NAME := libkm

$(NAME)_CFLAGS      += -Wall -Werror -Os

GLOBAL_INCLUDES     += include

ifeq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
$(NAME)_PREBUILT_LIBRARY := lib/linuxhost/libkm.a
else ifeq ($(findstring armhflinux, $(BUILD_STRING)), armhflinux)
$(NAME)_PREBUILT_LIBRARY := lib/armhflinux/libkm.a
else ifeq ($(findstring mk3060, $(BUILD_STRING)), mk3060)
$(NAME)_PREBUILT_LIBRARY := lib/mk3060/libkm.a
else ifeq ($(findstring b_l475e, $(BUILD_STRING)), b_l475e)
$(NAME)_PREBUILT_LIBRARY := lib/b_l475e/libkm.a
else
$(error "not find correct platform!")
endif

$(NAME)_SOURCES +=  \
		   lib/b_l475e/osa_flash.c \

#$(NAME)_SOURCES +=  \
		   sample/ks_test.c
