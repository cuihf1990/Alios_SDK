NAME := usyscall

$(NAME)_INCLUDES := ./ \
                    ../../kernel/protocols/net/include/ \
                    ../../kernel/protocols/mesh/include/

$(NAME)_CFLAGS += -Wall -Werror

$(NAME)_COMPONENTS := protocols.net

$(NAME)_SOURCES := syscall_uapi.c

