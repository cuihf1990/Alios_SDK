NAME := meshapp
no_with_lwip := 0

$(NAME)_SOURCES     := main.c

$(NAME)_COMPONENTS  += protocols.net protocols.mesh cli
GLOBAL_DEFINES      += TAPIF_DEFAULT_OFF DEBUG

ifneq (,$(filter linuxhost,$(COMPONENTS)))
DDA ?= 1
endif

ifneq (,$(filter armhflinux,$(COMPONENTS)))
DDA ?= 1
endif

MESHDEBUG ?= 1

ifeq ($(DDA),1)
GLOBAL_LDFLAGS += -lreadline -lncurses
$(NAME)_COMPONENTS  += dda
endif
