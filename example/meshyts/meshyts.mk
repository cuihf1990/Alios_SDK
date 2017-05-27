NAME := meshyts

$(NAME)_SOURCES := main.c
$(NAME)_COMPONENTS := protocols.net protocols.mesh dda testcase
$(NAME)_CFLAGS += -Wall -Werror

GLOBAL_DEFINES += TAPIF_DEFAULT_OFF
GLOBAL_LDFLAGS += -lreadline -lncurses
GLOBAL_DEFINES += CONFIG_YOS_MESHYTS
