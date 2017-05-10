NAME := os
$(NAME)_SOURCES = os_misc.c os_thread.c
$(NAME)_COMPONENTS := protocol.alink.os.platform protocol.alink.os.product
$(NAME)_INCLUDES := product/ platform/ 
