NAME := libkm

$(NAME)_CFLAGS      += -Wall -Werror -Os

GLOBAL_INCLUDES     += include

PLATFORM := linuxhost
ifeq ($(HOST_ARCH), linux)
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libkm.a
else ifeq ($(HOST_ARCH), armhflinux)
PLATFORM := armhflinux
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libkm.a
else ifeq ($(HOST_ARCH), ARM968E-S)
PLATFORM := mk3060
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libkm.a
else ifeq ($(findstring b_l475e, $(BUILD_STRING)), b_l475e)
$(NAME)_PREBUILT_LIBRARY := lib/b_l475e/libkm.a
else
$(error "not find correct platform!")
endif

$(NAME)_SOURCES +=  \
		   lib/b_l475e/osa_flash.c \

#$(NAME)_SOURCES +=  \
		   sample/ks_test.c
