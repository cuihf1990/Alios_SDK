NAME := yts

$(NAME)_SOURCES := main.c
$(NAME)_COMPONENTS := protocols.net protocols.mesh dda testcase  rhino.test

$(NAME)_CFLAGS += -Wall -Werror

GLOBAL_LDFLAGS += -lreadline -lncurses
GLOBAL_DEFINES += CONFIG_AOS_MESHYTS DEBUG
