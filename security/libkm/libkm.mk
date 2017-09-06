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
else
$(error "not find correct platform!")
endif

