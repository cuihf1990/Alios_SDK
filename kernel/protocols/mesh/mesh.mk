NAME := mesh

GLOBAL_INCLUDES += include

$(NAME)_INCLUDES += include

include kernel/protocols/mesh/files.mk
$(NAME)_SOURCES := $(umesh_srcs)

ifeq ($(MESHDEBUG), 1)
$(NAME)_SOURCES +=  src/tools/cli.c
$(NAME)_SOURCES +=  src/tools/diags.c
GLOBAL_DEFINES += CONFIG_YOS_MESH_DEBUG
endif

$(NAME)_CFLAGS += -Wall -Werror
GLOBAL_DEFINES += CONFIG_YOS_MESH
