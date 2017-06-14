NAME := cli

$(NAME)_SOURCES     := aos_cli.c
$(NAME)_SOURCES     += dumpsys.c

GLOBAL_INCLUDES     += include
GLOBAL_DEFINES      += HAVE_NOT_ADVANCED_FORMATE
