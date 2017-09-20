NAME := mesh

$(NAME)_TYPE := kernel
GLOBAL_INCLUDES += include

$(NAME)_INCLUDES += include

include kernel/protocols/mesh/files.mk
$(NAME)_SOURCES := $(umesh_srcs)

MESHDEBUG ?= 1
ifeq ($(MESHDEBUG), 1)
$(NAME)_SOURCES += src/tools/diags.c
$(NAME)_SOURCES += src/utilities/mac_whitelist.c
$(NAME)_SOURCES += src/utilities/logging.c
GLOBAL_DEFINES += CONFIG_AOS_MESH_DEBUG
endif

MESHSUPER ?= 1
ifeq ($(MESHSUPER), 1)
$(NAME)_SOURCES += src/core/routing/vector_router.c
$(NAME)_SOURCES += src/core/routing/rsid_allocator.c
GLOBAL_DEFINES += CONFIG_AOS_MESH_SUPER
endif

ifeq ($(CONFIG_AOS_MESH_TAPIF), 1)
$(NAME)_SOURCES += src/ip/tapif_adapter.c
$(NAME)_DEFINES += CONFIG_AOS_MESH_TAPIF
endif

LWIP ?=1
ifeq ($(LWIP), 1)
$(NAME)_SOURCES += src/ip/lwip_adapter.c
$(NAME)_SOURCES += src/ip/lwip_ip6.c
else
$(NAME)_SOURCES += src/utilities/mem/pbuf.c
$(NAME)_SOURCES += src/utilities/mem/def.c
endif

$(NAME)_CFLAGS += -Wall -Werror
GLOBAL_DEFINES += CONFIG_AOS_MESH
