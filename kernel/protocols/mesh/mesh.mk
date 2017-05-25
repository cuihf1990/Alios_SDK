NAME := mesh

GLOBAL_INCLUDES += include

$(NAME)_INCLUDES += include

include kernel/protocols/mesh/files.mk
$(NAME)_SOURCES := $(umesh_srcs)

$(NAME)_CFLAGS += -Wall -Werror
GLOBAL_DEFINES += CONFIG_YOS_MESH
