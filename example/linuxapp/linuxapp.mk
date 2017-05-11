NAME := linuxapp

$(NAME)_SOURCES     := main.c

$(NAME)_COMPONENTS  := ysh
ifeq ($(net),1)
$(NAME)_COMPONENTS  += protocols.net
endif

ifeq ($(mesh),1)
$(NAME)_COMPONENTS  += protocols.net protocols.mesh
endif

ifeq ($(dda),1)
GLOBAL_LDFLAGS += -lreadline
$(NAME)_COMPONENTS  += dda protocols.net protocols.mesh
endif
