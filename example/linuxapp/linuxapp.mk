NAME := linuxapp

$(NAME)_SOURCES     := main.c

$(NAME)_COMPONENTS  := ysh
ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
endif

GLOBAL_DEFINES += CONFIG_USE_DEF_AP
