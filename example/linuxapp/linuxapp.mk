NAME := linuxapp

$(NAME)_SOURCES     := main.c

$(NAME)_COMPONENTS  := ysh
ifeq ($(NET),1)
$(NAME)_COMPONENTS  += protocols.net
endif
