NAME := sal
GLOBAL_DEFINES += WITH_SAL # for sal general use
$(NAME)_TYPE := kernel
$(NAME)_SOURCES := sal_sockets.c
GLOBAL_INCLUDES += .
