NAME := syscall

$(NAME)_TYPE := kernel
$(NAME)_INCLUDES := ./ \
                    ../protocols/net/include/ \
                    ../protocols/mesh/include/

$(NAME)_CFLAGS += -Wall -Werror

$(NAME)_COMPONENTS += protocols.net protocols.mesh

$(NAME)_SOURCES := \
                   syscall_tbl.c

GLOBAL_INCLUDES += ./

