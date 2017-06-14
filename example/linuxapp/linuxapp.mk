NAME := linuxapp

$(NAME)_SOURCES     := main.c

$(NAME)_COMPONENTS  := cli
ifeq ($(LWIP),1)
$(NAME)_COMPONENTS  += protocols.net
endif
