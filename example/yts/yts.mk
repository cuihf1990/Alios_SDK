NAME := yts

$(NAME)_SOURCES := main.c
$(NAME)_COMPONENTS := testcase rhino.test log vfs yloop hal

$(NAME)_CFLAGS += -Wall -Werror

ifneq (,$(findstring linux, $(BUILD_STRING)))
$(info build string is $(BUILD_STRING))
$(NAME)_COMPONENTS += protocols.net protocols.mesh dda netmgr modules.fs.fatfs framework.common

GLOBAL_LDFLAGS += -lreadline -lncurses
GLOBAL_DEFINES += CONFIG_AOS_MESHYTS DEBUG CONFIG_AOS_YTS_ALL
endif

