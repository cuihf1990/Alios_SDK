NAME := libid2

$(NAME)_CFLAGS      += -Wall -Werror -Os

GLOBAL_INCLUDES     += include

PLATFORM := linuxhost
ifeq ($(HOST_ARCH), linux)
PLATFORM := linuxhost
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libid2.a
else ifeq ($(HOST_ARCH), armhflinux)
PLATFORM := armhflinux
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libid2.a
else ifeq ($(HOST_ARCH), ARM968E-S)
PLATFORM := mk3060
$(NAME)_PREBUILT_LIBRARY := lib/$(PLATFORM)/libid2.a
else ifeq ($(findstring b_l475e, $(BUILD_STRING)), b_l475e)
$(NAME)_PREBUILT_LIBRARY := lib/b_l475e/libid2.a
else
$(error "not find correct platform!")
endif

#$(NAME)_SOURCES +=  \
		   sample/id2_test.c
