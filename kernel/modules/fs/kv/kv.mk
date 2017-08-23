NAME := kv

$(NAME)_TYPE := kernel
$(NAME)_SOURCES := kvmgr.c 
$(NAME)_COMPONENTS += log

GLOBAL_INCLUDES  += include
