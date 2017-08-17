NAME := mesh

GLOBAL_INCLUDES += include

$(NAME)_INCLUDES += include

include kernel/protocols/mesh/files.mk
$(NAME)_SOURCES := $(umesh_srcs)

MESHDEBUG ?= 1
ifeq ($(MESHDEBUG), 1)
$(NAME)_SOURCES += src/tools/diags.c
$(NAME)_SOURCES += src/utilities/mac_whitelist.c
GLOBAL_DEFINES += CONFIG_YOS_MESH_DEBUG
endif

MESHSUPER ?= 1
ifeq ($(MESHSUPER), 1)
$(NAME)_SOURCES += src/core/routing/vector_router.c
$(NAME)_SOURCES += src/core/routing/rsid_allocator.c
GLOBAL_DEFINES += CONFIG_YOS_MESH_SUPER
endif

$(NAME)_CFLAGS += -Wall -Werror
GLOBAL_DEFINES += CONFIG_YOS_MESH
