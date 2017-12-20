NAME := newlib_stub

ifeq ($(COMPILER),armcc)
$(NAME)_TYPE := share
$(NAME)_SOURCES := compilers/armlibc/armcc_libc.c
GLOBAL_INCLUDES += compilers/armlibc 
else ifeq ($(COMPILER),EWARM)
GLOBAL_INCLUDES += compilers/EWARM 
$(NAME)_TYPE := share
$(NAME)_SOURCES := compilers/armlibc/iar_libc.c
else ifneq ($(HOST_MCU_FAMILY),linux)
$(NAME)_TYPE := share
$(NAME)_SOURCES := newlib_stub.c
endif

