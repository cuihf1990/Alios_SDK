NAME := linuxapp

$(NAME)_SOURCES     := main.c

$(NAME)_COMPONENTS  := cli
ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
endif

ifeq ($(BENCHMARKS),1)
$(NAME)_COMPONENTS  += benchmarks
GLOBAL_DEFINES      += CONFIG_CMD_BENCHMARKS
endif
