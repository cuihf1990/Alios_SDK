NAME := linuxapp

$(NAME)_SOURCES     := main.c

$(NAME)_COMPONENTS  := ysh
ifeq ($(net),1)
$(NAME)_COMPONENTS  += protocols.net
endif
