NAME := kv

$(NAME)_SOURCES := kvmgr.c 
$(NAME)_COMPONENTS += log hashtable digest_algorithm
$(NAME)_INCLUDES += ../../../utility/digest_algorithm/ 
$(NAME)_INCLUDES += ../../../utility/hashtable 

GLOBAL_INCLUDES  += include
