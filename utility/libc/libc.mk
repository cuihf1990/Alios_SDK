NAME := newlib_stub

ifneq ($(findstring linuxhost, $(BUILD_STRING)), linuxhost)
$(NAME)_TYPE := share
$(NAME)_SOURCES := newlib_stub.c 
$(NAME)_LINK_FILES := newlib_stub.o
endif
