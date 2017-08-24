NAME := fatfs

$(NAME)_TYPE        := kernel

$(NAME)_SOURCES     := ff.c
$(NAME)_SOURCES     += fatfs.c
$(NAME)_SOURCES     += ffunicode.c

GLOBAL_INCLUDES     += include 
