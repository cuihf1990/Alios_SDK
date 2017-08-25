NAME := fatfs

$(NAME)_TYPE        := kernel

$(NAME)_SOURCES     := ff.c
$(NAME)_SOURCES     += fatfs.c
$(NAME)_SOURCES     += ffunicode.c
$(NAME)_CFLAGS      += -Wall -Werror

GLOBAL_INCLUDES     += include 
