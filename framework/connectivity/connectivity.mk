NAME := connectivity

$(NAME)_SOURCES := connectivity_manager.c 
$(NAME)_COMPONENTS := connectivity.wsf
$(NAME)_INCLUDES := ../protocol/alink/os/
GLOBAL_INCLUDES += .
