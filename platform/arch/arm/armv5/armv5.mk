NAME := armv5

GLOBAL_INCLUDES += .

$(NAME)_CFLAGS := -marm -mthumb-interwork

$(NAME)_SOURCES	 += port_c.c \
                    port_s.S 

