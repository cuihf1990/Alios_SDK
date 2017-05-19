NAME := kv

$(NAME)_SOURCES := kvmgr.c 
$(NAME)_COMPONENTS += log hashtable digest_algorithm
$(NAME)_COMPONENTS += protocol.alink.os
$(NAME)_INCLUDES := ../../../framework/protocol/alink/os/
$(NAME)_INCLUDES += ../../../utility/digest_algorithm/ 
$(NAME)_INCLUDES += ../../../framework/protocol/alink/system/ 
$(NAME)_INCLUDES += ../../../utility/hashtable ./include 

