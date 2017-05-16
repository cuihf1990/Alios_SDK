NAME := meshhapp

$(NAME)_SOURCES     := main.c

$(NAME)_COMPONENTS  := ysh
$(NAME)_COMPONENTS  += protocols.net protocols.mesh
GLOBAL_DEFINES      += TAPIF_DEFAULT_OFF

ifneq (,$(filter linuxhost,$(COMPONENTS)))
DDA := 1
endif

ifeq ($(DDA),1)
GLOBAL_LDFLAGS += -lreadline
$(NAME)_COMPONENTS  += dda
endif
