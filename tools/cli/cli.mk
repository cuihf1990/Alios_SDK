NAME := cli

$(NAME)_TYPE := kernel

$(NAME)_SOURCES := cli.c dumpsys.c
$(NAME)_CFLAGS  += -Wall -Werror

$(NAME)_COMPONENTS += hal

GLOBAL_INCLUDES += include
GLOBAL_DEFINES  += HAVE_NOT_ADVANCED_FORMATE CONFIG_AOS_CLI
