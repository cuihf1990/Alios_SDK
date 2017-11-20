NAME := device_sal_mk3060

$(NAME)_COMPONENTS += sal atparser

$(NAME)_SOURCES += mk3060.c

GLOBAL_DEFINES += WITH_SAL_WIFI
