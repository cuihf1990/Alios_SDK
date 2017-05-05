HOST_OPENOCD := linux

NAME := linuximpl

$(NAME)_SOURCES := cpu_impl.c
$(NAME)_SOURCES += cpu_longjmp_32.S

GLOBAL_INCLUDES += .
