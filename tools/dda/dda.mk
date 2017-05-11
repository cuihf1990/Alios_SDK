NAME := dda

GLOBAL_INCLUDES += include

$(NAME)_INCLUDES += kernel/protocols/mesh/include

$(NAME)_SOURCES := eloop.c \
                   agent.c \
                   msg.c \
                   hal.c \
                   config_parser.c \
                   master.c
