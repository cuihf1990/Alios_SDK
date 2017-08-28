NAME := helloworld

$(NAME)_SOURCES := helloworld.c

$(NAME)_COMPONENTS += cli

GLOBAL_DEFINES += YOS_NO_WIFI

ifeq ($(BENCHMARKS),1)
$(NAME)_COMPONENTS  += benchmarks
GLOBAL_DEFINES      += CONFIG_CMD_BENCHMARKS
endif
