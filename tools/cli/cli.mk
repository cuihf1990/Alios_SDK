NAME := kernel_cli

$(NAME)_SOURCES     := yos_cli.c
$(NAME)_SOURCES     += dumpsys.c

GLOBAL_INCLUDES     += include
GLOBAL_DEFINES      += HAVE_NOT_ADVANCED_FORMATE CONFIG_YOS_CLI
